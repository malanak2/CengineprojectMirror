#include "ComponentRenderable.hpp"
#include "Util/FileUtil.hpp"
#include <spdlog/spdlog-inl.h>

using namespace Graphics;

ComponentRenderable::ComponentRenderable(std::string path) { Load(path); }

ComponentRenderable::ComponentRenderable(
    std::string material_path,
    std::unordered_map<std::string, std::vector<float>> uniforms) {
  this->_material_path = material_path;
  this->_material = Material::Create(material_path);
  auto logger = spdlog::get("console");
  if (!this->_material->usable) {
    logger->error("Failed to load material at {}", material_path);
    return;
  }
  this->_uniforms = uniforms;
};

void ComponentRenderable::Setup(){};
void ComponentRenderable::Update(){};
void ComponentRenderable::FixedUpdate(){};

void ComponentRenderable::Save(std::string path) {
  RenderableJson js = ToJson();
  json a = js;
  std::string astr = a;
  if (FileUtil::SaveFile(path, &astr) != 0) {
    spdlog::get("console")->error("Failed to save component");
  }
};
void ComponentRenderable::Load(std::string path) {
  std::string js;
  if (FileUtil::ReadFile(path, &js) != 0) {
    spdlog::get("console")->error("Failed to load comprenderable file.");
    return;
  }
  FromJson(json::parse(js));
};

void ComponentRenderable::FromJson(RenderableJson json_inst) {
  this->_material_path = json_inst.material_path;
  this->_material = Material::Create(_material_path);
  auto logger = spdlog::get("console");
  if (!this->_material->usable) {
    logger->error("Failed to load material at {}", _material_path);
    return;
  }
  this->_uniforms = json_inst.uniforms;
};

RenderableJson ComponentRenderable::ToJson() {
  RenderableJson j;
  j.material_path = _material_path;
  j.uniforms = _uniforms;
  return j;
};
