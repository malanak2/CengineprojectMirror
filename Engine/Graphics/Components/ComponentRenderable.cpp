#include "ComponentRenderable.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include <spdlog/spdlog.h>

using namespace Graphics;

ComponentRenderable::ComponentRenderable(std::string path) {
  this->path = path;
  Load();
}

ComponentRenderable::ComponentRenderable(
    std::string material_path,
    std::map<std::string, std::vector<float>> uniforms
    ) {
  this->_material_path = material_path;
  this->_material = Material::Create(material_path);
  for (auto &[key, val] : _material->program->uniforms) {
    if (!uniforms.contains(key)) {
      SPDLOG_LOGGER_WARN(
          spdlog::get("console"),
          "Component at {} doesnt contain uniform {} specified in material",
          "%instantiated%", key);
    }
  }
  for (auto &[key, val] : uniforms) {
    if (!_material->program->uniforms.contains(key)) {
      SPDLOG_LOGGER_WARN(spdlog::get("console"),
                         "Component at {} specifies uniform {} that is not "
                         "specified in material",
                         "%instantiated%", key);
    }
  }
  auto logger = spdlog::get("console");
  if (!this->_material->usable) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to load material at {}", material_path);
    return;
  }
  this->_uniforms = uniforms;
};

void ComponentRenderable::Setup() {};
void ComponentRenderable::Update() {};
void ComponentRenderable::FixedUpdate() {};

void ComponentRenderable::Save() {
  RenderableDataJson js = ToJson();
  json a = js;
  std::string astr = a;
  if (FileUtil::SaveFile(path, &astr) != 0) {
    SPDLOG_LOGGER_ERROR(spdlog::get("console"),
                        "Failed to save component renderable");
  }
};
void ComponentRenderable::Load() {
  std::string js;
  if (FileUtil::ReadFile(path, &js) != 0) {
    SPDLOG_LOGGER_ERROR(spdlog::get("console"),
                        "Failed to load compononent renderable file.");
    return;
  }
  auto js_p = json::parse(js);
  js_p["data"]["path"] = path;
  FromJson(js_p);
};

/*void ComponentRenderable::FromJson(RenderableJson json_inst) {
  if (json_inst.object_type != "component::renderable") {
    SPDLOG_LOGGER_ERROR(spdlog::get("console"),
                        "Tried to load a component::renderable from a json "
                        "file of different object_type. Path: {}",
                        json_inst.path);
    return;
  }
  this->_material_path = json_inst.material_path;
  this->_material = Material::Create(_material_path);
  auto logger = spdlog::get("console");
  if (!this->_material->usable) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to load material at {}",
                        _material_path);
    return;
  }
  this->_uniforms = json_inst.uniforms;
};*/

/*RenderableJson ComponentRenderable::toJson() {
  RenderableJson j;
  j.object_type = "component:renderable";
  j.material_path = _material_path;
  j.uniforms = _uniforms;
  j.uses_camera = uses_camera;
  return j;
};*/

json ComponentRenderable::ToJson() {
  RenderableDataJson j;
  j.material_path = _material_path;
  j.uniforms = _uniforms;
  return j;
}

ENGINE_COMPONENT_TYPE ComponentRenderable::GetType() {
  return ENGINE_COMPONENT_TYPE::renderable;
}

void ComponentRenderable::FromJson(json &js) {
  JsonFileBase jb;
  jb = js;
  if (jb.object_type != ObjectType::Component) {
    SPDLOG_LOGGER_ERROR(spdlog::get("console"),
                        "Tried to load a component::renderable from a json "
                        "file of different object_type. Path: {}",
                        (std::string)js["data"]["path"]);
  }
  RenderableDataJson json_inst;
  json_inst = js["data"];
  this->_material_path = json_inst.material_path;
  this->_material = Material::Create(_material_path);
  auto logger = spdlog::get("console");
  if (!this->_material->usable) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to load material at {}",
                        _material_path);
    return;
  }
  this->_uniforms = json_inst.uniforms;
};
ComponentRenderable::ComponentRenderable(json &js) { FromJson(js); }
