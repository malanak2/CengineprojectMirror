//
// Created by malan on 30.04.2026.
//

#include "Scene.hpp"
#include "Graphics/Components/CameraComponent.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include "Util/LoggerUtil.hpp"
#include <algorithm>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace Engine
{
  namespace
  {
    constexpr int kMaxSceneDepth = 16;

    std::string NormalizePath(const std::string& p)
    {
      return std::filesystem::path(p).lexically_normal().generic_string();
    }

    struct LegacySceneJson
    {
      std::vector<JsonFileBase> objects;
      std::string default_camera;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LegacySceneJson, objects, default_camera)

    json NormalizeForDiff(json obj)
    {
      if (obj.is_object() && obj.contains("data") && obj["data"].is_object() &&
        obj["data"].contains("components") &&
        obj["data"]["components"].is_array())
      {
        json m = json::object();
        for (const auto& c : obj["data"]["components"])
        {
          if (c.is_object() && c.contains("name") && c["name"].is_string())
            m[c["name"].get<std::string>()] =
              c.contains("data") ? c["data"] : json();
        }
        obj["data"]["components"] = std::move(m);
      }
      return obj;
    }

    json DenormalizeAfterPatch(json obj)
    {
      if (obj.is_object() && obj.contains("data") && obj["data"].is_object() &&
        obj["data"].contains("components") &&
        obj["data"]["components"].is_object())
      {
        json arr = json::array();
        for (auto& it : obj["data"]["components"].items())
          arr.push_back({{"name", it.key()}, {"data", it.value()}});
        obj["data"]["components"] = std::move(arr);
      }
      return obj;
    }
  } // namespace
  std::shared_ptr<Scene> SceneObject::GetRootScene()
  {
    std::shared_ptr<SceneObject> cur = shared_from_this();
    while (auto p = cur->Parent.lock())
      cur = p;
    return std::dynamic_pointer_cast<Scene>(cur);
  }

  std::shared_ptr<Scene> SceneObject::OwnerOf(const std::shared_ptr<SceneObject>& so)
  {
    if (!so)
      return nullptr;
    if (auto s = std::dynamic_pointer_cast<Scene>(so))
      return s;
    return so->scene.lock();
  }

  void SceneObject::SetOwner(const std::shared_ptr<Scene>& owner)
  {
    if (dynamic_cast<Scene*>(this))
      return;
    scene = owner;
    if (instance)
      instance->scene = owner;
    for (const auto& child : Children)
      child->SetOwner(owner);
  }

  void SceneObject::OnDisable(bool ignoreState)
  {
    if (!ignoreState)
    {
      if (!isEnabled)
        return;
    }
    if (instance) instance->OnDisable();
    for (const auto& var : this->Children) var->OnDisable(ignoreState);
  }

  std::shared_ptr<Graphics::CameraComponent>
  SceneObject::BuildInstance(const json& objectJson, const std::shared_ptr<Scene>& owner)
  {
    auto o = std::make_shared<Object>(owner);
    json ref = objectJson;
    auto cam = o->FromJson(ref, nullptr);
    o->sceneobject = shared_from_this();
    this->instance = o;
    o->UpdateWorldTransform();
    return cam;
  }

  CameraList SceneObject::LoadChildren(const std::vector<JsonFileBase>& children,
                                       const std::shared_ptr<Scene>& owner)
  {
    CameraList cams;
    auto self = shared_from_this();
    for (const auto& var : children)
    {
      try
      {
        auto child = CreateFromJson(var, owner, self, cams);
        Children.push_back(child);
      }
      catch (const std::exception& e)
      {
        SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to parse scene object: ({})", e.what());
      }
    }
    return cams;
  }

  std::shared_ptr<SceneObject>
  SceneObject::CreateFromJson(const JsonFileBase& js, const std::shared_ptr<Scene>& owner,
                              const std::shared_ptr<SceneObject>& parent, CameraList& cameras)
  {
    std::shared_ptr<SceneObject> so;
    const bool isObj = js.data.is_object();
    if (isObj && js.data.contains("scene_path"))
      so = Scene::Create();
    else if (isObj && js.data.contains("prefab_path"))
      so = std::make_shared<PrefabSceneObject>();
    else
      so = std::make_shared<SceneObject>();
    auto c = so->FromJson(js, owner, parent);
    cameras.insert(cameras.end(), c.begin(), c.end());
    return so;
  }

  CameraList SceneObject::FromJson(const JsonFileBase& jsbase, const std::shared_ptr<Scene>& owner,
                                   const std::shared_ptr<SceneObject>& parent)
  {
    CameraList camcomps = {};
    if (jsbase.object_type != ObjectType::Object)
      throw std::logic_error("Bad object type");
    SceneObjectJson js = jsbase.data;
    this->Parent = parent;
    this->scene = owner;
    this->_name = js.name;
    if (auto cc = BuildInstance(json(js.data), owner))
      camcomps.push_back(cc);
    auto c = LoadChildren(js.children, owner);
    camcomps.insert(camcomps.end(), c.begin(), c.end());
    return camcomps;
  }


  void SceneObject::SetParent(const std::shared_ptr<SceneObject>& newParent)
  {
    auto self = shared_from_this();
    std::shared_ptr<SceneObject> target = newParent;
    if (!target)
      target = scene.lock();
    if (!target || target == self)
      target = GetRootScene();
    if (!target || target == self)
      return;
    for (auto p = target; p; p = p->Parent.lock())
    {
      if (p == self)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Refusing to parent an object to its own descendant");
        return;
      }
    }
    if (auto old = Parent.lock())
      std::erase(old->Children, self);
    this->Parent = target;
    target->Children.push_back(self);
    SetOwner(OwnerOf(target));
    if (instance)
      instance->UpdateWorldTransform();
  }

  void SceneObject::EarlySetup()
  {
    ZoneScoped;
    if (instance) instance->EarlySetup();
    for (const auto& var : this->Children) var->EarlySetup();
  }

  void SceneObject::Setup()
  {
    ZoneScoped;
    if (instance) instance->Setup();
    for (const auto& var : this->Children) var->Setup();
  }

  void SceneObject::Update()
  {
    if (instance->followsPausedExecution && !isEnabled)
      return;
    if (instance) instance->Update();
    for (const auto& var : this->Children) var->Update();
  }

  void SceneObject::End()
  {
    auto children = this->Children;
    for (const auto& var : children) var->End();
    if (instance) instance->End();
  }

  void SceneObject::LateEnd()
  {
    if (instance) instance->LateEnd();
    for (const auto& var : this->Children) var->LateEnd();
  }

  void SceneObject::OnEnable()
  {
    ZoneScoped;
    if (instance) instance->OnEnable();
    for (const auto& var : this->Children) var->OnEnable();
  }


  bool SceneObject::IsEnabled() const
  {
    return isEnabled;
  }

  JsonFileBase SceneObject::ToJson()
  {
    auto ret = std::make_shared<SceneObjectJson>();
    try
    {
      ret->data = instance->ToJson();
    }
    catch (const std::exception& e)
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                          "Could not convert object to json: {}", e.what());
    }
    std::vector<JsonFileBase> child = {};
    for (const auto& scene_object_json : this->Children)
    {
      child.insert(child.end(), (scene_object_json->ToJson()));
    }
    ret->children = child;
    ret->isEnabled = isEnabled;
    ret->name = _name;
    JsonFileBase retbase;
    retbase.object_type = ObjectType::Object;
    retbase.data = *ret;
    return retbase;
  }


  static std::shared_ptr<SceneObject>
  FindObjectRecursive(const std::shared_ptr<SceneObject>& so,
                      const std::string& name)
  {
    if (so && so->instance && so->_name == name)
      return so;
    if (so)
    {
      for (const auto& child : so->Children)
      {
        auto res = FindObjectRecursive(child, name);
        if (res)
          return res;
      }
    }
    return nullptr;
  }

  std::shared_ptr<Scene> Scene::Create()
  {
    auto s = std::make_shared<Scene>();
    s->Init();
    return s;
  }

  void Scene::Unload()
  {
    ZoneScoped;
    if (isEnabled)
      OnDisable();
    End();
    LateEnd();
    auto root = GetRootScene();
    std::erase_if(root->cameras, [&](const auto& cam)
    {
      auto o = cam->object.lock();
      return !o || isInside(o->sceneobject.lock());
    });
    
    Children.clear();
  }

  void Scene::Init()
  {
    auto self = SelfScene();
    this->scene = self;
    this->instance = std::make_shared<Object>(self);
    this->_name = "New Scene";
    this->instance->sceneobject = self;
  }

  std::shared_ptr<Scene> Scene::Load(std::string path)
  {
    auto s = Create();
    s->path = std::move(path);
    try {
      auto cams = s->LoadFromFile();
      s->ResolveName();
      s->FinalizeCameras(cams);
      return s;
    } catch (std::exception &e)
    {
      return nullptr;
    }
      return nullptr;
  }

  bool Scene::WouldCycle() const
  {
    ZoneScoped;
    const std::string mine = NormalizePath(path);
    int depth = 0;
    for (auto p = Parent.lock(); p; p = p->Parent.lock())
    {
      ++depth;
      if (auto s = std::dynamic_pointer_cast<Scene>(p))
      {
        if (!s->path.empty() && NormalizePath(s->path) == mine)
          return true;
      }
    }
    return depth > kMaxSceneDepth;
  }

  CameraList Scene::LoadFromFile()
  {
    ZoneScoped;
    std::string res;
    if (path == "")
    {
      SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Tried to load scene with no path");
      throw std::logic_error("");
    }
    if (FileUtil::ReadFile(path, &res) != 0)
    {
      SPDLOG_LOGGER_CRITICAL(ENGINE_UTIL_LOGGER, "Failed to open scene at {}", path);
      throw std::logic_error("Failed to open scene at " + path);
    }
    json js;
    try
    {
      js = json::parse(res);
    }
    catch (const std::exception& e)
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to parse json scene at {} ({})", path, e.what());
      throw std::logic_error("Failed to parse json scene at " + path + " " + e.what());
    }
    fileName.clear();
    auto self = SelfScene();
    CameraList cams;
    try
    {
      this->Children.clear();
      if (js.is_object() && js.contains("objects"))
      {
        // legacy format
        LegacySceneJson lj = js;
        this->default_camera = lj.default_camera;
        cams = LoadChildren(lj.objects, self);
      }
      else
      {
        JsonFileBase base = js;
        if (base.object_type != ObjectType::Scene)
          throw std::logic_error("File is not a scene");
        SceneJson sj = base.data;
        fileName = sj.name;
        this->default_camera = sj.default_camera;
        if (sj.data.object_type == ObjectType::Object)
        {
          if (auto cc = BuildInstance(json(sj.data), self))
            cams.push_back(cc);
        }
        auto c = LoadChildren(sj.children, self);
        cams.insert(cams.end(), c.begin(), c.end());
      }
    }
    catch (const std::exception& e)
    {
      SPDLOG_LOGGER_CRITICAL(ENGINE_UTIL_LOGGER, "Failed to convert json to scene {} ({})", path, e.what());
      throw std::logic_error("Failed to convert json to scene " + path + " " + e.what());
    }
    if (instance)
      instance->UpdateWorldTransform();
    return cams;
  }
  CameraList Scene::FromJson(const JsonFileBase& jsbase, const std::shared_ptr<Scene>&,
                             const std::shared_ptr<SceneObject>& parent)
  {
    CameraList cams;
    this->Parent = parent;
    std::string instanceName;
    try
    {
      if (jsbase.object_type != ObjectType::Object)
        throw std::logic_error("Bad object type");
      rawReference = json(jsbase);
      SceneRefJson ref = jsbase.data;
      this->path = ref.scene_path;
      instanceName = ref.name;
      if (WouldCycle())
        throw std::logic_error("Scene " + path + " contains itself (or is nested too deeply)");
      cams = LoadFromFile();
      ApplyRootOverrides(json(ref.data));
      ResolveName(instanceName);
    }
    catch (const std::exception& e)
    {
      if (e.what()[0] != '\0')
        SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to load nested scene {}: ({}). Keeping a placeholder.", path,
                            e.what());
      loadFailed = true;
      Children.clear();
      cams.clear();
      ResolveName(instanceName);
      if (path.empty() && instanceName.empty())
        _name = "Empty path";
    }
    return cams;
  }

  void Scene::ApplyRootOverrides(const json& objectJson)
  {
    ZoneScoped;
    if (!instance || !objectJson.is_object() || objectJson.empty())
      return;
    JsonFileBase base = objectJson.get<JsonFileBase>();
    if (base.object_type != ObjectType::Object)
      return;
    ObjectJson oj = base.data.get<ObjectJson>();
    if (oj.scale.size() == 3)
      instance->scale = glm::vec3(oj.scale[0], oj.scale[1], oj.scale[2]);
    instance->inheritParentRotation = oj.inherit_parent_rotation;
    if (oj.rotation.size() == 4)
      instance->SetRotation(glm::quat(oj.rotation[0], oj.rotation[1], oj.rotation[2], oj.rotation[3]));
    if (oj.position.size() == 3)
      instance->SetPosition(glm::vec3(oj.position[0], oj.position[1], oj.position[2]));
    instance->UpdateWorldTransform();
  }

  bool Scene::Reload()
  {
    ZoneScoped;
    auto self = SelfScene();
    auto root = GetRootScene();
    if (!root)
      return false; // Hmm
    SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Reloading scene {}", path);
    json placement;
    if (!IsRootScene())
    {
      try
      {
        placement = json(ReferenceToJson().data.get<SceneRefJson>().data);
      }
      catch (const std::exception&)
      {
      }
    }

    auto fresh = Create();
    fresh->path = path;
    fresh->Parent = Parent;
    if (fresh->WouldCycle())
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                          "Can't reload {}: it would contain itself", path);
      return false;
    }
    CameraList cams;
    try
    {
      cams = fresh->LoadFromFile();
    }
    catch (const std::exception& e)
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to reload scene {}: ({})",
                          path, e.what());
      return false;
    }

    bool activeLost = false;
    if (root->camera)
    {
      auto o = root->camera->object.lock();
      activeLost = !o || isInside(o->sceneobject.lock());
    }
    
    Unload();

    Children = std::move(fresh->Children);
    fresh->Children.clear();
    instance = fresh->instance;
    if (instance)
    {
      instance->sceneobject = self;
      instance->scene = self;
    }
    for (const auto& c : Children)
    {
      c->Parent = self;
      c->SetOwner(self);
    }
    default_camera = fresh->default_camera;
    fileName = fresh->fileName;
    ResolveName(IsRootScene() ? std::string{} : _name);
    loadFailed = false;
    if (!placement.is_null())
      ApplyRootOverrides(placement);
    if (instance)
      instance->UpdateWorldTransform();

    // Cameras
    if (IsRootScene())
    {
      FinalizeCameras(cams); // uses the reloaded scene's own default camera
    }
    else
    {
      root->cameras.insert(root->cameras.end(), cams.begin(), cams.end());
      if (activeLost)
      {
        auto pick = [&](const std::string& name)
          -> std::shared_ptr<Graphics::CameraComponent>
        {
          if (name.empty())
            return nullptr;
          for (const auto& cam : root->cameras)
          {
            auto o = cam->object.lock();
            if (o && _name == name)
              return cam;
          }
          return nullptr;
        };
        std::shared_ptr<Graphics::CameraComponent> chosen;
        if (auto parentScene = OwnerOf(Parent.lock()))
          chosen = pick(parentScene->default_camera);
        if (!chosen)
          chosen = pick(default_camera);
        if (chosen)
        {
          root->camera = chosen;
        }
        else
        {
          CameraList all = root->cameras;
          root->FinalizeCameras(all); // falls back / creates a default camera
        }
      }
    }
    {
      ZoneScopedN("Enable");
      EarlySetup();
      Setup();
      OnEnable();
    }
    return true;
  }

  std::shared_ptr<Scene> Scene::InstantiateScene(const std::string& scenePath,
                                                 const std::shared_ptr<SceneObject>& parent)
  {
    auto self = SelfScene();
    std::shared_ptr<SceneObject> parentSo = parent ? parent : std::static_pointer_cast<SceneObject>(self);
    auto s = Create();
    s->path = scenePath;
    s->Parent = parentSo;
    if (s->WouldCycle())
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Can't nest scene {}: it would contain itself", scenePath);
      return nullptr;
    }
    CameraList cams;
    try
    {
      cams = s->LoadFromFile();
    }
    catch (const std::exception& e)
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to nest scene {}: ({})", scenePath, e.what());
      return nullptr;
    }
    
    s->ResolveName();
    parentSo->Children.push_back(s);
    if (s->instance)
      s->instance->UpdateWorldTransform();
    if (auto root = GetRootScene())
      root->cameras.insert(root->cameras.end(), cams.begin(), cams.end());
    s->EarlySetup();
    s->Setup();
    s->OnEnable();
    return s;
  }

  void Scene::FinalizeCameras(const CameraList& found)
  {
    ZoneScoped;
    this->cameras = found;
    this->camera = nullptr;
    auto self = SelfScene();
    if (cameras.empty())
    {
      SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER, "Scene {} has no cameras, creating default camera", this->path);
      auto camobj = std::make_shared<Object>(self);
      auto so = std::make_shared<SceneObject>();
      so->_name = "Default Camera";
      so->Parent = self;
      so->scene = self;
      so->instance = camobj;
      camobj->sceneobject = so;
      this->Children.push_back(so);
      camobj->AddComponent<Graphics::CameraComponent>();
      auto cam = camobj->GetComponent<Graphics::CameraComponent>();
      if (cam)
      {
        cam->near = 0.1f;
        cam->far = 1000.0f;
        this->cameras.push_back(cam);
        this->camera = cam;
      }
      camobj->UpdateWorldTransform();
      return;
    }
    for (const auto& cam : cameras)
    {
      auto o = cam->object.lock();
      if (!o) return;
      auto so = o->sceneobject.lock(); 
      if (so && so->_name == default_camera)
      {
        this->camera = cam;
        return;
      }
    }
    SPDLOG_LOGGER_DEBUG(ENGINE_UTIL_LOGGER, "Scene {} is missing its default camera, using first camera", this->path);
    if (cameras.size() == 0)
    {
      SPDLOG_LOGGER_DEBUG(ENGINE_UTIL_LOGGER, "Scene {} is missing cameras", this->path);
      return;
    }
    this->camera = cameras[0];
  }

  JsonFileBase Scene::FileToJson()
  {
    SceneJson sj;
    try
    {
      sj.data = instance->ToJson().get<JsonFileBase>();
    }
    catch (const std::exception& e)
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Could not convert scene root to json: {}", e.what());
    }
    for (const auto& child : Children)
    {
      if (!child->saveable) continue;
      sj.children.push_back(child->ToJson());
    }
    sj.name = IsRootScene() ? _name : fileName;
    sj.default_camera = default_camera;
    JsonFileBase out;
    out.object_type = ObjectType::Scene;
    out.data = sj;
    return out;
  }

  JsonFileBase Scene::ReferenceToJson()
  {
    if (loadFailed && !rawReference.is_null())
      return rawReference.get<JsonFileBase>();
    SceneRefJson ref;
    ref.scene_path = path;
    ref.name = _name;
    JsonFileBase ob = instance->ToJson().get<JsonFileBase>();
    ObjectJson oj = ob.data.get<ObjectJson>();
    oj.components.clear();
    ob.data = oj;
    ref.data = ob;
    JsonFileBase out;
    out.object_type = ObjectType::Object;
    out.data = ref;
    return out;
  }

  JsonFileBase Scene::ToJson()
  {
    if (Parent.lock())
      return ReferenceToJson();
    return FileToJson();
  }

  CameraList PrefabSceneObject::FromJson(
    const JsonFileBase& jsbase, const std::shared_ptr<Scene>& owner,
    const std::shared_ptr<SceneObject>& parent)
  {
    CameraList cams = {};
    try
    {
      if (jsbase.object_type != ObjectType::Object)
        throw std::logic_error("Bad object type");
      PrefabJson js = jsbase.data;
      this->Parent = parent;
      this->scene = owner;
      this->_name = js.name;
      this->prefab_path = js.prefab_path;
      this->overrides = js.overrides;
      std::string res;
      if (FileUtil::ReadFile(this->prefab_path, &res) != 0)
      {
        SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to open prefab at {}",
                            prefab_path);
        throw std::logic_error("Failed to open prefab");
      }
      JsonFileBase sjsfb = json::parse(res);
      SceneObjectJson sjs = sjsfb.data;
      nodes.clear();
      baseJson.clear();
      cams = LoadNode(shared_from_this(), sjs, {}, owner);
      hasBase = true;
    }
    catch (const std::exception& e)
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                          "Failed to parse scene object: ({})", e.what());
      throw;
    }
    return cams;
  }

  CameraList PrefabSceneObject::LoadNode(
    const std::shared_ptr<SceneObject>& node, const SceneObjectJson& js,
    const std::vector<int>& path, const std::shared_ptr<Scene>& owner)
  {
    CameraList cams;
    node->scene = owner;
    // Build from the file first: that is the base the overrides are against
    auto cc = node->BuildInstance(json(js.data), owner);
    json base = NormalizeForDiff(node->instance->ToJson());
    baseJson[path] = base;
    nodes.emplace_back(node, path);

    for (const auto& ov : overrides)
    {
      if (ov.path != path)
        continue;
      json patched = base;
      for (const auto& op : ov.patch)
      {
        // op by op: one bad op doesn't kill the rest
        try
        {
          patched = patched.patch(json::array({op}));
        }
        catch (const std::exception& e)
        {
          SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                             "Skipping bad prefab override in {}: {}",
                             prefab_path, e.what());
        }
      }
      try
      {
        cc = node->BuildInstance(DenormalizeAfterPatch(patched), owner);
      }
      catch (const std::exception& e)
      {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                           "Failed to apply overrides in {}: {}", prefab_path,
                           e.what());
      }
      break;
    }
    if (cc)
      cams.push_back(cc);

    int idx = 0;
    for (const auto& childJs : js.children)
    {
      std::vector<int> childPath = path;
      childPath.push_back(idx++);
      try
      {
        const bool nestedRef =
          childJs.data.is_object() && (childJs.data.contains("prefab_path") ||
            childJs.data.contains("scene_path"));
        if (nestedRef)
        {
          // nested prefab / scene manage themselves
          auto child = CreateFromJson(childJs, owner, node, cams);
          node->Children.push_back(child);
        }
        else
        {
          if (childJs.object_type != ObjectType::Object)
            throw std::logic_error("Bad object type");
          SceneObjectJson cjs = childJs.data;
          auto child = std::make_shared<SceneObject>();
          child->Parent = node;
          auto c = LoadNode(child, cjs, childPath, owner);
          cams.insert(cams.end(), c.begin(), c.end());
          node->Children.push_back(child);
        }
      }
      catch (const std::exception& e)
      {
        SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                            "Failed to parse scene object: ({})", e.what());
      }
    }
    return cams;
  }

  void PrefabSceneObject::CaptureTracking(
    const std::shared_ptr<SceneObject>& node, const std::vector<int>& path)
  {
    nodes.emplace_back(node, path);
    if (node->instance)
      baseJson[path] = NormalizeForDiff(node->instance->ToJson());
    int idx = 0;
    for (const auto& child : node->Children)
    {
      std::vector<int> cp = path;
      cp.push_back(idx++);
      if (std::dynamic_pointer_cast<PrefabSceneObject>(child) ||
        std::dynamic_pointer_cast<Scene>(child))
        continue; // manage themselves
      CaptureTracking(child, cp);
    }
  }

  std::vector<PrefabOverride> PrefabSceneObject::ComputeOverrides()
  {
    std::vector<PrefabOverride> out;
    for (const auto& [weak, path] : nodes)
    {
      auto so = weak.lock();
      if (!so || !so->instance)
        continue; // removed objects can't be expressed; they come back on load
      auto it = baseJson.find(path);
      if (it == baseJson.end())
        continue;
      json patch =
        json::diff(it->second, NormalizeForDiff(so->instance->ToJson()));
      if (!patch.empty())
        out.push_back({path, patch});
    }
    return out;
  }

  JsonFileBase PrefabSceneObject::ToJson()
  {
    if (!hasBase) // never loaded from / saved to a file: create it once
      SavePrefab();
    PrefabJson out;
    out.prefab_path = prefab_path;
    if (hasBase)
      out.overrides = ComputeOverrides();
    JsonFileBase b;
    b.object_type = ObjectType::Object;
    b.data = out;
    return b;
  }

  bool PrefabSceneObject::SavePrefab()
  {
    if (prefab_path.empty() || !instance)
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                          "Can't save a prefab with no path");
      return false;
    }
    SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Saving prefab to {}", prefab_path);
    SceneObjectJson save;
    save.data = instance->ToJson().get<JsonFileBase>();
    for (const auto& c : Children)
      save.children.push_back(c->ToJson());
    save.isEnabled = isEnabled;
    JsonFileBase savebase;
    savebase.object_type = ObjectType::Object;
    savebase.data = save;
    std::string str = json(savebase).dump();
    if (FileUtil::SaveFile(prefab_path, &str) != 0)
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to save prefab to {}",
                          prefab_path);
      return false;
    }
    overrides.clear();
    nodes.clear();
    baseJson.clear();
    CaptureTracking(shared_from_this(), {});
    hasBase = true;
    return true;
  }

  bool Scene::Save()
  {
    if (loadFailed || !saveable)
      return true;
    if (path.empty())
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Can't save a scene with no path");
      return false;
    }
    bool ok = true;
    std::function<void(const std::shared_ptr<SceneObject>&)> saveNested =
      [&](const std::shared_ptr<SceneObject>& so)
    {
      for (const auto& c : so->Children)
      {
        if (auto s = std::dynamic_pointer_cast<Scene>(c))
          ok = s->Save() && ok;
        else
          saveNested(c);
      }
    };
    saveNested(shared_from_this());
    SPDLOG_LOGGER_INFO(ENGINE_UTIL_LOGGER, "Saving scene to {}...", path);
    std::string str = json(FileToJson()).dump(2);
    if (FileUtil::SaveFile(path, &str) != 0)
    {
      SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER, "Failed to save scene to {}", path);
      ok = false;
    }
    return ok;
  }

  void Scene::Instantiate(const std::shared_ptr<Object>& object,
                          const std::shared_ptr<SceneObject>& parent, bool asPrefab)
  {
    std::shared_ptr<SceneObject> so;
    if (asPrefab) so = std::make_shared<PrefabSceneObject>();
    else so = std::make_shared<SceneObject>();
    std::shared_ptr<SceneObject> p = parent ? parent : std::static_pointer_cast<SceneObject>(SelfScene());
    auto owner = OwnerOf(p);
    so->Parent = p;
    so->scene = owner;
    p->Children.push_back(so);
    object->scene = owner;
    object->sceneobject = so;
    so->instance = object;
    object->UpdateWorldTransform();
    so->EarlySetup();
    so->Setup();
    so->OnEnable();
  }

  void Scene::RemoveObject(const std::shared_ptr<SceneObject>& so)
  {
    if (!so || so.get() == this) return;
    auto parent = so->Parent.lock();
    if (!parent) return;
    if (so->instance)
    {
      so->instance->OnDisable();
      so->instance->End();
      so->instance->LateEnd();
    }
    auto kids = so->Children;
    so->Children.clear();
    std::erase(parent->Children, so);
    so->Parent.reset();
    auto owner = OwnerOf(parent);
    for (const auto& child : kids)
    {
      child->Parent = parent;
      parent->Children.push_back(child);
      child->SetOwner(owner);
      if (child->instance) child->instance->UpdateWorldTransform();
    }
  }

  void Scene::RemoveObjectCascade(const std::shared_ptr<SceneObject>& so)
  {
    if (!so || so.get() == this) return;
    auto parent = so->Parent.lock();
    if (!parent) return;
    so->OnDisable();
    so->End();
    so->LateEnd();
    std::erase(parent->Children, so);
    so->Parent.reset();
  }

  std::shared_ptr<SceneObject> Scene::FindObject(const std::string& name)
  {
    for (const auto& child : Children)
    {
      auto res = FindObjectRecursive(child, name);
      if (res) return res;
    }
    if (!IsRootScene())
    {
      auto root = GetRootScene();
      if (root && root.get() != this)
        return root->FindObject(name);
    }
    return nullptr;
  }
  
  void Scene::ResolveName(const std::string& instanceName)
  {
    if (!instanceName.empty())
      _name = instanceName;
    else if (!path.empty())
      _name = std::filesystem::path(path).stem().string();
    else
      _name = "New Scene";
  }
} // namespace Engine
