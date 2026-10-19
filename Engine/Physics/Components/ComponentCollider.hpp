#pragma once

#include "Graphics/Material.hpp"
#include "Graphics/Program.hpp"
#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
#include "Object.hpp" // IWYU pragma: keep
#include "Physics/Components/ComponentPhysics.hpp"
#include <functional>
#include <memory>
namespace Engine {
namespace Physics {
class ComponentCollider
    : public IComponent,
      public std::enable_shared_from_this<ComponentCollider> {
  REGISTER_CLASS(ComponentCollider);

public:
  json ToJson() override;

  void FromJson(json &js) override;
  void OnCollisionEnter(std::shared_ptr<ComponentCollider> other);
  void Setup() override;
  void Update() override;
  void End() override;
  //!
  //! @brief Add a function to call on collision enter
  //!
  //! @param[in] f The function to call, getting a ComponentCollider ref
  //!
  void SetOnCollisionEnter(
      std::function<void(std::shared_ptr<ComponentCollider>)> f) {
    func = f;
  }
  std::function<void(std::shared_ptr<ComponentCollider>)> func = nullptr;
  bool isTrigger = true;
  std::vector<glm::vec3> Corners = {{-1, -1, -1}, {1, -1, -1}, {-1, -1, 1},
                                    {1, -1, 1},   {-1, 1, -1}, {1, 1, -1},
                                    {-1, 1, 1},   {1, 1, 1}};
  std::vector<glm::vec3> GetCachedCorners();
  bool DoesCollide(std::shared_ptr<ComponentCollider> other);

  static void RenderDebugBounds();
  static bool debugDrawEnabled;

private:
  std::vector<glm::vec3> cachedCorners = {};
};
} // namespace Physics
} // namespace Engine
