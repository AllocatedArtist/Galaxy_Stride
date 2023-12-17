#ifndef MODEL_COMPONENT_H_
#define MODEL_COMPONENT_H_

#include <raylib.h>
#include <raymath.h>

class ModelComponent {
public:
  ModelComponent();
  ModelComponent(Vector3 size, Color color);
  ModelComponent(float radius, Color color);
  ModelComponent(const char* filename, Color color);

  ~ModelComponent();

  void SetColor(Color color);
  const Color GetColor() const;

  void Draw(
    Vector3 position, 
    Vector3 scale = { 1.0, 1.0, 1.0 },
    Quaternion rotation = QuaternionIdentity()
  );

private:
  bool loaded_;
  Model model_;
  Color color_;
};

#endif
