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
  const std::vector<glm::mat4> &GetFinalBoneMatrices() const;

  void SetSockets(const std::vector<SocketAttachment> &newSockets);
  const std::vector<SocketAttachment> &GetSockets() const { return sockets; }
  void AddSocket(const SocketAttachment &socket);
  void RemoveSocket(size_t index);

  glm::mat4 GetBoneGlobalTransform(const std::string &boneName) const;
  glm::mat4 GetBoneWorldTransform(const std::string &boneName) const;
  std::vector<std::string> GetNodeNames() const;

  std::vector<SocketAttachment> sockets;
  std::map<std::string, std::shared_ptr<Animation>> animations;
  std::map<std::string, std::shared_ptr<Animator>> animators;

private:
  bool isPlaying = true;
  std::string current_animation = "";
  std::shared_ptr<Animator> animation = nullptr;
  std::vector<glm::mat4> m_DefaultBoneMatrices;
};
} // namespace Graphics
} // namespace Engine
