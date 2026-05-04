//
// Created by malan on 20.04.2026.
//

#include "Engine.hpp"
#include "Engine/Graphics/Graphics.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
namespace Engine {
std::shared_ptr<Engine::Main> Engine::Main::Create() {
  std::shared_ptr<Main> e = std::make_shared<Main>();
  e->setupLogger();
  auto logger = spdlog::get("console");
  SPDLOG_LOGGER_INFO(logger, "Loading config...");
  e->config = new Config("Main.ini");
  SPDLOG_LOGGER_INFO(logger, "Loading graphics...");
  e->graphics = new Graphics::Main();

  if (e->graphics->Init(e->config) != 0) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to initialize graphics!");
    throw std::logic_error("Failed to initialize graphics!");
  }
  CHECK_GL_ERROR();
  return e;
}

void Engine::Main::Run() {
  auto logger = spdlog::get("console");
  SPDLOG_LOGGER_INFO(logger, "Running...");
  // Handle
  while (true) {
    if (graphics->Tick() != 0) {
      Terminate();
      break;
    }
    CHECK_GL_ERROR();
  }
  SPDLOG_LOGGER_INFO(logger, "Main stopping.");
}

void Engine::Main::setupLogger() {
  auto console = spdlog::stdout_color_mt("console");
  spdlog::get("console")->set_pattern("[%H:%M:%S %z] [%n] [%^%l%$] [%@] %v");
  SPDLOG_LOGGER_INFO(spdlog::get("console"), "Set up logger!");
}

void Engine::Main::Terminate() { graphics->Terminate(); }
} // namespace Engine
