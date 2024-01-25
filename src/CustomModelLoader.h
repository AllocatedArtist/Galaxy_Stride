#ifndef CUSTOM_MODEL_LOADER_H_
#define CUSTOM_MODEL_LOADER_H_

#include <raylib.h>
#include <tiny_gltf.h>

#include <vector>

#include "FlyCamera.h"

struct CustomMesh {
  std::vector<unsigned int> vao_;
  std::vector<unsigned int> vbo_;
  std::vector<unsigned int> ebo_;
  std::vector<Vector3> base_color_;
  Matrix transform_;
  std::vector<int> index_count_;
  std::vector<Vector3> mins_;
  std::vector<Vector3> maxs_;
};

class CustomModel { 
public:
  CustomModel() = default;
  void LoadFromMemory(const char* filename);
  void Unload();
  void Draw(
    FlyCamera& camera, 
    Shader shader,
    int model_matrix_loc,
    int view_projection_loc,
    int base_color_loc,
    Vector3 position = { 0.0, 0.0, 0.0 }, 
    Quaternion rotation = QuaternionIdentity(),
    Vector3 scale = { 1.0, 1.0, 1.0 }
  );
  const BoundingBox GetBoundingBox() const;
private:
  std::vector<CustomMesh> meshes_;
private:
  void ProcessNodes(const tinygltf::Node& node, const tinygltf::Model& model);
  void ProcessMesh(
    const tinygltf::Mesh& mesh, 
    const tinygltf::Model& model,
    const Matrix& transform
  );
};



#endif
