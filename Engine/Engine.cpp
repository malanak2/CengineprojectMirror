//
// Created by malan on 20.04.2026.
//

#include "Engine.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"



void Engine::Init() {
    setupLogger();
    spdlog::get("console")->info("setting up rest...");
}

void Engine::Run() {
}

void Engine::setupLogger() {
    auto console = spdlog::stdout_color_mt("console");
    spdlog::get("console")->info("Set up logger!");
}
