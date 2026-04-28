//
// Created by malan on 20.04.2026.
//

#include "Engine.hpp"
#include "Engine/Graphics/Graphics.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

int Engine::Init() {
  setupLogger();
  auto logger = spdlog::get("console");
  logger->info("Loading config...");
  config = new Config("Engine.ini");
  logger->info("Loading graphics...");
  graphics = new Graphics::Main();
  if (graphics->Init(config) != 0) {
    logger->error("Failed to initialize graphics!");
    return -1;
  }
  CHECK_GL_ERROR();
  return 0;
}

void Engine::Run() {
  auto logger = spdlog::get("console");
  logger->info("Running...");
  // Handle
  while (true) {
    if (graphics->Tick() != 0) {
      Terminate();
      break;
    }
    CHECK_GL_ERROR();
  }
  logger->info("Engine stopping.");
}

void Engine::setupLogger() {
  auto console = spdlog::stdout_color_mt("console");
  spdlog::get("console")->set_pattern("[%H:%M:%S %z] [%n] [%^%l%$] [%@] %v");
  spdlog::get("console")->info("Set up logger!");
  SPDLOG_LOGGER_INFO(spdlog::get("console"), "Set up logger!");
}

void Engine::Terminate() { graphics->Terminate(); }
