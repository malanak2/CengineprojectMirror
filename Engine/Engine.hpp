//
// Created by malan on 20.04.2026.
//
#pragma once
#include "Config/Config.hpp"
#include "Graphics/Graphics.hpp"
#include "Scene.hpp"
#include <memory>
namespace Engine {
class Main {
public:
  static std::shared_ptr<Main> Create();
  void Run();
  static int width, height;

private:
  void setupLogger();
  void Terminate();
  std::unique_ptr<Graphics::Main> graphics;
  std::shared_ptr<Config> config;
  std::shared_ptr<Scene> current_scene;
};
} // namespace Engine
