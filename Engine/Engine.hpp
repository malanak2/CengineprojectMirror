//
// Created by malan on 20.04.2026.
//
#pragma once
#include "Scene.hpp"
#include "Config/Config.hpp"
#include "Graphics/Graphics.hpp"
namespace Engine {
class Engine {
public:
  static std::shared_ptr<Engine> Create();
  void Run();

private:
  void setupLogger();
  void Terminate();

  Graphics::Main *graphics;
  Config *config;
  std::unique_ptr<Scene> scene;
};
}