#pragma once
#include "Scene.hpp"
#include "ScriptSystem.hpp"

namespace Editor
{
  class [[=Engine::Attr::Script{}]] ComponentUserScene : public Engine::IComponent
  {
    REGISTER_CLASS(ComponentUserScene)
  public:
    void EarlySetup() override
    {
      Load();
      if (isLoaded)
        sceneHolder->EarlySetup();
    }
    void Setup() override
    {
      
      if (isLoaded) {
        sceneHolder->Setup();
      }
    }

    void OnEnable() override
    {
    }

    void OnDisable() override
    {
    }

    void Update() override
    {
      // TODO: Figure out a better way to do this
      if (!hasDisabled)
      {
        hasDisabled = true;
        sceneHolder->SetIsEnabled(false);
      }
      if (isLoaded && isPlaying && !isPaused)
      {
        for (const auto& var : scenePlayHolder->Children) var->Update();
      }
    }

    [[=Engine::Attr::Editor{}]] void Play()
    {
      if (isPlaying)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Tried to play when already in play mode");
        return;
      }
      auto js = sceneHolder->ToJson();
      Engine::CameraList cams;
      scenePlayHolder = std::static_pointer_cast<Engine::Scene>(Engine::Scene::CreateFromJson(js, object.lock()->sceneobject.lock()->GetRootScene(),  object.lock()->sceneobject.lock(), cams));
      object.lock()->sceneobject.lock()->Children.push_back(scenePlayHolder);
      scenePlayHolder->saveable = false;
      // We manage this from here, so it would kinda be not great if the engine did its thing
      scenePlayHolder->isEnabled = false;
      scenePlayHolder->EarlySetup();
      scenePlayHolder->Setup();
      scenePlayHolder->OnEnable();
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
      object.lock()->scene.lock()->RemoveObjectCascade(scenePlayHolder);
      scenePlayHolder = nullptr;
      isPlaying = false;
    }

    [[=Engine::Attr::Editor{}]] void Load()
    {
      if (isPlaying)
      {
          SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Cannot load scene while playing");
        return;
      }
      
      if (isLoaded)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Cannot load when loaded");
        return;
      }
      sceneHolder = Engine::Scene::Load(path);
      if (sceneHolder == nullptr)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Tried to load scene, but nullptr");
        return;
      }
      sceneHolder->SetParent(object.lock()->sceneobject.lock());
      ptrpath = &sceneHolder->path;
      // We will manage the object here - no checking if it is enabled needed;
      sceneHolder->saveable = false;
      isLoaded = true;
    }

    [[=Engine::Attr::Editor{}]] void Unload()
    {
      
      if (isPlaying)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Cannot unload scene while playing");
        return;
      }
      if (!isLoaded)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Cannot unload when not loaded");
        return;
      }
      sceneHolder->End();
      sceneHolder->LateEnd();
      auto root = sceneHolder->GetRootScene();
      std::erase_if(root->cameras, [&](const auto& cam)
      {
        auto o = cam->object.lock();
        return !o || sceneHolder->isInside(o->sceneobject.lock());
      });
    
      sceneHolder->Children.clear();
      ptrpath = nullptr;
      isLoaded = false;
    }

    [[=Engine::Attr::Editor{}]] [[=Engine::Attr::ReadOnly{}]] bool isPlaying = false;
    [[=Engine::Attr::Editor{}]] [[=Engine::Attr::ReadOnly{}]] bool isPaused = false;
    [[=Engine::Attr::Editor{}]] [[=Engine::Attr::ReadOnly{}]] bool isLoaded = false;

    [[=Engine::Attr::Editor{}]] [[=Engine::Attr::ReadOnly{}]] std::string *ptrpath;
    
    [[=Engine::Attr::Editor{}]][[=Engine::Attr::Serialize{}]] std::string path;

  private:
    std::shared_ptr<Engine::Scene> sceneHolder;
    bool hasDisabled = false;
    std::shared_ptr<Engine::Scene> scenePlayHolder;
  };
}