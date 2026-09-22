//
// Created by malan on 20.04.2026.
//

#include "Graphics.hpp"
#include "Config/Config.hpp"
#include "Engine.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Interfaces/IComponent.hpp"
#include "Material.hpp"
#include "Scene.hpp"
#include "Texture.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include "tracy/TracyOpenGL.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <ctime>
#include <memory>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>
#include <unordered_map>
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  Engine::Engine::width = width;
  Engine::Engine::height = height;
}

void glfw_error_callback(int error, const char *description) {
  SPDLOG_LOGGER_ERROR(spdlog::get("console"), "GLFW Error ({}): {}", error,
                      description);
}

namespace Engine::Graphics {

std::shared_ptr<Texture> Main::FallbackTexture = nullptr;
// Initialize vecotrs
std::unordered_map<std::string, std::shared_ptr<Shader>> Main::vertexShaders =
    {};
std::unordered_map<std::string, std::shared_ptr<Shader>> Main::fragmentShaders =
    {};
std::unordered_map<std::string, std::shared_ptr<Material>> Main::materials = {};
std::shared_ptr<Main> Main::instance = std::make_shared<Main>();

int Main::Init(std::shared_ptr<Config> config) {

  ZoneScoped;
  auto logger = ENGINE_UTIL_LOGGER;
  glfwSetErrorCallback(glfw_error_callback);
  instance->config = config;
  SPDLOG_LOGGER_INFO(logger, "Initializing GLFW...");
  if (!glfwInit()) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to initialize GLFW.");
    return -1;
  }
  SPDLOG_LOGGER_INFO(logger, "GLFW initialized.");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  SPDLOG_LOGGER_INFO(logger, "Creating GLFW window (800x600, title: {})...",
                     config->window->title);
  instance->window =
      glfwCreateWindow(800, 600, config->window->title.c_str(), NULL, NULL);
  Engine::Engine::width = 800;
  Engine::Engine::height = 600;
  if (instance->window == NULL) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to create GLFW window.");
    glfwTerminate();
    return -1;
  }
  SPDLOG_LOGGER_INFO(logger, "GLFW window created successfully.");
  glfwMakeContextCurrent(instance->window);

  if (config->graphics->enableVsync) {
    glfwSwapInterval(1);
  } else {
    glfwSwapInterval(0);
  }

  for (auto f : *instance->init) {
    f();
  }

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    SPDLOG_LOGGER_ERROR(spdlog::get("console"), "Failed to initialize GLAD.");
    return -1;
  }

  TracyGpuContext;
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(instance->window, framebuffer_size_callback);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  if (config->graphics->enableAntiAliasing) {
    glEnable(GL_MULTISAMPLE);
  }

  // Load fallback texture
  FallbackTexture = std::make_shared<Texture>(
      Config::inst->graphics->texturePath + "/fallback.json");
  if (FallbackTexture->texture == -1) {
    CHECK_GL_ERROR();
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to load fallback texture.");
    return 1;
  }
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Loaded fallback texture to {}",
                     FallbackTexture->texture);
  auto func = &Main::keyCallback;
  glfwSetKeyCallback(instance->window, func);
  // Load fallback Texture
  CHECK_GL_ERROR();
  // TODO: Remove
  return 0;
}

int Main::Tick(

    std::chrono::duration<float, std::chrono::seconds::period> dur_other,
    std::chrono::duration<float, std::chrono::seconds::period> dur_graphics) {
  ZoneScoped;
  if (glfwWindowShouldClose(instance->window)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "GLFW Window should close.");
    return -1;
  }

  {
    ZoneScopedN("Clear");
    TracyGpuZone("Clear");
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERROR();
  }
#ifndef IMGUI
  float total = 0;
  for (auto var : frameTimes) {
    total += var;
  }
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Fps: {}", frameTimes.size() / total);
#endif
  {
    ZoneScopedN("PreRender");
    for (auto f : *instance->preRender) {
      f();
    }
  }
  {
    TracyGpuZone("Rendering");
    ZoneScopedN("Rendering objects");
    for (auto &[key, val] : materials) {
      ZoneScopedN("Material");
      ZoneText(key.c_str(), strlen(key.c_str()));
      TracyGpuZoneTransient(__gpu_mat_zone, key.c_str(), true);
      val->SetupMaterial();
      val->RenderObjects();
    }
  }
  {
    ZoneScopedN("PostRender");
    for (auto f : *instance->postRender) {
      f();
    }
  }

  CHECK_GL_ERROR();
  {
    ZoneScopedNC("VSync", 0x111111);
    TracyGpuZone("SwapBuffers");
    glfwSwapBuffers(instance->window);
  }
  TracyGpuCollect;
  FrameMark;
  glfwPollEvents();

  return 0;
}

void Main::Terminate() {
  materials.clear();
  vertexShaders.clear();
  fragmentShaders.clear();
  for (auto f : *instance->terminate) {
    f();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Main::keyCallback(GLFWwindow *window, int key, int scancode, int action,
                       int mods) {
  ZoneScoped;
  if (instance->keyMap.contains(scancode)) {
    for (auto var : instance->keyMap[scancode]) {
      var(action, mods);
    }
  }
}

void Main::SetKeyCallback(const int key, std::function<void(int, int)> action) {
  const int scancode = glfwGetKeyScancode(key);
  if (!instance->keyMap.contains(scancode)) {
    instance->keyMap[scancode] = std::vector<std::function<void(int, int)>>{};
  }
  instance->keyMap[scancode].insert(instance->keyMap[scancode].end(), action);
}
} // namespace Engine::Graphics
