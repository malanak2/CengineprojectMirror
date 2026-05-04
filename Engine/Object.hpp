#pragma once

#include "Graphics/Components/ComponentRenderable.hpp"
#include "Interfaces/IComponent.hpp"
#include <memory>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
using json = nlohmann::json;

namespace Engine {
struct ObjectJson {
  std::vector<float> position;
  std::vector<float> rotation;
  std::vector<ComponentJson> components;
  std::string name;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectJson, position, rotation, components)
class Object : public IJson {
public:
  std::map<ENGINE_COMPONENT_TYPE, std::shared_ptr<IComponent>> _components = {};
  std::vector<float> _position;
  std::vector<float> _rotation;
  Object(std::string path);
  Object();
  std::string _name;
  void Setup();
  void Update();
  void FixedUpdate();
  void Save();
  void Load();
  json ToJson() override;

  void FromJson(json &js) override;
  void FromJson(json &js, std::string path);

  void fromParams(std::string name,
                  std::vector<std::shared_ptr<IComponent>> comps = {},
                  std::vector<float> position = {0, 0, 0},
                  std::vector<float> rotation = {0, 0, 0, 0});
};
} // namespace Engine
