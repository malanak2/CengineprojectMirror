#include "Texture.hpp"
#include "Graphics.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include "sys/stat.h"
#include <memory>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>
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
  {
    ZoneNamedN(__zone_read, "Texture::ReadFile", true);
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
  }
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER,
                     "Loading texture at {}, with image at {}", this->path,
                     this->texture_path);
  std::shared_ptr<FileUtil::ImageFile> tex;
  {
    ZoneNamedN(__zone_decode, "Texture::LoadImage", true);
    tex = FileUtil::LoadImage(this->texture_path);
  }
  if (!tex || !tex->data) {
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
  {
    ZoneNamedN(__zone_upload, "Texture::GpuUpload", true);
    {
      TracyGpuZone("TextureUpload");
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glPixelStorei(GL_UNPACK_ALIGNMENT, (tex->nrChannels == 4) ? 4 : 1);
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

      GLenum internalFormat = GL_RGBA8;
      GLenum format = GL_RGBA;
      if (tex->nrChannels == 1) {
        internalFormat = GL_R8;
        format = GL_RED;
      } else if (tex->nrChannels == 2) {
        internalFormat = GL_RG8;
        format = GL_RG;
      } else if (tex->nrChannels == 3) {
        internalFormat = GL_RGB8;
        format = GL_RGB;
      } else {
        internalFormat = GL_RGBA8;
        format = GL_RGBA;
      }

      int levels = 1;
      if (mipmap) {
        int maxDim = std::max(tex->width, tex->height);
        while (maxDim > 1) {
          maxDim >>= 1;
          levels++;
        }
      }

      glTexStorage2D(GL_TEXTURE_2D, levels, internalFormat, tex->width,
                     tex->height);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->width, tex->height, format,
                      GL_UNSIGNED_BYTE, tex->data);

      if (mipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
      }
      glBindTexture(GL_TEXTURE_2D, 0);
    }
    glFlush();
  }
}
std::shared_ptr<Engine::Graphics::Texture>
Engine::Graphics::Texture::Create(std::string json_path) {
  ZoneScoped;
  auto cache = &Engine::Graphics::Main::textures;
  if (cache->contains(json_path))
    return (*cache)[json_path];

  auto t = std::make_shared<Texture>(json_path);
  (*cache)[json_path] = t;
  return t;
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
  std::string diskPath = "resources/" + jsonPath;
  if (stat(diskPath.c_str(), &sb) != 0) {
    TextureJson tj;
    tj.path = "textures/models/" + filename;
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
