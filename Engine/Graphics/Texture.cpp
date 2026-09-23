#include "Texture.hpp"
#include "Graphics.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include "sys/stat.h"
#include <memory>
#include <tracy/Tracy.hpp>
using namespace Engine::Graphics;
json Engine::Graphics::Texture::ToJson() {
  JsonFileBase ret = JsonFileBase();
  ret.object_type = ObjectType::Texture;
  TextureJson js = TextureJson();
  js.translucent = translucent;
  js.filterType = filterType;
  js.path = texture_path;
  js.mipmap = mipmap;
  js.wrapS = wrapS;
  js.wrapT = wrapT;
  ret.data = js;
  return ret;
}

void Engine::Graphics::Texture::FromJson(json &js) {
  JsonFileBase data = js;
  if (data.object_type != ObjectType::Texture) {
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                       "Tried to open {} as texture (path: {})",
                       static_cast<int>(data.object_type), path);
    return;
  }
  TextureJson tj = data.data;
  this->filterType = tj.filterType;
  this->texture_path = tj.path;
  this->translucent = tj.translucent;
  this->mipmap = tj.mipmap;
  this->wrapS = tj.wrapS;
  this->wrapT = tj.wrapT;
}

Engine::Graphics::Texture::Texture(std::string json_path) {
  ZoneScoped;
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Loading texture at {}", json_path);
  this->path = json_path;
  std::string js;
  auto r = FileUtil::ReadFile(json_path, &js);
  if (r != 0) {
    auto a = Graphics::Main::FallbackTexture;
    if (a == nullptr) {
      texture = -1;
    } else {
      texture = a->texture;
    }
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Failed to open file at {}, falling back to {}",
                        json_path, -1);
    return;
  }
  json parsed = json::parse(js);
  FromJson(parsed);
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER,
                     "Loading texture at {}, with image at {}", this->path,
                     this->texture_path);
  auto tex = FileUtil::LoadImage(this->texture_path);
  if (!tex->data) {
    auto a = Graphics::Main::FallbackTexture;
    if (a == nullptr) {
      texture = -1;
    } else {
      texture = a->texture;
    }
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Failed to load image at {}, using backup...",
                        this->texture_path);
    return;
  }
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  switch (filterType) {
  case nearest:
    if (mipmap) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_NEAREST);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    break;
  case linear:
    if (mipmap) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    break;
  }
  switch (wrapS) {
  case repeat:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    break;
  case mirrored_repeat:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    break;
  case clamp_to_edge:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    break;
  case clamp_to_border:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    break;
  }
  switch (wrapT) {
  case repeat:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    break;
  case mirrored_repeat:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    break;
  case clamp_to_edge:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    break;
  case clamp_to_border:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    break;
  }
  GLenum format = GL_RGB;
  if (tex->nrChannels == 4 || translucent) {
    format = GL_RGBA;
  } else if (tex->nrChannels == 1) {
    format = GL_RED;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, format, tex->width, tex->height, 0, format,
               GL_UNSIGNED_BYTE, tex->data);

  if (mipmap) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
}
// TODO: Have a cache
std::shared_ptr<Engine::Graphics::Texture>
Engine::Graphics::Texture::Create(std::string json_path) {
  return std::make_shared<Texture>(json_path);
}
std::shared_ptr<Texture>
Engine::Graphics::Texture::CreateModel(std::string textureName) {
  std::string modelTexMetaPath =
      Config::inst->graphics->texturePath + "/models";

  size_t lastSlash = textureName.find_last_of("/\\");
  std::string filename = (lastSlash != std::string::npos)
                             ? textureName.substr(lastSlash + 1)
                             : textureName;

  size_t lastDot = filename.find_last_of(".");
  std::string rawname =
      (lastDot != std::string::npos) ? filename.substr(0, lastDot) : filename;

  struct stat sb;
  std::string jsonPath = modelTexMetaPath + "/" + rawname + ".json";
  if (stat(jsonPath.c_str(), &sb) != 0) {
    TextureJson tj;
    tj.path = "textures/" + filename; // points cleanly to textures/<filename>
    tj.filterType = TexFiltering::linear;
    tj.wrapS = TexWrap::repeat;
    tj.wrapT = TexWrap::repeat;
    tj.mipmap = true;
    tj.translucent = false;

    JsonFileBase fb;
    fb.object_type = ObjectType::Texture;
    fb.data = tj;
    json j = fb;

    std::string data = j.dump(2);
    FileUtil::SaveFile(jsonPath, &data);
  }
  return Texture::Create(jsonPath);
}
