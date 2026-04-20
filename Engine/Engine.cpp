//
// Created by malan on 20.04.2026.
//

#include "Engine.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"



void Engine::Init() {
    setupLogger();
    graphics = new Graphics();
    graphics->Init();
}

void Engine::Run() {
    // Handle
    while (true) {
        if (graphics->Tick() != 0) {
            Terminate();
            break;
        }
    }
    spdlog::get("console")->info("Engine stopping.");
}

void Engine::setupLogger() {
    auto console = spdlog::stdout_color_mt("console");
    spdlog::get("console")->info("Set up logger!");
}

void Engine::Terminate() {
    graphics->Terminate();
}
