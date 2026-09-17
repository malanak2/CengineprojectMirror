//
// Created by malan on 21.04.2026.
//

#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
namespace Engine::Graphics {
enum ShaderType {
  Invalid,
  Vertex,
  Fragment,
};
NLOHMANN_JSON_SERIALIZE_ENUM(ShaderType, {
                                             {Invalid, nullptr},
                                             {Vertex, "vertex"},
                                             {Fragment, "fragment"},
                                         })

//!
//! @brief Class for handling shaders
//!
class Shader {
public:
  ShaderType type;
  unsigned int id = 0;
  std::string path = "";
  bool isValid = false;
  //!
  //! @brief Creates the shader + Cache
  //!
  //! @param[in] type Type of the shader
  //! @param[in] path Path to the definition
  //! @param[in] entrypoint name of the EntryPoint func
  //! @param[in] reusable If it is reusable - if it should be cached / destroyed
  //!
  static std::shared_ptr<Shader> Create(ShaderType type, std::string path,
                                        std::string entrypoint,
                                        bool reusable = false);
  ~Shader();
  bool _reusable = false;
  void Delete();

  Shader(ShaderType type, std::string path, std::string entrypoint,
         bool reusable = false);
};
} // namespace Engine::Graphics
