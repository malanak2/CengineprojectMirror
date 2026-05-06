//
// Created by malan on 20.04.2026.
//

#include "Engine.hpp"
#include "Engine/Graphics/Graphics.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Object.hpp"
#include "Scene.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <chrono>
#include <memory>

namespace Engine {
int Main::width = 0;
int Main::height = 0;
std::shared_ptr<Engine::Main> Engine::Main::Create() {
  std::shared_ptr<Main> e = std::make_shared<Main>();
  e->setupLogger();
  auto logger = spdlog::get("console");
  SPDLOG_LOGGER_INFO(logger, "Loading config...");
  e->config = new Config("Engine.ini");
  SPDLOG_LOGGER_INFO(logger, "Loading graphics...");
  e->graphics = new Graphics::Main();

  if (e->graphics->Init(e->config) != 0) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to initialize graphics!");
    throw std::logic_error("Failed to initialize graphics!");
  }
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Loading scene");
  try {
    e->current_scene = Scene::Load(e->config->defaults->StartupScenePath);
  } catch (const std::exception &ex) {
    e->current_scene = std::make_shared<Scene>();
    auto object_default = std::make_shared<Object>(e->current_scene);
    JsonFileBase jsbase = {};
    jsbase.object_type = ObjectType::Component;
    Graphics::RenderableDataJson rdj;
    rdj.indices = {0, 1, 2};
    rdj.vertices = {
        -0.5f, -0.5f, 0.0f, // left
        0.5f,  -0.5f, 0.0f, // right
        0.0f,  0.5f,  0.0f  // top
    };
    rdj.material_path = "materials/basic.json";
    rdj.uniforms = {{"color", {1, 0, 0, 1}}};
    jsbase.data = rdj;
    json jsbase_js = jsbase;
    auto com_render =
        Graphics::ComponentRenderable::Create(jsbase_js, object_default);
    object_default->fromParams("Test object", {com_render});
    e->current_scene->Instantiate(object_default);
    std::string scene_json = e->current_scene->ToJson().dump();
    FileUtil::SaveFile("scenes/default.json", &scene_json);
  }
  CHECK_GL_ERROR();
  return e;
}

void Engine::Main::Run() {
  auto logger = spdlog::get("console");
  SPDLOG_LOGGER_INFO(logger, "Running...");
  // Handle
  std::chrono::time_point last_tick_begin = std::chrono::steady_clock::now();
  current_scene->Setup();
  while (true) {
    auto current_time = std::chrono::steady_clock::now();
    auto duration = current_time - last_tick_begin;
    last_tick_begin = current_time;
    current_scene->Update();
    if (graphics->Tick(
#ifdef IMGUI
            this->current_scene,
#endif
            std::chrono::duration_cast<std::chrono::duration<double>>(
                duration)) != 0) {
      Terminate();
      break;
    }
    CHECK_GL_ERROR();
  }
  SPDLOG_LOGGER_INFO(logger, "Main stopping.");
}

void Engine::Main::setupLogger() {
  auto console = spdlog::stdout_color_mt("console");
  console->set_pattern("[%H:%M:%S %z] [%n] [%^%l%$] [%@] %v");
  spdlog::set_level(spdlog::level::debug);
  spdlog::flush_on(spdlog::level::debug);
  SPDLOG_LOGGER_INFO(console, "Set up logger!");
}

void Engine::Main::Terminate() { graphics->Terminate(); }
} // namespace Engine
