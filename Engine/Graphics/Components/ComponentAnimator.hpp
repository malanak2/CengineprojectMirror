#pragma once

#include "Graphics/Model.hpp"
#include "Interfaces/IComponent.hpp"
#include <string>
namespace Engine {
namespace Graphics {
class ComponentAnimator : public IComponent {
  REGISTER_CLASS(ComponentAnimator);

public:
  json ToJson() override;

  void FromJson(json &js) override;
  void Update() override;
  void Setup() override;
  void LoadAnimationsFromModel(const std::string &modelPath,
                               std::shared_ptr<Model> model);

  void SetAnimation(std::string name);
  void SetIsPlaying(bool state);
  bool GetIsPlaying();
  std::shared_ptr<Animator> GetAnimator();
  std::string GetCurrentAnimation();

  std::map<std::string, std::shared_ptr<Animation>> animations;
  std::map<std::string, std::shared_ptr<Animator>> animators;

private:
  bool isPlaying = true;
  std::string current_animation = "";
  std::shared_ptr<Animator> animation = nullptr;
};
} // namespace Graphics
} // namespace Engine
