//
// Created by malan on 20.04.2026.
//
#pragma once
#include "../Config/Config.hpp"
#include "Material.hpp"
#include "Object.hpp"
#include "Program.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include "Util/LoggerUtil.hpp"
#include <chrono>
#ifdef IMGUI
#endif
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <unordered_map>
namespace Engine::Graphics {
class Texture;
#ifdef DEBUG
#define CHECK_GL_ERROR()                                                       \
  {                                                                            \
    GLenum err;                                                                \
    while ((err = glGetError()) != GL_NO_ERROR) {                              \
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "OpenGL Error: {}", err);        \
    }                                                                          \
  }
#else
#define CHECK_GL_ERROR()
#endif
class Main {
public:
  static std::shared_ptr<Main> instance;
  static int Init(std::shared_ptr<Config> config);
  static int
  Tick(std::chrono::duration<float, std::chrono::seconds::period> dur_other,
       std::chrono::duration<float, std::chrono::seconds::period> dur_graphics);
  void Terminate();

  Shader GetShader(ShaderType type, std::string source);
  Program NewProgram(int count, ...);
  static std::unordered_map<std::string, std::shared_ptr<Shader>> vertexShaders;
  static std::unordered_map<std::string, std::shared_ptr<Shader>>
      fragmentShaders;
  static std::unordered_map<std::string, std::shared_ptr<Material>> materials;
  // Inputs
  static void SetKeyCallback(const int key,
                             std::function<void(int action, int mods)> action);
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
  static std::shared_ptr<Texture> FallbackTexture;
  std::map<int, std::vector<std::function<void(int, int)>>> keyMap = {};
  std::shared_ptr<std::vector<void (*)()>> init =
      std::make_shared<std::vector<void (*)()>>();
  std::shared_ptr<std::vector<void (*)()>> terminate =
      std::make_shared<std::vector<void (*)()>>();
  std::shared_ptr<std::vector<void (*)()>> preRender =
      std::make_shared<std::vector<void (*)()>>();
  std::shared_ptr<std::vector<void (*)()>> postRender =
      std::make_shared<std::vector<void (*)()>>();

  GLFWwindow *window = nullptr;
  std::vector<float> frameTimesGraphics = {};
  float dur_graphics_total = 0;
  std::vector<float> frameTimesOther = {};
  float dur_other_total = 0;
  float dur_largest = 0;

private:
  std::unique_ptr<Engine::Object> camera = nullptr;
  std::shared_ptr<Config> config = nullptr;
};
}; // namespace Engine::Graphics
