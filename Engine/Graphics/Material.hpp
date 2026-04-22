#pragma once

#include "IRenderable.hpp"
#include "Program.hpp"
#include <memory>
#include <vector>
namespace Graphics {
class Material {
public:
  std::shared_ptr<Program> program;
  unsigned int vao = 0;
  std::string path = "";
  bool usable = false;
  std::vector<std::shared_ptr<IRenderable>> objects;
  void SetupMaterial();
  void RenderObjects();
  Material(std::string path);
  Material();
};
} // namespace Graphics
