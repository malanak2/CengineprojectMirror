#pragma once

#include "Graphics/Interfaces/IUniform.hpp"
#include "implot.h" // IWYU pragma: keep
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <typeindex>
#define IMGUI_SCALE 1
namespace Editor {
namespace ImGuiRenderer {
#define IMGUI_REGISTER_UNIFORM(p1, p2)                                         \
  {                                                                            \
    instance->Register(std::type_index(typeid(p1)), [                          \
    ](std::shared_ptr<Engine::Graphics::IUniform> uniform) p2);                \
  }
class ImGuiUniformRenderer {
public:
  static void Init();
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
