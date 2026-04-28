#pragma once

#include <string>
namespace Graphics {
class IComponent {
public:
  virtual ~IComponent() = default;
  virtual void Setup() = 0;
  virtual void Update() = 0;
  virtual void FixedUpdate() = 0;
  virtual void Save(std::string path) = 0;
  virtual void Load(std::string path) = 0;
};
} // namespace Graphics
