//
// Created by malan on 20.04.2026.
//
#include "Engine.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Graphics/Graphics.hpp"
#include "Graphics/Uniforms/UniformFloatVector.hpp"
#include "InputSystem.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Object.hpp"
#include "Physics/PhysicsSystem.hpp"
#include "Scene.hpp"
#include "Util/LoggerUtil.hpp"
#include "spdlog/spdlog.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <exception>
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdexcept>
#include <tracy/Tracy.hpp>
namespace Engine {
bool Engine::isSimulationEnabled = true;
std::shared_ptr<Engine> Engine::instance = nullptr;
int Engine::width = 0;
int Engine::height = 0;
void Engine::Engine::Init() {
  ZoneScoped;
  std::shared_ptr<Engine> e = std::make_shared<Engine>();
  e->setupLogger();
  auto logger = spdlog::get("console");

  if (!Config::inst) {
    SPDLOG_LOGGER_INFO(logger, "Loading config...");
    Config::inst = std::make_shared<Config>("Engine.ini");
    SPDLOG_LOGGER_INFO(logger, "Loading Graphics::Main::instance...");
  }
  auto config = Config::inst;
  //  Graphics::Main::instance = std::make_unique<Graphics::Main>();

  if (Graphics::Main::Init(config) != 0) {
    SPDLOG_LOGGER_ERROR(logger,
                        "Failed to initialize Graphics::Main::instance!");
    throw std::logic_error("Failed to initialize Graphics::Main::instance!");
  }
  InputSystem::Init();
  instance = e;
}
void Engine::LoadScene() {
  LoadScene(Config::inst->defaults->StartupScenePath);
}
void Engine::LoadScene(std::string path) {
  ZoneScoped;
  SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Loading scene");
  auto e = Engine::instance;
  auto config = Config::inst;
  if (e == nullptr || config == nullptr) {
    throw std::logic_error("Engine or config is not initialized!");
  }
  try {
    e->current_scene = Scene::Load(path);
  } catch (const std::exception &ex) {
    
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                           "Failed to load scene at {} : {}. Using an empty scene.",
                           path, ex.what());
    auto fallback = Scene::Create();
    fallback->path = path;
    fallback->FinalizeCameras({});
    e->current_scene = fallback;
  }
  CHECK_GL_ERROR();
}

void Engine::Engine::Run() {
  ZoneScoped;
  auto logger = spdlog::get("console");

  // Handle
  std::chrono::time_point last_tick_begin = std::chrono::steady_clock::now();
  current_scene->EarlySetup();
  current_scene->Setup();
  current_scene->OnEnable();

  SPDLOG_LOGGER_INFO(logger, "Running...");
  while (true) {
    ZoneScopedN("Frame");
    float currTime = glfwGetTime();
    if (lastTime == 0) {
      lastTime = currTime;
    }
    deltaTime = currTime - lastTime;
    lastTime = currTime;
    // break;
    auto current_time = std::chrono::steady_clock::now();
    auto dur_graphics = current_time - last_tick_begin;
    {
      ZoneScopedN("Inputs");
      InputSystem::ProcessEvents();
    }

    {
      ZoneScopedN("Update");
      current_scene->Update();
    }

    if (isSimulationEnabled) {
      Physics::PhysicsSystem::ProcessPhysics();
    }
    auto cur = std::chrono::steady_clock::now();
    auto dur_other = cur - current_time;
    last_tick_begin = cur;
    {
      ZoneScopedN("Graphics");
      if (Graphics::Main::Tick(
              std::chrono::duration_cast<std::chrono::duration<double>>(
                  dur_other),
              std::chrono::duration_cast<std::chrono::duration<double>>(
                  dur_graphics)) != 0) {
        Terminate();
        break;
      }
    }
    CHECK_GL_ERROR();
  }
  SPDLOG_LOGGER_INFO(logger, "Main stopping.");
}
float Engine::GetDeltaTime() { return instance->deltaTime; }

void Engine::Engine::setupLogger() {
  auto console = spdlog::stdout_color_mt("console");
  console->set_pattern("[%H:%M:%S %z] [%n] [%^%l%$] [%@] %v");
  console->set_level(spdlog::level::debug);
  console->flush_on(spdlog::level::debug);
  spdlog::set_level(spdlog::level::debug);
  spdlog::flush_on(spdlog::level::debug);
  SPDLOG_LOGGER_INFO(console, "Set up logger!");
}

void Engine::Engine::Terminate() { Graphics::Main::instance->Terminate(); }
} // namespace Engine
