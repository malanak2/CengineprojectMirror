#pragma once

#include "Interfaces/IJson.hpp"
#include "glad/glad.h"
#include <memory>
using json = nlohmann::json;
namespace Engine::Graphics {
enum TexFiltering { nearest, linear };
NLOHMANN_JSON_SERIALIZE_ENUM(TexFiltering, {{TexFiltering::linear, "linear"},
                                            {TexFiltering::nearest, "nearest"}})
enum TexWrap { repeat, mirrored_repeat, clamp_to_edge, clamp_to_border };
NLOHMANN_JSON_SERIALIZE_ENUM(TexWrap,
                             {{TexWrap::repeat, "repeat"},
                              {TexWrap::mirrored_repeat, "mirrored_repeat"},
                              {TexWrap::clamp_to_edge, "clamp_to_edge"},
                              {TexWrap::clamp_to_border, "clamp_to_border"}})
struct TextureJson {
public:
  TexFiltering filterType;
  TexWrap wrapS;
  TexWrap wrapT;
  std::string path;
  bool translucent;
  bool mipmap;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextureJson, filterType, wrapS, wrapT, path,
                                   translucent, mipmap);

class Texture : public IJson {
public:
  json ToJson() override;
  void FromJson(json &js) override;

  Texture(std::string json_path, int fallback);
  /// Texture::Create
  /// Checks the cache to avoid loading one texture more than once, using its
  /// path as the index
  static std::shared_ptr<Texture> Create(std::string json_path, int fallback);
  unsigned int texture = -1;

private:
  std::string path;
  std::string texture_path;
  TexFiltering filterType = TexFiltering::linear;
  bool translucent = false;
  bool mipmap = true;
  TexWrap wrapS = TexWrap::repeat;
  TexWrap wrapT = TexWrap::repeat;
};
} // namespace Engine::Graphics
