#pragma once

namespace Graphics {
class IComponent {
public:
  virtual ~IComponent() = default;
  virtual void Setup() = 0;
  virtual void Update() = 0;
  virtual void FixedUpdate() = 0;
  virtual void Save() = 0;
  virtual void Load() = 0;
};
} // namespace Graphics
