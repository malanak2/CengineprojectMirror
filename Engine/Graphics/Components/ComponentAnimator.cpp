#include "ComponentAnimator.hpp"
#include "Graphics/Components/ComponentRenderable.hpp"
#include "Object.hpp"
#include "Util/LoggerUtil.hpp"
#include <memory>
#include <tracy/Tracy.hpp>
json Engine::Graphics::ComponentAnimator::ToJson() { return {}; }
void Engine::Graphics::ComponentAnimator::FromJson(json &js) {}
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
    std::string name = anim->GetName();
    animations[name] = anim;
    animators[name] = std::make_shared<Animator>(anim);
  }

  if (!animations.empty() && current_animation.empty()) {
    SetAnimation(animations.begin()->first);
  }
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
