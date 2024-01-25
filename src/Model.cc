#include "Model.h"
#include <utility>


ModelComponent::ModelComponent() {
  loaded_ = false;
  color_ = PURPLE;
  use_custom_ = false;
}

ModelComponent::ModelComponent(ModelComponent&& other) {
  model_ = std::move(other.model_); 
  custom_model_ = std::move(other.custom_model_);

  color_ = other.color_;
  use_custom_ = other.use_custom_;

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

ModelComponent::ModelComponent(const char* filename, Color color, bool memory) {
  if (!memory) {
    model_ = LoadModel(filename);
    if (model_.meshCount > 0) {
      loaded_ = true;
    }
  } else {
    use_custom_ = true;
    custom_model_.LoadFromMemory(filename);
    loaded_ = true;
  }
  color_ = color;
}

ModelComponent::~ModelComponent() {
  if (loaded_) {
    loaded_ = false;
    if (!use_custom_) {
      UnloadModel(model_);
    } else {
      custom_model_.Unload();
    }
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
  
  if (!use_custom_) {
    DrawModelEx(model_, position, rotation_axis, angle * RAD2DEG, scale, color_);
  }
}

void ModelComponent::SetTexture(Texture texture) {
  if (loaded_) {
    SetMaterialTexture(&model_.materials[0], MATERIAL_MAP_ALBEDO, texture);
  }
}

const BoundingBox ModelComponent::GetBoundingBox() const {
  if (!use_custom_) {
    return GetModelBoundingBox(model_);
  }

  BoundingBox custom_box = custom_model_.GetBoundingBox();

  return custom_box;
}

void ModelComponent::DrawCustomModel(    
  FlyCamera& camera, 
  Shader shader,
  int model_matrix_loc,
  int view_projection_loc,
  int base_color_loc,
  Vector3 position,
  Quaternion rotation,
  Vector3 scale
) {
  custom_model_.Draw(
    camera, 
    shader, 
    model_matrix_loc, 
    view_projection_loc, 
    base_color_loc, 
    position, 
    rotation, 
    scale
  );
}

