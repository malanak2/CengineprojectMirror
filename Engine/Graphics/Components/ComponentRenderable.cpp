#include "ComponentRenderable.hpp"
#include "Graphics/Graphics.hpp"
#include "Interfaces/IComponent.hpp"
#include "JsonFileBase.hpp"
#include "Util/FileUtil.hpp"
#include <memory>
#include <spdlog/spdlog.h>

using namespace Engine::Graphics;

ComponentRenderable::ComponentRenderable(std::string path) {
  this->path = path;
  Load();
}
/*
std::shared_ptr<ComponentRenderable> ComponentRenderable::Create(
    std::string material_path,
    std::map<std::string, std::vector<float>> uniforms) {
  auto cr = std::make_shared<ComponentRenderable>();
  cr->_material_path = material_path;
  std::shared_ptr<Material> m =
      std::static_pointer_cast<Material>(cr->FromData(material_path, uniforms));
  m->renderableObjects.insert(m->renderableObjects.end(), cr);
  return cr;
};*/

std::shared_ptr<void> ComponentRenderable::FromData(
    std::string material_path,
    std::map<std::string, std::vector<float>> uniforms) {
  auto _material = Material::Create(material_path);
  auto logger = spdlog::get("console");
  if (!_material->usable) {
    SPDLOG_LOGGER_ERROR(logger, "Failed to load material at {}", material_path);
    return _material;
  }
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
  this->_uniforms = uniforms;
  return _material;
}

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
  JsonFileBase jb;
  RenderableDataJson j;
  j.material_path = _material_path;
  j.uniforms = _uniforms;
  j.indices = indices;
  j.vertices = vertices;
  jb.object_type = ObjectType::Component;
  jb.data = j;
  return jb;
}

Engine::ENGINE_COMPONENT_TYPE ComponentRenderable::GetType() {
  return ENGINE_COMPONENT_TYPE::renderable;
}

std::shared_ptr<ComponentRenderable> ComponentRenderable::Create(json &js) {
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
  std::shared_ptr<ComponentRenderable> cr =
      std::make_shared<ComponentRenderable>();
  cr->_material_path = json_inst.material_path;
  std::shared_ptr<Material> m = std::static_pointer_cast<Material>(
      cr->FromData(json_inst.material_path, json_inst.uniforms));
  m->renderableObjects.insert(m->renderableObjects.end(), cr);

  glUseProgram(m->program->id);
  CHECK_GL_ERROR();
  std::vector<float> vertices = json_inst.vertices; /*{
       -0.5f, -0.5f, 0.0f, // left
       0.5f,  -0.5f, 0.0f, // right
       0.0f,  0.5f,  0.0f  // top
   };*/

  std::vector<int> indices = json_inst.indices;
  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
  // then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &(vertices[0]),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &(indices[0]),
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  // TODO: Load from json
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO
  // as the vertex attribute's bound vertex buffer object so afterwards we can
  // safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally
  // modify this VAO, but this rarely happens. Modifying other VAOs requires a
  // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
  // VBOs) when it's not directly necessary.
  glBindVertexArray(0);
  cr->vertices = vertices;
  cr->indices = indices;
  cr->vao = VAO;
  cr->vbo = VBO;
  cr->ebo = EBO;
  CHECK_GL_ERROR();
  return cr;
};

void ComponentRenderable::FromJson(json &js) {
  SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                     "Do not call FromJson, call Create instead");
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
  this->_material_path = json_inst.material_path;
  this->_uniforms = json_inst.uniforms;
  this->vertices = json_inst.vertices;
  this->indices = json_inst.indices;
  FromData(json_inst.material_path, json_inst.uniforms);
}

ComponentRenderable::ComponentRenderable(json &js) { FromJson(js); }

ComponentRenderable::ComponentRenderable() {}
