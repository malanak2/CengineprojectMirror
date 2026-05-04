//
// Created by malan on 20.04.2026.
//
#pragma once
#include "Config/Config.hpp"
#include "Graphics/Graphics.hpp"
#include "Scene.hpp"
namespace Engine {
class Main {
public:
  static std::shared_ptr<Main> Create();
  void Run();

private:
  void setupLogger();
  void Terminate();

  Graphics::Main *graphics;
  Config *config;
  std::unique_ptr<Scene> scene;
};
} // namespace Engine
