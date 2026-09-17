#pragma once

#include "Graphics/Interfaces/IUniform.hpp"
#include "Scene.hpp"
#include "implot.h" // IWYU pragma: keep
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <typeindex>
#define IMGUI_SCALE 1
namespace Editor {
namespace ImGuiRenderers {
#define IMGUI_REGISTER_UNIFORM(p1, p2)                                         \
  {                                                                            \
    instance->Register(std::type_index(typeid(p1)), [                          \
    ](std::shared_ptr<Engine::Graphics::IUniform> uniform) p2);                \
  }
//!
//! @brief Class for registering imgui renderers for Uniform types
//!
class ImGuiUniformRenderer {
public:
  //!
  //! @brief Initializes the ImGuiUniformRenderer
  //!
  //!
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
//!
//! @brief Class for containing genreal imgui renderers
//!
class ImGuiRenderer {
public:
  static std::shared_ptr<Engine::SceneObject> sceneObject;
  static void ShowSceneObjectMenu(
      std::vector<std::shared_ptr<Engine::SceneObject>> *sceneObjects);
  //!
  //! @brief Renderes the scene view using ImGui
  //!
  //! @param[in] scene scene ptr to render the view
  //!
  static void RenderSceneView(std::shared_ptr<Engine::Scene> scene);

  //!
  //! @brief Renders the object inspector using ImGui
  //!
  //!
  static void RenderObjectInspector();

  //!
  //! @newObjectParent Variable for changing hierarchy
  //!
  static std::shared_ptr<Engine::SceneObject> newObjectParent;

  static bool wasSavePressedThisFrame;
  static char namebuf[64];
  static char matbuf[64];
  static float coords[3];
  static float rotation[4];
};
} // namespace ImGuiRenderers
} // namespace Editor
