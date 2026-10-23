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
    if (animations.contains(current_animation) || current_animation.empty()) {
      SetAnimation(current_animation);
    }
  }
  if (js.contains("is_playing") && js["is_playing"].is_boolean()) {
    SetIsPlaying(js["is_playing"]);
  }
}
void Engine::Graphics::ComponentAnimator::Update() {
  ZoneScoped;
  if (isPlaying && animator) {
    animator->UpdateAnimation(GetDeltaTime());
  }
}
void Engine::Graphics::ComponentAnimator::EarlySetup() {
  auto obj = object.lock();
  if (!obj)
    return;
  auto renderable = obj->GetComponent<Engine::Graphics::ComponentRenderable>();
  if (renderable && renderable->model)
    LoadAnimationsFromModel(renderable->_model_path, renderable->model);
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
  animations.clear();
  for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
    std::shared_ptr<Animation> anim;
    try {
      anim = std::make_shared<Animation>(sanitized_path, model, i);
    } catch (const std::exception &e) {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Skipping animation {} of {}: {}",
                          i, sanitized_path, e.what());
      continue;
    }
    anim->SetSockets(sockets);
    animations[anim->GetName()] = anim;
  }
  animator->SetSockets(sockets); // the animator that actually plays

  if (!animations.empty())
    m_DefaultBoneMatrices =
        animations.begin()->second->GetDefaultBoneMatrices();
  if (!current_animation.empty() && animations.contains(current_animation))
    SetAnimation(current_animation);
  else if (!animations.empty())
    SetAnimation(animations.begin()->first);
}

void Engine::Graphics::ComponentAnimator::SetSockets(
    const std::vector<SocketAttachment> &newSockets) {

  sockets = newSockets;
  for (auto &[name, anim] : animations)
    anim->SetSockets(sockets);
  animator->SetSockets(sockets);
  if (!animations.empty())
    m_DefaultBoneMatrices =
        animations.begin()->second->GetDefaultBoneMatrices();
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
  if (animator) {
    return animator->GetBoneGlobalTransform(boneName);
  }
  return glm::mat4(1.0f);
}

glm::mat4 Engine::Graphics::ComponentAnimator::GetBoneWorldTransform(
    const std::string &boneName) const {
  glm::mat4 localBone = GetBoneGlobalTransform(boneName);
  auto obj = object.lock();
  if (obj) {
    glm::mat4 modelMatrix =
        glm::translate(glm::mat4(1.0f), obj->GetPosition()) *
        glm::mat4_cast(obj->GetRotation()) *
        glm::scale(glm::mat4(1.0f), obj->scale);
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

  auto it = animations.find(path);
  if (it != animations.end()) {
    current_animation = path;
    animator->PlayAnimation(it->second);
  } else if (path.empty()) {
    current_animation = path;
    animator->PlayAnimation(nullptr);
  } else {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Tried to set a non existent animation: {}", path);
  }
}

void Engine::Graphics::ComponentAnimator::SetIsPlaying(bool state) {
  isPlaying = state;
}
bool Engine::Graphics::ComponentAnimator::GetIsPlaying() { return isPlaying; }
std::shared_ptr<Engine::Graphics::Animator>
Engine::Graphics::ComponentAnimator::GetAnimator() {
  return animator;
}

std::string Engine::Graphics::ComponentAnimator::GetCurrentAnimation() {
  return current_animation;
}

static const std::vector<glm::mat4> s_defaultBoneMatrices(500, glm::mat4(1.0f));

const std::vector<glm::mat4> &
Engine::Graphics::ComponentAnimator::GetFinalBoneMatrices() const {
  if (animator) {
    return animator->GetFinalBoneMatrices();
  }
  if (!m_DefaultBoneMatrices.empty()) {
    return m_DefaultBoneMatrices;
  }
  return s_defaultBoneMatrices;
}
