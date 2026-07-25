#pragma once

#include "Program.hpp"
#include <memory>
namespace Engine {
namespace Graphics {
        class ComponentRenderable;
class Material {
public:
  std::shared_ptr<Program> program;
  unsigned int vao = 0;
  std::string path = "";
  std::string name = "";
  bool usable = false;
  bool uses_camera;
  void SetupMaterial();
  void RenderObjects();
  std::vector<std::shared_ptr<ComponentRenderable>> renderableObjects;
  static std::shared_ptr<Material> Create(std::string path);
  Material(std::string path);
  Material();
  ~Material();

private:
  static bool ran_from_create;
};
} // namespace Graphics
} // namespace Engine
