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
  static int width, height;

private:
  void setupLogger();
  void Terminate();
  Graphics::Main *graphics;
  Config *config;
  std::shared_ptr<Scene> current_scene;
};
} // namespace Engine
