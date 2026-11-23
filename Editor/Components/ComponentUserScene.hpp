#pragma once
#include "Scene.hpp"
#include "ScriptSystem.hpp"

namespace Editor
{
  class [[=Engine::Attr::Script{}]] ComponentUserScene : public Engine::IComponent
  {
    REGISTER_CLASS(ComponentUserScene)
  public:
    void Setup() override
    {
      sceneHolder = std::make_shared<Engine::Object>();
      sceneHolder->_name = "User scene holder";
      object.lock()->scene.lock()->Instantiate(sceneHolder, object.lock()->sceneobject.lock());
      // We will manage the object here - no checking if it is enabled needed;
      sceneHolder->sceneobject.lock()->SetIsEnabled(false);
    }

    void OnEnable() override
    {
      sceneHolder->OnEnable();
      for (const auto& var : sceneHolder->sceneobject.lock()->Children) var->OnEnable();
    }

    void OnDisable() override
    {
      sceneHolder->OnDisable();
      for (const auto& var : sceneHolder->sceneobject.lock()->Children) var->OnDisable();
    }

    void Update() override
    {
      if (isPlaying && !isPaused)
      {
        sceneHolder->Update();
        for (const auto& var : sceneHolder->sceneobject.lock()->Children) var->Update();
      }
    }

    [[=Engine::Attr::Editor{}]] void Play()
    {
      if (isPlaying)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Tried to play when already in play mode");
        return;
      }
      auto js = sceneHolder->sceneobject.lock()->ToJson();
      auto so = std::make_shared<Engine::SceneObject>();
      so->FromJson(js, sceneHolder->sceneobject.lock()->scene.lock(), object.lock()->sceneobject.lock());
      scenePlayHolder = so->instance;
      scenePlayHolder->_name = "User scene play holder";
      object.lock()->sceneobject.lock()->Children.push_back(so);
      so->EarlySetup();
      so->Setup();
      so->OnEnable();
      isPlaying = true;
      isPaused = false;
    }

    [[=Engine::Attr::Editor{}]] void Pause()
    {
      isPaused = true;
    }

    [[=Engine::Attr::Editor{}]] void Unpause()
    {
      isPaused = false;
    }

    [[=Engine::Attr::Editor{}]] void Stop()
    {
      if (!isPlaying)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Tried to stop when not in play mode");
        return;
      }
      object.lock()->scene.lock()->RemoveObjectCascade(scenePlayHolder->sceneobject.lock());
      scenePlayHolder = nullptr;
      isPlaying = false;
    }

    [[=Engine::Attr::Editor{}]] void Load()
    {
    }

    [[=Engine::Attr::Editor{}]] void Unload()
    {
    }

    [[=Engine::Attr::Editor{}]] [[=Engine::Attr::ReadOnly{}]] bool isPlaying = false;
    [[=Engine::Attr::Editor{}]] [[=Engine::Attr::ReadOnly{}]] bool isPaused = false;
    [[=Engine::Attr::Editor{}]] [[=Engine::Attr::ReadOnly{}]] bool isLoaded = false;

    [[=Engine::Attr::Editor{}]] [[=Engine::Attr::Serialize{}]] std::string path;

  private:
    std::shared_ptr<Engine::Object> sceneHolder;
    std::shared_ptr<Engine::Object> scenePlayHolder;
  };
}