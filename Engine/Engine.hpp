//
// Created by malan on 20.04.2026.
//
#pragma once
#include "Config/Config.hpp"
#include "Graphics/Graphics.hpp"

class Engine {
public:
  int Init();
  void Run();

private:
  void setupLogger();
  void Terminate();

  Graphics::Main *graphics;
  Config *config;
};
