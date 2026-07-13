#include "Texture.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <memory>

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

Engine::Graphics::Texture::Texture(std::string json_path, int fallback) {
  this->path = json_path;
  std::string js;
  FileUtil::ReadFile(json_path, &js);
  json parsed = json::parse(js);
  FromJson(parsed);
  auto tex = FileUtil::LoadImage(this->texture_path);
  if (!tex->data) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Failed to load image at {}, using backup...");
    texture = fallback;
    return;
  }
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  switch (filterType) {
  case nearest:
    if (mipmap) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_NEAREST);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    break;
  case linear:
    if (mipmap) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex->width, tex->height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, tex->data);
  if (mipmap) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
}
// TODO: Have a cache
std::shared_ptr<Engine::Graphics::Texture>
Engine::Graphics::Texture::Create(std::string json_path, int fallback) {
  return std::make_shared<Texture>(json_path, fallback);
}
