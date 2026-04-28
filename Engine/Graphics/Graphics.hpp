//
// Created by malan on 20.04.2026.
//
#pragma once
#include "../Config/Config.hpp"
#include "Material.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <unordered_map>
namespace Graphics {

#ifdef DEBUG
#define CHECK_GL_ERROR()                                                       \
  {                                                                            \
    GLenum err;                                                                \
    while ((err = glGetError()) != GL_NO_ERROR) {                              \
      SPDLOG_LOGGER_ERROR(spdlog::get("console"),                              \
                          "OpenGL Error: {} at {}:{}\n", err, __FILE__,        \
                          __LINE__);                                           \
    }                                                                          \
  }
#else
#define CHECK_GL_ERROR
#endif
class Main {
public:
  int Init(Config *config);
  int Tick();
  void Terminate();

  Shader GetShader(ShaderType type, std::string source);
  Program NewProgram(int count, ...);
  static std::unordered_map<std::string, std::shared_ptr<Shader>> vertexShaders;
  static std::unordered_map<std::string, std::shared_ptr<Shader>>
      fragmentShaders;
  static std::unordered_map<std::string, std::shared_ptr<Material>> materials;

private:
  GLFWwindow *window = nullptr;
  // TODO: Remove
  std::shared_ptr<Material> material = nullptr;
  unsigned int vao = 0;
};
}; // namespace Graphics
