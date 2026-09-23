#include "ComponentAnimator.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Object.hpp"
#include "Util/LoggerUtil.hpp"
#include <memory>
#include <tracy/Tracy.hpp>
json Engine::Graphics::ComponentAnimator::ToJson() {
  json js;
  json socketsJson = json::array();
  for (const auto &s : sockets) {
    json item;
    to_json(item, s);
    socketsJson.push_back(item);
  }
  js["sockets"] = socketsJson;
  js["current_animation"] = current_animation;
  js["is_playing"] = isPlaying;
  return js;
}

void Engine::Graphics::ComponentAnimator::FromJson(json &js) {
  if (js.is_null())
    return;
  if (js.contains("sockets") && js["sockets"].is_array()) {
    sockets.clear();
    for (const auto &item : js["sockets"]) {
      SocketAttachment sa;
      from_json(item, sa);
      sockets.push_back(sa);
    }
    SetSockets(sockets);
  }
  if (js.contains("current_animation") && js["current_animation"].is_string()) {
    current_animation = js["current_animation"];
    if (animators.contains(current_animation) || current_animation.empty()) {
      SetAnimation(current_animation);
    }
  }
  if (js.contains("is_playing") && js["is_playing"].is_boolean()) {
    SetIsPlaying(js["is_playing"]);
  }
}
void Engine::Graphics::ComponentAnimator::Update() {
  ZoneScoped;
  if (isPlaying && animation) {
    animation->UpdateAnimation(GetDeltaTime());
  }
}
void Engine::Graphics::ComponentAnimator::Setup() {
  auto obj = object.lock();
  auto renderable = obj->GetComponent<Engine::Graphics::ComponentRenderable>();
  if (renderable && renderable->model) {
    LoadAnimationsFromModel(renderable->_model_path, renderable->model);
  }
}

void Engine::Graphics::ComponentAnimator::LoadAnimationsFromModel(
    const std::string &modelPath, std::shared_ptr<Model> model) {
  std::string sanitized_path = (modelPath.rfind("resources/", 0) == 0)
                                   ? modelPath
                                   : ("resources/" + modelPath);

  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(sanitized_path, aiProcess_Triangulate);
  if (!scene || scene->mNumAnimations == 0) {
    SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                       "No animations found in: {} (Error: {})", sanitized_path,
                       importer.GetErrorString());
    return;
  }

  for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
    auto anim = std::make_shared<Animation>(sanitized_path, model, i);
    anim->SetSockets(sockets);
    std::string name = anim->GetName();
    animations[name] = anim;
    auto animator = std::make_shared<Animator>(anim);
    animator->SetSockets(sockets);
    animators[name] = animator;
  }

  if (!animations.empty()) {
    m_DefaultBoneMatrices = animations.begin()->second->GetDefaultBoneMatrices();
  }

  if (!current_animation.empty() && animators.contains(current_animation)) {
    SetAnimation(current_animation);
  } else if (!animations.empty()) {
    SetAnimation(animations.begin()->first);
  }
}

void Engine::Graphics::ComponentAnimator::SetSockets(
    const std::vector<SocketAttachment> &newSockets) {
  sockets = newSockets;
  for (auto &[name, anim] : animations) {
    anim->SetSockets(sockets);
  }
  for (auto &[name, anim] : animators) {
    anim->SetSockets(sockets);
  }
  if (!animations.empty()) {
    m_DefaultBoneMatrices = animations.begin()->second->GetDefaultBoneMatrices();
  }
}

void Engine::Graphics::ComponentAnimator::AddSocket(
    const SocketAttachment &socket) {
  sockets.push_back(socket);
  SetSockets(sockets);
}

void Engine::Graphics::ComponentAnimator::RemoveSocket(size_t index) {
  if (index < sockets.size()) {
    sockets.erase(sockets.begin() + index);
    SetSockets(sockets);
  }
}

glm::mat4 Engine::Graphics::ComponentAnimator::GetBoneGlobalTransform(
    const std::string &boneName) const {
  if (animation) {
    return animation->GetBoneGlobalTransform(boneName);
  }
  return glm::mat4(1.0f);
}

glm::mat4 Engine::Graphics::ComponentAnimator::GetBoneWorldTransform(
    const std::string &boneName) const {
  glm::mat4 localBone = GetBoneGlobalTransform(boneName);
  auto obj = object.lock();
  if (obj) {
    glm::mat4 modelMatrix =
        glm::translate(glm::mat4(1.0f), obj->_position) *
        glm::mat4_cast(obj->_rotation);
    return modelMatrix * localBone;
  }
  return localBone;
}

std::vector<std::string>
Engine::Graphics::ComponentAnimator::GetNodeNames() const {
  if (!animations.empty()) {
    return animations.begin()->second->GetNodeNames();
  }
  return {};
}

void Engine::Graphics::ComponentAnimator::SetAnimation(std::string path) {
  if (animators.contains(path)) {
    current_animation = path;
    animation = animators[path];
  } else if (path == "") {
    current_animation = path;
    animation = nullptr;
  } else {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Tried to set a non existent animation");
  }
}

void Engine::Graphics::ComponentAnimator::SetIsPlaying(bool state) {
  isPlaying = state;
}
bool Engine::Graphics::ComponentAnimator::GetIsPlaying() { return isPlaying; }
std::shared_ptr<Engine::Graphics::Animator>
Engine::Graphics::ComponentAnimator::GetAnimator() {
  return animation;
}

std::string Engine::Graphics::ComponentAnimator::GetCurrentAnimation() {
  return current_animation;
}

static const std::vector<glm::mat4> s_defaultBoneMatrices(500, glm::mat4(1.0f));

const std::vector<glm::mat4> &
Engine::Graphics::ComponentAnimator::GetFinalBoneMatrices() const {
  if (animation) {
    return animation->GetFinalBoneMatrices();
  }
  if (!m_DefaultBoneMatrices.empty()) {
    return m_DefaultBoneMatrices;
  }
  return s_defaultBoneMatrices;
}

