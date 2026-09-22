#pragma once
#include "Graphics/Program.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Texture.hpp"
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
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

private:
  //  render data
  unsigned int VAO, VBO, EBO;

  void setupMesh();
};
class Model {
public:
  Model(char *path) { loadModel(path); }
  void Draw(Shader &shader);

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
