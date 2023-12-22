#include "Model.h"
#include <iostream>

ModelComponent::ModelComponent() {
  loaded_ = false;
  color_ = PURPLE;
}

ModelComponent::ModelComponent(ModelComponent&& other) {
  model_ = std::move(other.model_); 
  color_ = other.color_;

  loaded_ = other.loaded_;
  other.loaded_ = false;
}

ModelComponent::ModelComponent(Vector3 size, Color color) {
  model_ = LoadModelFromMesh(GenMeshCube(size.x, size.y, size.z));
  loaded_ = true;
  color_ = color;
}

ModelComponent::ModelComponent(float radius, Color color) {
  model_ = LoadModelFromMesh(GenMeshSphere(radius, 8, 8));
  loaded_ = true;
  color_ = color;
}

ModelComponent::ModelComponent(const char* filename, Color color) {
  model_ = LoadModel(filename);
  if (model_.meshCount > 0) {
    loaded_ = true;
  }
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
  
  DrawModelEx(model_, position, rotation_axis, angle * RAD2DEG, scale, color_);
}

void ModelComponent::SetTexture(Texture texture) {
  if (loaded_) {
    SetMaterialTexture(&model_.materials[0], MATERIAL_MAP_ALBEDO, texture);
  }
}

const BoundingBox ModelComponent::GetBoundingBox() const {
  return GetModelBoundingBox(model_);
}






