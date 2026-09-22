#pragma once
#include "Graphics/Program.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Texture.hpp"
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine {
namespace Graphics {
struct MVertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};
struct MTexture {
  std::shared_ptr<Texture> texture;
  std::string type;
};
class Mesh {
public:
  // mesh data
  std::vector<MVertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<MTexture> textures;

  Mesh(std::vector<MVertex> vertices, std::vector<unsigned int> indices,
       std::vector<MTexture> textures);
  void Draw(Engine::Graphics::Program &program);
  void DrawInstanced(Engine::Graphics::Program &program, unsigned int instanceCount);

  unsigned int GetVAO() const { return VAO; }
  unsigned int GetIndexCount() const { return static_cast<unsigned int>(indices.size()); }

private:
  //  render data
  unsigned int VAO = 0, VBO = 0, EBO = 0;

  void setupMesh();
};
class Model {
public:
  static std::shared_ptr<Model> Create(const std::string &path);
  static std::unordered_map<std::string, std::shared_ptr<Model>> models;

  Model(const std::string &path);
  Model() = default;
  void Draw(Engine::Graphics::Program &program);
  void DrawInstanced(Engine::Graphics::Program &program, unsigned int instanceCount);

  const std::vector<Mesh> &GetMeshes() const { return meshes; }
  std::string path;

private:
  // model data
  std::vector<Mesh> meshes;
  std::string directory;

  void loadModel(std::string path);
  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  std::vector<MTexture> loadMaterialTextures(aiMaterial *mat,
                                             aiTextureType type,
                                             std::string typeName);
};
} // namespace Graphics
} // namespace Engine

