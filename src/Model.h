#ifndef MODEL_COMPONENT_H_
#define MODEL_COMPONENT_H_

#include <raylib.h>
#include <raymath.h>

#include "CustomModelLoader.h"

class ModelComponent {
public:
  ModelComponent();
  ModelComponent(ModelComponent&& other);

  ModelComponent(Vector3 size, Color color);
  ModelComponent(float radius, Color color);
  ModelComponent(const char* filename, Color color, bool move);

  const BoundingBox GetBoundingBox() const;

  void SetTexture(Texture texture);

  ~ModelComponent();

  void SetColor(Color color);
  const Color GetColor() const;

  void Draw(
    Vector3 position, 
    Vector3 scale = { 1.0, 1.0, 1.0 },
    Quaternion rotation = QuaternionIdentity()
  );

  void DrawCustomModel(    
    FlyCamera& camera, 
    Shader shader,
    int model_matrix_loc,
    int view_projection_loc,
    int base_color_loc,
    Vector3 position = { 0.0, 0.0, 0.0 }, 
    Quaternion rotation = QuaternionIdentity(),
    Vector3 scale = { 1.0, 1.0, 1.0 }
  );

private:
  bool loaded_;
  Model model_;
  CustomModel custom_model_;
  bool use_custom_;
  Color color_;
};

#endif
