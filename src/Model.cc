#include "Model.h"

ModelComponent::ModelComponent() {
  loaded_ = false;
  color_ = PURPLE;
}

ModelComponent::ModelComponent(Vector3 size, Color color) {
  loaded_ = true;
  model_ = LoadModelFromMesh(GenMeshCube(size.x, size.y, size.z));
  color_ = color;
}

ModelComponent::ModelComponent(float radius, Color color) {
  loaded_ = true;
  model_ = LoadModelFromMesh(GenMeshSphere(radius, 8, 8));
  color_ = color;
}

ModelComponent::ModelComponent(const char* filename, Color color) {
  loaded_ = true;
  model_ = LoadModel(filename);
  color_ = color;
}

ModelComponent::~ModelComponent() {
  if (loaded_) {
    loaded_ = false;
    UnloadModel(model_);
  }
}

void ModelComponent::SetColor(Color color) {
  color_ = color;
}

const Color ModelComponent::GetColor() const {
  return color_;
}

void ModelComponent::Draw(
  Vector3 position, 
  Vector3 scale,
  Quaternion rotation
) {
  float angle = 0.0;
  Vector3 rotation_axis = Vector3Zero();

  QuaternionToAxisAngle(rotation, &rotation_axis, &angle);
  
  DrawModelEx(model_, position, rotation_axis, angle, scale, color_);
}






