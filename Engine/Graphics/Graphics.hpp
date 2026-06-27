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
#include "ImGuiMacros.hpp"
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
  int Init(std::shared_ptr<Config> config);
  int Tick(
#ifdef IMGUI
      std::shared_ptr<Scene> scene,
#endif
      std::chrono::duration<float, std::chrono::seconds::period> dur_other,
      std::chrono::duration<float, std::chrono::seconds::period> dur_graphics);
  void Terminate();

  Shader GetShader(ShaderType type, std::string source);
  Program NewProgram(int count, ...);
  static std::unordered_map<std::string, std::shared_ptr<Shader>> vertexShaders;
  static std::unordered_map<std::string, std::shared_ptr<Shader>>
      fragmentShaders;
  static std::unordered_map<std::string, std::shared_ptr<Material>> materials;
  // Inputs
  void SetKeyCallback(const int key, std::function<void(int action, int mods)> action);
  void keyCallback(int key, int scancode, int action, int mods);
  static void keyCallbackStatic(GLFWwindow *window, int key, int scancode,
                                int action, int mods);
  static std::shared_ptr<Texture> FallbackTexture;

private:
  GLFWwindow *window = nullptr;
  std::unique_ptr<Engine::Object> camera = nullptr;
  std::vector<float> frameTimesGraphics = {};
  float dur_graphics_total = 0;
  std::vector<float> frameTimesOther = {};
  float dur_other_total = 0;
  float dur_largest = 0;
  std::shared_ptr<Config> config = nullptr;

  std::map<int, std::vector<std::function<void(int, int)>>> keyMap = {};

#ifdef IMGUI
  std::shared_ptr<SceneObject> sceneObject = nullptr;
  void
  ShowSceneObjectMenu(std::vector<std::shared_ptr<SceneObject>> *sceneObjects);
  void RenderSceneView(std::shared_ptr<Scene> scene);
  void RenderPerformanceGraph();
  void RenderComponentInspector(std::shared_ptr<IComponent> component,
                                ENGINE_COMPONENT_TYPE type);
  void RenderObjectInspector();
  std::shared_ptr<SceneObject> newObjectParent = nullptr;
  bool wasSavePressedThisFrame = false;
  char namebuf[64] = "";
  char matbuf[64] = "";
  float coords[3] = {0, 0, 0};
  float rotation[4] = {0, 0, 0, 0};
#endif
};
}; // namespace Engine::Graphics
