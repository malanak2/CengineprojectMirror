#pragma once

#include "Program.hpp"
#include <memory>
namespace Graphics {
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
  Material(std::string path);
  static std::shared_ptr<Material> Create(std::string path);
  Material();
  ~Material();
};
} // namespace Graphics
