#pragma once

#include "Engine/IComponent.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
using json = nlohmann::json;
namespace Graphics {
struct ObjectJson {
public:
  std::string object_type;
  std::vector<float> position;
  std::vector<json> components;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectJson, components);

class Object {
public:
  std::vector<std::shared_ptr<IComponent>> _components;
  std::vector<float> _position;
  Object(std::string name, std::vector<std::shared_ptr<IComponent>> comps = {},
         std::vector<float> position = {0, 0, 0});
  Object(std::string paht);
  ~Object();
  std::string _name;
  void Setup();
  void Update();
  void FixedUpdate();
  void Save();
  void Load();

private:
};
} // namespace Graphics
