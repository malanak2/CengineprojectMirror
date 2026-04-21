//
// Created by malan on 20.04.2026.
//

#ifndef CENGINEPROJECT_ENGINE_HPP
#define CENGINEPROJECT_ENGINE_HPP
#include "Config/Config.hpp"
#include "Graphics/Graphics.hpp"

class Engine {
public:
  void Init();
  void Run();

private:
  void setupLogger();
  void Terminate();

  Graphics *graphics;
  Config *config;
};

#endif // CENGINEPROJECT_ENGINE_HPP
