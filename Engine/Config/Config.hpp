//
// Created by malan on 21.04.2026.
//
#pragma once
#include <memory>
#include <string>
class Config {
public:
  class Graphics {
  public:
    bool enableAntiAliasing;
    bool enableVsync;
    std::string materialPath;
    std::string texturePath;
    std::string scenesPath;
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
};
