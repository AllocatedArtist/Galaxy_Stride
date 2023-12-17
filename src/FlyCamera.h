#ifndef FLY_CAMERA_H_
#define FLY_CAMERA_H_

#include "Camera.h"

class FlyCamera {
public:
  FlyCamera() = default;
  FlyCamera(Vector3 position, float sensitivity, float speed);

  CameraComponent& GetCamera();

  void LookAround();
  void Move();
private:
  CameraComponent camera_;

  float speed_;
  float sensitivity_;
};

#endif
