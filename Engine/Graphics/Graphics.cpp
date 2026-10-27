//
// Created by malan on 20.04.2026.
//

#include "Graphics.hpp"
#include "Config/Config.hpp"
#include "Engine.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Material.hpp"
#include "Physics/PhysicsSystem.hpp"
#include "Scene.hpp"
#include "Texture.hpp"
#include "Util/LoggerUtil.hpp"
#include "tracy/TracyOpenGL.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <ctime>
#include <glm/gtc/type_ptr.hpp>
#ifdef IMGUI
#include <imgui_impl_glfw.h>
#endif
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
std::unordered_map<std::string, std::shared_ptr<Texture>> Main::textures = {};
std::shared_ptr<Main> Main::instance = std::make_shared<Main>();

std::shared_ptr<Material> Main::debugMaterial = nullptr;
unsigned int Main::debugUBO = 0;
unsigned int Main::debugVBO = 0;
unsigned int Main::debugEBO = 0;

void Main::ReleaseDebugResources() {
  if (debugMaterial) {
    if (debugMaterial->vao != 0) {
      glDeleteVertexArrays(1, &debugMaterial->vao);
      debugMaterial->vao = 0;
    }
    debugMaterial.reset();
  }
  if (debugUBO != 0) {
    glDeleteBuffers(1, &debugUBO);
    debugUBO = 0;
  }
  if (debugVBO != 0) {
    glDeleteBuffers(1, &debugVBO);
    debugVBO = 0;
  }
  if (debugEBO != 0) {
    glDeleteBuffers(1, &debugEBO);
    debugEBO = 0;
  }
}
int Main::Init(const std::shared_ptr<Config> &config) {

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
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
  SPDLOG_LOGGER_INFO(logger, "Creating GLFW window (Fullscreen, title: {})...",
                     config->window->title);
  instance->window =
      glfwCreateWindow(800, 600, config->window->title.c_str(), NULL, NULL);

  glfwGetWindowSize(instance->window, &Engine::Engine::width,
                    &Engine::Engine::height);

  SPDLOG_LOGGER_INFO(logger, "Glfw window created, {}x{}",
                     Engine::Engine::width, Engine::Engine::height);
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

  for (const auto &f : *instance->init) {
    f();
  }

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    SPDLOG_LOGGER_ERROR(spdlog::get("console"), "Failed to initialize GLAD.");
    return -1;
  }

  TracyGpuContext;
  glViewport(0, 0, Engine::Engine::width, Engine::Engine::height);
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
  {
    ZoneScopedN("PreRender");
    for (const auto &f : *instance->preRender) {
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
  RenderBBOS();
  {
    ZoneScopedN("PostRender");
    for (const auto &f : *instance->postRender) {
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
  if (auto engine = Engine::Engine::instance; engine && engine->current_scene) {
    std::function<void(const std::shared_ptr<SceneObject> &)> endAll =
        [&](const std::shared_ptr<SceneObject> &so) {
          if (!so)
            return;
          auto children = so->Children;
          for (auto &child : children)
            endAll(child);
          so->End();
        };
    auto roots = engine->current_scene->objects;
    for (auto &so : roots)
      endAll(so);
    engine->current_scene.reset();
  }
  ReleaseDebugResources();
  materials.clear();
  textures.clear();
  FallbackTexture.reset();
  Model::models.clear();
  vertexShaders.clear();
  fragmentShaders.clear();
  for (const auto &f : *instance->terminate)
    f();
  glfwDestroyWindow(window);
  window = nullptr;
  glfwTerminate();
}

void Main::keyCallback(GLFWwindow *window, int key, int scancode, int action,
                       int mods) {
  ZoneScoped;
  if (instance->keyMap.contains(scancode)) {
    for (const auto &var : instance->keyMap[scancode]) {
      var(action, mods);
    }
  }
}
void Main::RenderBBOS() {
  if (Config::inst->graphics->renderBoundingBox) {
    ZoneScopedN("BoundingBoxRender");
    bool useFrustum = false;
    Frustum frustum;
    if (auto obj = Engine::instance->current_scene->FindObject("fakecam")) {
      if (auto c = obj->GetComponent<CameraComponent>()) {
        useFrustum = true;
        frustum = c->GetFrustum();
      }
    }
    if (!debugMaterial)
      debugMaterial = Material::Create("materials/bounding_box.json");
    auto &debugmat = debugMaterial;
    if (!debugmat || !debugmat->usable) {
      static bool warned = false;
      if (!warned) {
        warned = true;
        SPDLOG_LOGGER_ERROR(
            ENGINE_UTIL_LOGGER,
            "Bounding box material is not usable (are the shaders generated?)");
      }
      return;
    }
    if (debugUBO == 0) {
      glGenBuffers(1, &debugUBO);
      glBindBuffer(GL_UNIFORM_BUFFER, debugUBO);
      glBindBuffer(GL_UNIFORM_BUFFER, debugUBO);
      glBufferData(GL_UNIFORM_BUFFER, 160, nullptr, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    if (debugmat->vao == 0) {
      glGenVertexArrays(1, &debugmat->vao);
      glBindVertexArray(debugmat->vao);

      float vertices[] = {-1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f, 1.0f,
                          -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
                          -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f, 1.0f};
      unsigned int indices[] = {
          0, 1, 1, 2, 2, 3, 3, 0, // Bottom face
          4, 5, 5, 6, 6, 7, 7, 4, // Top face
          0, 4, 1, 5, 2, 6, 3, 7  // Vertical edges
      };

      GLuint vbo, ebo;
      glGenBuffers(1, &vbo);
      glGenBuffers(1, &ebo);

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                   GL_STATIC_DRAW);

      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                            (void *)0);
    }

    debugmat->SetupMaterial();

    auto scene = Engine::Engine::instance
                     ? Engine::Engine::instance->current_scene
                     : nullptr;
    if (scene && scene->camera) {
      auto proj = scene->camera->GetProjMatrix();
      auto view = scene->camera->GetViewMatrix();
      float color[4] = {0.0f, 1.0f, 0.0f, 1.0f};
      float color_trigger[4] = {0.0f, 0.0f, 1.0f, 1.0f};

      glBindBuffer(GL_UNIFORM_BUFFER, debugUBO);
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                      glm::value_ptr(proj));
      glBufferSubData(GL_UNIFORM_BUFFER, 64, sizeof(glm::mat4),
                      glm::value_ptr(view));
      glBufferSubData(GL_UNIFORM_BUFFER, 128, sizeof(color), color);
      glBufferSubData(GL_UNIFORM_BUFFER, 144, sizeof(color_trigger),
                      color_trigger);

      glBindBufferBase(GL_UNIFORM_BUFFER, 0, debugUBO);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    struct InstanceData {
      glm::mat4 modelMatrix;
      uint32_t isTrigger;
      float padding[3];
    };
    std::vector<InstanceData> instances;
    instances.reserve(Physics::PhysicsSystem::boundingBoxes.size() + 1);

    for (const auto &bb : Physics::PhysicsSystem::boundingBoxes) {
      if (!bb)
        continue;
      auto obb = bb->GetOBB();
      if (!obb)
        continue;
      glm::mat4 translation = glm::translate(glm::mat4(1.0f), obb->position);
      glm::mat4 rotation = glm::mat4(obb->orientation);
      glm::mat4 scale = glm::scale(glm::mat4(1.0f), obb->size);
      InstanceData data{};
      data.modelMatrix = translation * rotation * scale;
      data.isTrigger = bb->isTrigger ? 1 : 0;
      data.padding[0] = 0;
      data.padding[1] = 0;
      data.padding[2] = 0;
      instances.push_back(data);
    }
    if (useFrustum) {
      if (auto c = Engine::instance->current_scene->camera) {
        InstanceData frustumInstance{};
        frustumInstance.modelMatrix = c->GetFrustumMatrix();
        frustumInstance.isTrigger = 1;
        frustumInstance.padding[0] = 0;
        frustumInstance.padding[1] = 0;
        frustumInstance.padding[2] = 0;

        instances.push_back(frustumInstance);
      }
    }

    if (!instances.empty()) {
      if (debugmat->instanceSSBO == 0) {
        glGenBuffers(1, &debugmat->instanceSSBO);
      }

      glBindBuffer(GL_SHADER_STORAGE_BUFFER, debugmat->instanceSSBO);
      // Allocate/reallocate storage for all bounding boxes in this frame
      glBufferData(GL_SHADER_STORAGE_BUFFER,
                   instances.size() * sizeof(InstanceData), instances.data(),
                   GL_DYNAMIC_DRAW);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, debugmat->instanceSSBO);
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
      glDisable(GL_CULL_FACE);
      glDepthMask(GL_FALSE);
      glBindVertexArray(debugmat->vao);
      glDrawElementsInstanced(GL_LINES, 24, GL_UNSIGNED_INT, nullptr,
                              static_cast<GLsizei>(instances.size()));
      glBindVertexArray(0);
      glDepthMask(GL_TRUE);
      glEnable(GL_CULL_FACE);
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
