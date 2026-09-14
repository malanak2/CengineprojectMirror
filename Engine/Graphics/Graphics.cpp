//
// Created by malan on 20.04.2026.
//

#include "Graphics.hpp"
#include "Config/Config.hpp"
#include "Engine.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "ImGuiMacros.hpp"
#include "Interfaces/IComponent.hpp"
#include "Material.hpp"
#include "Scene.hpp"
#include "Texture.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <ctime>
#include <memory>
#include <spdlog/spdlog.h>
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
      Config::inst->graphics->texturePath + "/fallback.json", -1);
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

/*
void Main::ShowSceneObjectMenu(
    std::vector<std::shared_ptr<SceneObject>> *sceneObjects) {
  if (!sceneObjects)
    return;
  for (auto &obj : *sceneObjects) {
    if (!obj || !obj->instance)
      continue;
    ImGui::PushID(obj.get());

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap;
    if (obj->Children.empty()) {
      flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool open = ImGui::TreeNodeEx(obj->instance->_name.c_str(), flags);

    if (ImGui::BeginDragDropSource()) {
      SceneObject *ptr = obj.get();
      ImGui::SetDragDropPayload("OBJ_PARENT", &ptr, sizeof(SceneObject *));
      ImGui::Text("%s", obj->instance->_name.c_str());
      ImGui::EndDragDropSource();
    }

    if (instance->sceneObject != obj) {
      ImGui::SameLine(ImGui::GetContentRegionAvail().x - 50);
      if (ImGui::Button("Select")) {
        instance->sceneObject = obj;
      }
    }

    if (open) {
      ShowSceneObjectMenu(&obj->Children);
      ImGui::TreePop();
    }

    ImGui::PopID();
  }
}

void Main::RenderSceneView(std::shared_ptr<Scene> scene) {
  ImGui::Begin("Scene");
  ImGui::Text("Edit current scene");
  IMGUI_CHECKBOX("Wireframe mode", false, [](bool state) {
    if (state) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  });
  IMGUI_CHECKBOX("AntiAliasing", config->graphics->enableAntiAliasing,
                 [](bool state) {
                   if (state) {
                     glEnable(GL_MULTISAMPLE);
                   } else {
                     glDisable(GL_MULTISAMPLE);
                   }
                 });
  IMGUI_CHECKBOX("Vsync", config->graphics->enableVsync, [](bool state) {
    if (state) {
      glfwSwapInterval(1);
    } else {
      glfwSwapInterval(0);
    }
  });
  if (ImGui::CollapsingHeader("Objects")) {
    ShowSceneObjectMenu(&scene->objects);
  }
  if (sceneObject != nullptr) {
    ImGui::Text("Scene object %s", sceneObject->instance->_name.c_str());
    if (ImGui::Button("Deselect"))
      sceneObject = nullptr;
    else {
      ImGui::Text("Parent: %s",
                  sceneObject->Parent.lock()
                      ? sceneObject->Parent.lock()->instance->_name.c_str()
                      : "nullptr");
      if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload =
                ImGui::AcceptDragDropPayload("OBJ_PARENT")) {
          SceneObject *draggedObj = *(SceneObject **)payload->Data;
          if (draggedObj->shared_from_this() != sceneObject) {
            sceneObject->SetParent(draggedObj->shared_from_this());
          }
        }
        ImGui::EndDragDropTarget();
      }
    }
  } else {
    ImGui::Text("New Scene Object");
    ImGui::InputText("Name", namebuf, 64);
    ImGui::InputText("Material path", matbuf, 64);
    ImGui::InputFloat3("Position: ", coords);
    ImGui::InputFloat4("Rotation: ", coords);
    ImGui::Text("Parent: %s", newObjectParent
                                  ? newObjectParent->instance->_name.c_str()
                                  : "nullptr");
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("OBJ_PARENT")) {
        SceneObject *draggedObj = *(SceneObject **)payload->Data;
        newObjectParent = draggedObj->shared_from_this();
      }
      ImGui::EndDragDropTarget();
    }
    if (newObjectParent != nullptr) {
      ImGui::SameLine();
      if (ImGui::Button("Remove")) {
        newObjectParent = nullptr;
      }
    }
    if (ImGui::Button("Create")) {
      auto o = std::make_shared<Object>(scene);
      std::vector<float> pos = {};
      std::vector<float> rot = {};
      pos.assign(coords, coords + sizeof(coords) / sizeof(float));
      rot.assign(rotation, rotation + sizeof(rotation) / sizeof(float));
      o->fromParams(namebuf, {}, pos, rot);
      scene->Instantiate(o, newObjectParent);
    }
  }
  ImGui::Text("File scene controls");
  ImGui::InputText("Scene location", &scene->path[0], 100);

  if (ImGui::Button("Save scene")) {
    if (!wasSavePressedinstanceFrame) {
      wasSavePressedinstanceFrame = true;
      SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Saving scene to {}...",
                         scene->path);
      json scenejs = scene->ToJson();
      auto st = scenejs.dump();
      FileUtil::SaveFile(scene->path, &st);
    }
  } else {
    wasSavePressedinstanceFrame = false;
  }

  ImGui::End();

}
void Main::RenderPerformanceGraph() {
  ImGui::Begin("Performance");
  float dur_total = dur_graphics_total + dur_other_total;
  ImGui::Text("Fps: %f (total: %f, count: %zu)",
              frameTimesGraphics.size() / dur_total, dur_total,
              frameTimesGraphics.size());
  if (ImGui::CollapsingHeader("Graph")) {
    const char *groups[] = {"Graphics", "Other"};
    std::vector<float> total = {};
    for (size_t i = 0; i < frameTimesGraphics.size(); ++i) {
      total.push_back(frameTimesGraphics[i] + frameTimesOther[i]);
    }
    if (ImPlot::BeginPlot("Frame Times")) {
      ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels,
                        ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, frameTimesGraphics.size(),
                              ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, 0, dur_largest * 1.5,
                              ImGuiCond_Always);
      ImPlot::PlotLine("fps", &total[0], total.size());
      ImPlot::EndPlot();
    }
    if (ImPlot::BeginPlot("Frame times distribution")) {
      ImPlotPieChartFlags flags = 0 | ImPlotPieChartFlags_Normalize;
      const char *titles[] = {"Graphics", "Other"};
      ImPlot::PlotPieChart(
          titles,
          std::vector<float>{dur_graphics_total / frameTimesGraphics.size(),
                             dur_other_total / frameTimesOther.size()}
              .data(),
          2, 0, 0, 10, "%.2f", 90, {ImPlotProp_Flags, flags});
      ;
      ImPlot::EndPlot();
    }
  }

  ImGui::End();
}

void Main::RenderObjectInspector() {
  ImGui::Begin("Object inspetor");
  if (instance->sceneObject == nullptr) {
    ImGui::Text("Please select an object");
    ImGui::End();
    return;
  }
  char buf[64] = {0};
  strncpy(buf, sceneObject->instance->_name.c_str(), 63);

  if (ImGui::InputText("Name:", buf, 64)) {
    sceneObject->instance->_name = buf; // instance correctly updates the size
  }
  if (ImGui::CollapsingHeader("Values")) {
    auto obj = sceneObject->instance;
    if (obj) {
      // Position:
      ImGui::InputFloat3("XYZ", &obj->_position[0]);
      // Rotation:
      ImGui::SliderFloat3("Rotations:", &obj->_rotation[0], -360, 360);
      ImGui::InputFloat3(": Rotation", &obj->_rotation[0]);
    }
  }
  for (auto [type, comp] : sceneObject->instance->_components) {
    if (comp != nullptr)
      if (ImGui::CollapsingHeader(comp->GetName().c_str())) {
        comp->RenderImGui();
      }
  }
  ImGui::End();
}
*/

int Main::Tick(
#ifdef IMGUI
    std::shared_ptr<Scene> scene,
#endif
    std::chrono::duration<float, std::chrono::seconds::period> dur_other,
    std::chrono::duration<float, std::chrono::seconds::period> dur_graphics) {
  if (glfwWindowShouldClose(instance->window)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "GLFW Window should close.");
    return -1;
  }

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  CHECK_GL_ERROR();
  instance->frameTimesGraphics.insert(instance->frameTimesGraphics.end(),
                                      dur_graphics.count());
  instance->dur_graphics_total += dur_graphics.count();
  if (instance->dur_largest < dur_graphics.count())
    instance->dur_largest = dur_graphics.count();
  if (instance->frameTimesGraphics.size() > 1000) {
    instance->dur_graphics_total -= instance->frameTimesGraphics[0];
    if (instance->dur_largest == instance->frameTimesGraphics[0]) {
      instance->dur_largest = 0;
      for (auto var : instance->frameTimesGraphics) {
        if (var > instance->dur_largest)
          instance->dur_largest = var;
      }
      for (auto var : instance->frameTimesOther) {
        if (var > instance->dur_largest)
          instance->dur_largest = var;
      }
    }
    instance->frameTimesGraphics.erase(instance->frameTimesGraphics.begin(),
                                       instance->frameTimesGraphics.begin() +
                                           1);
  }
  instance->frameTimesOther.insert(instance->frameTimesOther.end(),
                                   dur_other.count());
  instance->dur_other_total += dur_other.count();
  if (instance->dur_largest < dur_other.count())
    instance->dur_largest = dur_other.count();
  if (instance->frameTimesOther.size() > 2000) {
    instance->dur_other_total -= instance->frameTimesOther[0];
    if (instance->dur_largest == instance->frameTimesOther[0]) {
      instance->dur_largest = 0;
      for (auto var : instance->frameTimesGraphics) {
        if (var > instance->dur_largest)
          instance->dur_largest = var;
      }
      for (auto var : instance->frameTimesOther) {
        if (var > instance->dur_largest)
          instance->dur_largest = var;
      }
    }
    instance->frameTimesOther.erase(instance->frameTimesOther.begin(),
                                    instance->frameTimesOther.begin() + 1);
  }

#ifndef IMGUI
  float total = 0;
  for (auto var : frameTimes) {
    total += var;
  }
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Fps: {}", frameTimes.size() / total);
#endif
  for (auto f : *instance->preRender) {
    f();
  }
  for (auto &[key, val] : materials) {
    val->SetupMaterial();
    val->RenderObjects();
  }
  for (auto f : *instance->postRender) {
    f();
  }

  CHECK_GL_ERROR();
  glfwSwapBuffers(instance->window);
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
