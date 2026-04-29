#pragma once

#include "Interfaces/IComponent.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
using json = nlohmann::json;

class IObject {
public:
  std::vector<std::shared_ptr<IComponent>> _components;
  std::vector<float> _position;
  std::vector<float> _rotation;
  std::string _name;
  virtual void Setup() = 0;
  virtual void Update() = 0;
  virtual void FixedUpdate() = 0;
  virtual void Save() = 0;
  virtual void Load() = 0;

private:
};
