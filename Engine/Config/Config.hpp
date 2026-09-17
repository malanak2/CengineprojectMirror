//
// Created by malan on 21.04.2026.
//
#pragma once
#include <memory>
#include <string>
//!
//! @brief Here lives the configuraion of the engine
//!
class Config {
public:
  class Graphics {
  public:
    //!
    //! @enableAntiAliasing Whether anti-aliasing is enabled
    //!
    bool enableAntiAliasing;
    //!
    //! @enableVsync Whether vsync is enabled
    //!
    bool enableVsync;
    //!
    //! @materialPath Root path of materials
    //!
    std::string materialPath;
    //!
    //! @texturePath Root path of textures
    //!
    std::string texturePath;
    //!
    //! @scenePath Root path of scenes
    //!
    std::string scenePath;
  };
  class Defaults {
  public:
    std::string StartupScenePath;
  };
  class Window {
  public:
    std::string title;
  };
  std::unique_ptr<Graphics> graphics;
  std::unique_ptr<Window> window;
  std::unique_ptr<Defaults> defaults;

  Config(std::string file_path);
  static std::shared_ptr<Config> inst;
};
