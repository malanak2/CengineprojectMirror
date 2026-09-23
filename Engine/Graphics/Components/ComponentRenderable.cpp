#include "ComponentRenderable.hpp"
#include "Graphics/Material.hpp"
#include "Graphics/Model.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Util/LoggerUtil.hpp"
#include <memory>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>
using namespace Engine::Graphics;

ComponentRenderable::ComponentRenderable(std::string model_path,
                                         std::string material_path,
                                         std::shared_ptr<Object> object) {
  ZoneScoped;
  this->object = object;
  FromData(model_path, material_path, {});
  if (material) {
    material->renderableObjects.push_back(shared_from_this());
  }
}

void ComponentRenderable::FromData(
    std::string model_path, std::string material_path,
    std::map<std::string, std::shared_ptr<IUniform>> uniforms,
    std::vector<std::string> disabled_meshes) {
  ZoneScoped;
  _model_path = model_path;
  _model_path.reserve(100);
  model = Model::Create(model_path);

  _material_path = material_path;
  _material_path.reserve(100);
  material = Material::Create(material_path);
  this->disabled_meshes = disabled_meshes;

  auto logger = spdlog::get("console");
  if (!material || !material->usable) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to load material at {}", material_path);
    return;
  }
  for (auto &[key, val] : material->program->uniforms) {
    if (!uniforms.contains(key) && key != "camera") {
      SPDLOG_LOGGER_WARN(
          spdlog::get("console"),
          "Component at {} doesnt contain uniform {} specified in material",
          "%instantiated%", key);
    }
  }
  for (auto &[key, val] : uniforms) {
    if (!material->program->uniforms.contains(key) && key != "camera") {
      SPDLOG_LOGGER_WARN(spdlog::get("console"),
                         "Component at {} specifies uniform {} that is not "
                         "specified in material",
                         "%instantiated%", key);
    } else if (material->program->uniforms.contains(key)) {
      if (!val) {
        val = material->program->uniforms[key];
      } else if (material->program->uniforms[key]) {
        val->info.id = material->program->uniforms[key]->info.id;
        val->info.offset = material->program->uniforms[key]->info.offset;
        if (!val->info.name || val->info.name->empty()) {
          val->info.name = material->program->uniforms[key]->info.name;
        }
      }
    }
  }
  this->_uniforms = uniforms;
}

std::vector<bool> ComponentRenderable::GetEnabledMeshes() const {
  if (!model)
    return {};
  const auto &mList = model->GetMeshes();
  std::vector<bool> result(mList.size(), true);
  for (size_t i = 0; i < mList.size(); ++i) {
    if (std::find(disabled_meshes.begin(), disabled_meshes.end(),
                  mList[i].name) != disabled_meshes.end()) {
      result[i] = false;
    }
  }
  return result;
}

bool ComponentRenderable::IsMeshEnabled(const std::string &meshName) const {
  return std::find(disabled_meshes.begin(), disabled_meshes.end(),
                   meshName) == disabled_meshes.end();
}

void ComponentRenderable::SetMeshEnabled(const std::string &meshName,
                                         bool enabled) {
  auto it = std::find(disabled_meshes.begin(), disabled_meshes.end(), meshName);
  if (!enabled && it == disabled_meshes.end()) {
    disabled_meshes.push_back(meshName);
  } else if (enabled && it != disabled_meshes.end()) {
    disabled_meshes.erase(it);
  }
}

void ComponentRenderable::Setup() {};
void ComponentRenderable::Update() {};
void ComponentRenderable::FixedUpdate() {}
void Engine::Graphics::ComponentRenderable::SetObject(
    std::shared_ptr<Object> object) {
  this->object = object;
};

json ComponentRenderable::ToJson() {
  JsonFileBase jb;
  RenderableDataJson j;
  j.model_path = _model_path;
  j.material_path = _material_path;
  j.uniforms = this->_uniforms;
  j.disabled_meshes = this->disabled_meshes;
  jb.object_type = ObjectType::Component;
  jb.data = j;
  return jb;
}

std::shared_ptr<ComponentRenderable>
ComponentRenderable::Create(json &js, std::shared_ptr<Object> object) {
  JsonFileBase jb;
  jb = js;
  if (jb.object_type != ObjectType::Component) {
    SPDLOG_LOGGER_ERROR(spdlog::get("console"),
                        "Tried to load a component::renderable from a json "
                        "file of different object_type.");
  }
  RenderableDataJson json_inst;
  json_inst = js["data"];
  std::shared_ptr<ComponentRenderable> cr =
      std::make_shared<ComponentRenderable>();
  cr->SetObject(object);
  cr->FromData(json_inst.model_path, json_inst.material_path,
               json_inst.uniforms, json_inst.disabled_meshes);
  if (cr->material) {
    cr->material->renderableObjects.push_back(cr);
  }
  return cr;
};

void ComponentRenderable::FromJson(json &js) {
  JsonFileBase file_base;
  file_base = js;
  if (file_base.object_type != ObjectType::Component) {
    SPDLOG_LOGGER_ERROR(ENGINE_UTIL_LOGGER,
                        "Tried to load a component::renderable from a json "
                        "file of different object_type.");
    return;
  }
  RenderableDataJson json_inst;
  json_inst = js["data"];
  FromData(json_inst.model_path, json_inst.material_path, json_inst.uniforms,
           json_inst.disabled_meshes);
  if (material) {
    material->renderableObjects.push_back(shared_from_this());
  }
}

ComponentRenderable::ComponentRenderable(json &js) { FromJson(js); }

ComponentRenderable::ComponentRenderable() {}
