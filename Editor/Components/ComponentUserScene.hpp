#pragma once
#include "Engine.hpp"
#include "Scene.hpp"
#include "ScriptSystem.hpp"
#include "Physics/PhysicsSystem.hpp"

namespace Editor
{
  class [[=Engine::Attr::Script{}]] ComponentUserScene : public Engine::IComponent
  {
    REGISTER_CLASS(ComponentUserScene)
  public:
    
    [[=Engine::Attr::Editor{}]]
    [[=Engine::Attr::ReadOnly{}]]
    bool isPlaying = false;
    
    [[=Engine::Attr::Editor{}]]
    [[=Engine::Attr::ReadOnly{}]]
    bool isPaused = false;
    
    [[=Engine::Attr::Editor{}]]
    [[=Engine::Attr::ReadOnly{}]]
    bool isLoaded = false;
    
    void EarlySetup() override
    {
      Engine::Engine::isSimulationEnabled = false;
    }

    void Setup() override
    {
    }

    void OnEnable() override
    {
      Load();
    }

    void OnDisable() override
    {
    }

    void Update() override
    {
      if (isLoaded && isPlaying && !isPaused)
      {
        for (const auto& var : scenePlayHolder->Children) var->Update();
      }
    }

    [[=Engine::Attr::Editor{.isDisabled = ^^isPlaying}]]
    void Play()
    {
      if (isPlaying)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Tried to play when already in play mode");
        return;
      }
      if (!isLoaded)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Cannot play when not loaded");
        return;
      }
      auto js = sceneHolder->ToJson();
      Engine::CameraList cams;
      scenePlayHolder = std::static_pointer_cast<Engine::Scene>(Engine::Scene::CreateFromJson(
        js, object.lock()->sceneobject.lock()->GetRootScene(), object.lock()->sceneobject.lock(), cams));
      object.lock()->sceneobject.lock()->Children.push_back(scenePlayHolder);
      scenePlayHolder->saveable = false;
      // We manage this from here, so it would kinda be not great if the engine did that it is supposed to do
      scenePlayHolder->isEnabled = false;
      scenePlayHolder->_name = "User scene play mode holder";
      sceneHolder->OnDisable();
      scenePlayHolder->EarlySetup();
      scenePlayHolder->Setup();
      scenePlayHolder->OnEnable();
      isPlaying = true;
      isPaused = false;
      Engine::Engine::isSimulationEnabled = true;
    }

    bool CanPause() const
    {
      return !isPaused && isPlaying;
    }
    [[=Engine::Attr::Editor{.isDisabled = ^^CanPause, .invertDisabled = true}]]
    void Pause()
    {
      isPaused = true;

      Engine::Engine::isSimulationEnabled = false;
    }
    bool CanUnpause() const
    {
      return isPaused && isPlaying;
    }
    [[=Engine::Attr::Editor{.isDisabled = ^^CanUnpause, .invertDisabled = true}]]
    void Unpause()
    {
      isPaused = false;

      Engine::Engine::isSimulationEnabled = true;
    }

    [[=Engine::Attr::Editor{.isDisabled = ^^isPlaying, .invertDisabled = true}]]
    void Stop()
    {
      if (!isPlaying)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Tried to stop when not in play mode");
        return;
      }
      object.lock()->scene.lock()->RemoveObjectCascade(scenePlayHolder);

      scenePlayHolder = nullptr;
      Engine::Engine::isSimulationEnabled = false;
      sceneHolder->OnEnable();
      isPlaying = false;
    }
    bool CanLoad() const
    {
      return !isPlaying && !isLoaded;
    }
    [[=Engine::Attr::Editor{.isDisabled = ^^CanLoad, .invertDisabled = true}]]
    void Load()
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
      sceneHolder->_name = "User scene holder";
      sceneHolder->isEnabled = false;
      sceneHolder->EarlySetup();
      sceneHolder->Setup();
      sceneHolder->OnEnable();
      isLoaded = true;
    }
    bool CanUnload() const
    {
      return !isPlaying && isLoaded;
    }
    [[=Engine::Attr::Editor{.isDisabled = ^^CanUnload, .invertDisabled = true}]]
    void Unload()
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
      root->RemoveObjectCascade(sceneHolder);
      sceneHolder = nullptr;
      ptrpath = nullptr;
      isLoaded = false;
    }

    

    [[=Engine::Attr::Editor{}]]
    [[=Engine::Attr::ReadOnly{}]]
    std::string* ptrpath;

    [[=Engine::Attr::Editor{}]]
    [[=Engine::Attr::Serialize{}]]
    std::string path;

  private:
    std::shared_ptr<Engine::Scene> sceneHolder;
    bool hasDisabled = false;
    std::shared_ptr<Engine::Scene> scenePlayHolder;
  };
}
