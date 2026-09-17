//
// Created by malan on 20.04.2026.
//
#pragma once
#include "Graphics/Graphics.hpp"
#include "Scene.hpp"
#include <memory>
namespace Engine {
//!
//! @brief The main engine class.
//!
class Engine {
public:
  //!
  //! @brief Initializes the engine. Any setup / "Inject" calls should be before
  //! this call
  //!
  //!
  static void Init();
  //!
  //! @brief Loads the default scene.
  //!
  //!
  static void LoadScene();
  //!
  //! @brief Loads a scene
  //!
  //! @param[in] path Path to the scene to load
  //!
  static void LoadScene(std::string path);
  static std::shared_ptr<Engine> instance;
  //!
  //! @brief Pass control to the engine.
  //!
  //!
  void Run();
  static int width, height;

  std::shared_ptr<Scene> current_scene;

private:
  void setupLogger();
  void Terminate();
};
} // namespace Engine
