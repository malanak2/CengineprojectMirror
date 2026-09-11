//
// Created by malan on 20.04.2026.
//
#pragma once
#include "Graphics/Graphics.hpp"
#include "Scene.hpp"
#include <memory>
namespace Engine {
class Engine {
public:
  static void Init();
  /// Loads the default scene
  static void LoadScene();
  /// Loads the scene at the specified path
  static void LoadScene(std::string path);
  static std::shared_ptr<Engine> instance;
  void Run();
  static int width, height;

private:
  void setupLogger();
  void Terminate();
  std::unique_ptr<Graphics::Main> graphics;
  std::shared_ptr<Scene> current_scene;
};
} // namespace Engine
