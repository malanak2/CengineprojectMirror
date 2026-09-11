//
// Created by malan on 20.04.2026.
//
#include "Engine.hpp"
#include "Engine/Graphics/Graphics.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Graphics/Uniforms/UniformFloatVector.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Object.hpp"
#include "Scene.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include "spdlog/spdlog.h"
#include <chrono>
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdexcept>

namespace Engine {

std::shared_ptr<Engine> Engine::instance = nullptr;
int Engine::width = 0;
int Engine::height = 0;
void Engine::Engine::Init() {
  std::shared_ptr<Engine> e = std::make_shared<Engine>();
  e->setupLogger();
  auto logger = spdlog::get("console");

  if (!Config::inst) {
    SPDLOG_LOGGER_INFO(logger, "Loading config...");
    Config::inst = std::make_shared<Config>("Engine.ini");
    SPDLOG_LOGGER_INFO(logger, "Loading graphics...");
  }
  auto config = Config::inst;
  e->graphics = std::make_unique<Graphics::Main>();

  if (e->graphics->Init(config) != 0) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to initialize graphics!");
    throw std::logic_error("Failed to initialize graphics!");
  }
  instance = e;
}
void Engine::LoadScene() {
  LoadScene(Config::inst->defaults->StartupScenePath);
}
void Engine::LoadScene(std::string path) {
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Loading scene");
  auto e = Engine::instance;
  auto config = Config::inst;
  if (e == nullptr || config == nullptr) {
    throw new std::logic_error("Engine or config is not initialized!");
  }
  try {
    e->current_scene = Scene::Load(path);
  } catch (const std::exception &ex) {
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Failed to load scene at {} : {}",
                       path, ex.what());
    /*
    e->current_scene = std::make_shared<Scene>();
    auto camera_obj = std::make_shared<Object>(e->current_scene);
    auto camera_comp = std::make_shared<Graphics::CameraComponent>(camera_obj);
    camera_obj->fromParams("Main Camera", {camera_comp}, {0.0f, 0.0f, 5.0f},
                           {0.0f, -90.0f, 0.0f});
    e->current_scene->Instantiate(camera_obj);
    e->current_scene->camera = camera_comp;

    auto object_default = std::make_shared<Object>(e->current_scene);
    JsonFileBase jsbase = {};
    jsbase.object_type = ObjectType::Component;
    Graphics::RenderableDataJson rdj;
    rdj.indices = {
        0, 1, 2, // Bottom 1
        0, 2, 3, // Bottom 2
        0, 4, 1, // Front
        1, 4, 2, // Right
        2, 4, 3, // Back
        3, 4, 0  // Left
    };
    rdj.vertices = {
        -1.0, -1.0, -1.0, // Front left
        1.0,  -1.0, -1.0, // Front right
        1.0,  -1.0, 1.0,  // Back right
        -1.0, -1.0, 1.0,  // Back left
        0.0,  1.0,  0.0   // Top
    };
    rdj.material_path = "materials/basic.json";
    std::vector<float> unis = {0, 1, 0, 1};
    rdj.uniforms = {
        {"color", std::make_shared<Graphics::UniformFloatVector>(
                      Graphics::Vector, 0, 0,
                      std::make_shared<std::string>("color"), unis)}};
    jsbase.data = rdj;
    json jsbase_js = jsbase;
    auto com_render =
        Graphics::ComponentRenderable::Create(jsbase_js, object_default);
    object_default->fromParams("Test object", {com_render});
    e->current_scene->Instantiate(object_default);
    std::string scene_json = e->current_scene->ToJson().dump();
    FileUtil::SaveFile(path, &scene_json);
    */
  }
  CHECK_GL_ERROR();
}

void Engine::Engine::Run() {
  auto logger = spdlog::get("console");
  SPDLOG_LOGGER_INFO(logger, "Running...");
  // Handle
  std::chrono::time_point last_tick_begin = std::chrono::steady_clock::now();
  current_scene->Setup();
  while (true) {
    // break;
    auto current_time = std::chrono::steady_clock::now();
    auto dur_graphics = current_time - last_tick_begin;
    current_scene->Update();
    auto cur = std::chrono::steady_clock::now();
    auto dur_other = cur - current_time;
    last_tick_begin = cur;
    if (graphics->Tick(
#ifdef IMGUI
            this->current_scene,
#endif
            std::chrono::duration_cast<std::chrono::duration<double>>(
                dur_other),
            std::chrono::duration_cast<std::chrono::duration<double>>(
                dur_graphics)) != 0) {
      Terminate();
      break;
    }
    CHECK_GL_ERROR();
  }
  SPDLOG_LOGGER_INFO(logger, "Main stopping.");
}

void Engine::Engine::setupLogger() {
  auto console = spdlog::stdout_color_mt("console");
  console->set_pattern("[%H:%M:%S %z] [%n] [%^%l%$] [%@] %v");
  spdlog::set_level(spdlog::level::debug);
  spdlog::flush_on(spdlog::level::debug);
  SPDLOG_LOGGER_INFO(console, "Set up logger!");
}

void Engine::Engine::Terminate() { graphics->Terminate(); }
} // namespace Engine
