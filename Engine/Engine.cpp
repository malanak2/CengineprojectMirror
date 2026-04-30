//
// Created by malan on 20.04.2026.
//

#include "Engine.hpp"
#include "Engine/Graphics/Graphics.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
std::shared_ptr<Engine::Engine> Engine::Engine::Create() {
  std::shared_ptr<Engine> e = std::make_shared<Engine>();
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
  CHECK_GL_ERROR();
  return e;
}

void Engine::Engine::Run() {
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
  SPDLOG_LOGGER_INFO(logger, "Engine stopping.");
}

void Engine::Engine::setupLogger() {
  auto console = spdlog::stdout_color_mt("console");
  spdlog::get("console")->set_pattern("[%H:%M:%S %z] [%n] [%^%l%$] [%@] %v");
  SPDLOG_LOGGER_INFO(spdlog::get("console"), "Set up logger!");
}

void Engine::Engine::Terminate() { graphics->Terminate(); }
