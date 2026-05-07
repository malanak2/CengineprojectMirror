//
// Created by malan on 20.04.2026.
//

#include "Graphics.hpp"
#include "Engine.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Interfaces/IComponent.hpp"
#include "Material.hpp"
#include "Scene.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <ctime>
#include <memory>
#include <spdlog/spdlog.h>
#include <unordered_map>

#ifdef IMGUI
#include "implot.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#define IMGUI_SCALE 1
#endif

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  Engine::Main::width = width;
  Engine::Main::height = height;
}

void glfw_error_callback(int error, const char *description) {
  SPDLOG_LOGGER_ERROR(spdlog::get("console"), "GLFW Error ({}): {}", error,
                      description);
}

namespace Engine {
namespace Graphics {
// Initialize vecotrs
std::unordered_map<std::string, std::shared_ptr<Shader>> Main::vertexShaders =
    {};
std::unordered_map<std::string, std::shared_ptr<Shader>> Main::fragmentShaders =
    {};
std::unordered_map<std::string, std::shared_ptr<Material>> Main::materials = {};

int Main::Init(Config *config) {
  auto logger = ENGINE_UTIL_LOGGER;
  glfwSetErrorCallback(glfw_error_callback);
  SPDLOG_LOGGER_INFO(logger, "Initializing GLFW...");
  if (!glfwInit()) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to initialize GLFW.");
    return -1;
  }
  SPDLOG_LOGGER_INFO(logger, "GLFW initialized.");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  SPDLOG_LOGGER_INFO(logger, "Creating GLFW window (800x600, title: {})...",
                     config->window->title);
  window =
      glfwCreateWindow(800, 600, config->window->title.c_str(), NULL, NULL);
  Engine::Main::width = 800;
  Engine::Main::height = 600;
  if (window == NULL) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to create GLFW window.");
    glfwTerminate();
    return -1;
  }
  SPDLOG_LOGGER_INFO(logger, "GLFW window created successfully.");
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

#ifdef IMGUI
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "IMGUI initializing");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

  ImGui::StyleColorsDark();

  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(IMGUI_SCALE);
  style.FontScaleDpi = IMGUI_SCALE;
  if (io.ConfigFlags) { // ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");
#endif

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    SPDLOG_LOGGER_ERROR(spdlog::get("console"), "Failed to initialize GLAD.");
    return -1;
  }
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  auto func = &Main::keyCallbackStatic;
  glfwSetKeyCallback(window, func);
  CHECK_GL_ERROR();
  // TODO: Remove
  return 0;
}
void Main::keyCallbackStatic(GLFWwindow *window, int key, int scancode,
                             int action, int mods) {
  // Retrieve the instance pointer we stored earlier
  Main *instance = static_cast<Main *>(glfwGetWindowUserPointer(window));
  if (instance) {
    instance->keyCallback(key, scancode, action, mods);
  }
}

#ifdef IMGUI
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

    /*if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload =
    ImGui::AcceptDragDropPayload("OBJ_PARENT")) { SceneObject* draggedObjRaw =
    *(SceneObject**)payload->Data; std::shared_ptr<SceneObject> draggedObj =
    draggedObjRaw->shared_from_this(); if (draggedObj != obj) {
                draggedObj->Parent = obj;
                // Note: Real re-parenting should also update Children vectors
            }
        }
        ImGui::EndDragDropTarget();
    }*/

    if (this->sceneObject != obj) {
      ImGui::SameLine(ImGui::GetContentRegionAvail().x - 50);
      if (ImGui::Button("Select")) {
        this->sceneObject = obj;
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
    if (!wasSavePressedThisFrame) {
      wasSavePressedThisFrame = true;
      SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Saving scene to {}...",
                         scene->path);
      json scenejs = scene->ToJson();
      auto st = scenejs.dump();
      FileUtil::SaveFile(scene->path, &st);
    }
  } else {
    wasSavePressedThisFrame = false;
  }
  ImGui::End();
}
void Main::RenderPerformanceGraph() {
  ImGui::Begin("Performance");

  float total = 0;
  float largest = 0;
  for (auto var : frameTimes) {
    if (var > largest)
      largest = var;
    total += var;
  }
  ImGui::Text("Fps: %f (total: %f, count: %zu)", frameTimes.size() / total,
              total, frameTimes.size());
  if (ImGui::CollapsingHeader("Graph")) {
    if (ImPlot::BeginPlot("Frame Times")) {
      ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels,
                        ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, frameTimes.size(),
                              ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, 0, largest * 1.5, ImGuiCond_Always);
      ImPlot::PlotLine("ms", &frameTimes[0], frameTimes.size());
      ImPlot::EndPlot();
    }
  }

  ImGui::End();
}
void Main::RenderComponentInspector(std::shared_ptr<IComponent> component,
                                    ENGINE_COMPONENT_TYPE type) {
  switch (type) {
  case Engine::ENGINE_COMPONENT_TYPE::camera: {
    if (ImGui::CollapsingHeader("Camera")) {
    }
    break;
  }
  case Engine::ENGINE_COMPONENT_TYPE::renderable: {
    if (ImGui::CollapsingHeader("Renderable")) {
      // Render uniforms
      auto c = std::static_pointer_cast<ComponentRenderable>(component);
      ImGui::InputText("Material path", &c->_material_path[0], 100);

      if (ImGui::CollapsingHeader("Uniforms")) {
        for (auto &[key, val] : c->_uniforms) {
          switch (val.size()) {
          case 1: {
            ImGui::InputFloat(&key[0], &val[0]);
            break;
          }
          case 2: {
            ImGui::InputFloat2(&key[0], &val[0]);
            break;
          }
          case 3: {
            ImGui::InputFloat3(&key[0], &val[0]);
            break;
          }
          case 4: {
            if (key == "color") {
              ImGui::ColorPicker4(&key[0], &val[0]);
            } else {
              ImGui::InputFloat4(&key[0], &val[0]);
            }
            break;
          }
          default: {
            ImGui::Text("Unsupported float uniform %s of length %zu", &key[0],
                        val.size());
          }
          }
        }
      }
    }
  }
  }
}

void Main::RenderObjectInspector() {
  ImGui::Begin("Object inspetor");
  if (this->sceneObject == nullptr) {
    ImGui::Text("Please select an object");
    ImGui::End();
    return;
  }
  char buf[64] = {0};
  strncpy(buf, sceneObject->instance->_name.c_str(), 63);

  if (ImGui::InputText("Name:", buf, 64)) {
    sceneObject->instance->_name = buf; // This correctly updates the size
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
      RenderComponentInspector(comp, type);
  }
  ImGui::End();
}
#endif

int Main::Tick(
#ifdef IMGUI
    std::shared_ptr<Scene> scene,
#endif
    std::chrono::duration<float, std::chrono::seconds::period> duration) {
  if (glfwWindowShouldClose(window)) {
    SPDLOG_LOGGER_INFO(spdlog::get("console"), "GLFW Window should close.");
    return -1;
  }

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  CHECK_GL_ERROR();
  frameTimes.insert(frameTimes.end(), duration.count());
  if (frameTimes.size() > 2000)
    frameTimes.erase(frameTimes.begin(), frameTimes.begin() + 1);

#ifndef IMGUI
  float total = 0;
  for (auto var : frameTimes) {
    total += var;
  }
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Fps: {}", frameTimes.size() / total);
#endif
#ifdef IMGUI
  // Calc frame times

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;
  //           ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
  //           ImGuiDockNodeFlags_PassthruCentralNode);
  // DEMO:
  // ImGui::ShowDemoWindow();
  // ImPlot::ShowDemoWindow();
  RenderSceneView(scene);
  RenderPerformanceGraph();
  RenderObjectInspector();

#endif
  /*
  material->SetupMaterial();
  material->program->SetUniform("color", 1, 0, 0, 1);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  CHECK_GL_ERROR();
  */
  for (auto &[key, val] : materials) {
    val->SetupMaterial();
    val->RenderObjects();
  }
#ifdef IMGUI
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
  glfwSwapBuffers(window);
  glfwPollEvents();

  return 0;
} // namespace Graphics

void Main::Terminate() {
  materials.clear();
  vertexShaders.clear();
  fragmentShaders.clear();
#ifdef IMGUI
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
#endif
  glfwDestroyWindow(window);
  glfwTerminate();
}
void Main::keyCallback(int key, int scancode, int action, int mods) {
  if (keyMap.contains(scancode)) {
    for (auto var : keyMap[scancode]) {
      var(action, mods);
    }
  }
}
void Main::SetKeyCallback(const int key, void (*action)(int, int)) {
  const int scancode = glfwGetKeyScancode(key);
  if (!keyMap.contains(scancode)) {
    keyMap[scancode] = {};
  }
  keyMap[scancode].insert(keyMap[scancode].end(), action);
}
} // namespace Graphics
} // namespace Engine
