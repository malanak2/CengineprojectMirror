//
// Created by malan on 21.04.2026.
//
#pragma once
#include <memory>
#include <string>
#include <vector>
class Config {
public:
  class Graphics {
  public:
    // TODO: Move to level
    std::vector<float> CameraPos;
    std::vector<float> CameraRot;
  };
  class Window {
  public:
    std::string title;
  };
  std::unique_ptr<Graphics> graphics;
  std::unique_ptr<Window> window;

  Config(std::string file_path);
};
