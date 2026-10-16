#pragma once

#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include <functional>
namespace Engine {
namespace Physics {
class ComponentCollider : public IComponent {
  REGISTER_CLASS(ComponentCollider);

public:
  json ToJson() override;

  void FromJson(json &js) override;
  void OnCollisionEnter(ComponentCollider &other) {
    if (func != nullptr)
      func(other);
    // Do not process collision if is trigger
    if (isTrigger)
      return;
    // TODO: Process collision on my side
  }
  //!
  //! @brief Add a function to call on collision enter
  //!
  //! @param[[TODO:direction]] [TODO:name] [TODO:description]
  //!
  void SetOnCollisionEnter(std::function<void(ComponentCollider &)>) {}
  std::function<void(ComponentCollider &)> func = nullptr;
  bool isTrigger = true;
};
} // namespace Physics
} // namespace Engine
