#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IJson.hpp"
#include "Interfaces/IObject.hpp"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include <cpptrace/basic.hpp>
#include <nlohmann/detail/macro_scope.hpp>

using json = nlohmann::json;

struct CameraObjectJson {
public:
  std::string name;
  std::vector<float> position;
  std::vector<float> rotation;
  std::vector<ComponentJson> components;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraObjectJson, name, position, rotation,
                                   components);

class CameraObject : public IObject, public IJson {
public:
  CameraObject(std::string name,
               std::vector<std::shared_ptr<IComponent>> comps = {},
               std::vector<float> position = {0, 0, 0},
               std::vector<float> rotation = {0, 0, 0, 0});
  CameraObject(std::string path);
  ~CameraObject();
  std::string _name;

  // IObject
  void Setup() override;
  void Update() override;
  void FixedUpdate() override;
  void Save() override;
  void Load() override;

  // IJson
  json ToJson() override;

private:
  void fromParams(std::string name,
                  std::vector<std::shared_ptr<IComponent>> comps = {},
                  std::vector<float> position = {0, 0, 0},
                  std::vector<float> rotation = {0, 0, 0, 0});
};
