#pragma once

#include "Graphics/Interfaces/IUniform.hpp"
#include "implot.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <typeindex>
#define IMGUI_SCALE 1
namespace Editor {
namespace ImGuiRenderer {
class ImGuiUniformRenderer {
public:
  void Register(std::type_index type,
                void (*func)(std::shared_ptr<Engine::Graphics::IUniform>));
  void Render(std::shared_ptr<Engine::Graphics::IUniform> uni);

private:
  static std::shared_ptr<ImGuiUniformRenderer> instance;
  std::map<std::type_index,
           void (*)(std::shared_ptr<Engine::Graphics::IUniform>)>
      funcs = std::map<std::type_index,
                       void (*)(std::shared_ptr<Engine::Graphics::IUniform>)>();
};
} // namespace ImGuiRenderer
} // namespace Editor
