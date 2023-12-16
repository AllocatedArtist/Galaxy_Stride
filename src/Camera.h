#ifndef CAMERA_H_
#define CAMERA_H_

#include <raylib.h>
#include <raymath.h>

class CameraComponent {
public:
  CameraComponent();
  CameraComponent(Vector3 position, float yaw, float pitch, float fov);
  
  const float GetYaw() const;
  const float GetPitch() const;

  void SetYaw(float yaw);
  void SetPitch(float pitch);

  const Vector3 GetForward() const;
  const Vector3 GetRight() const;

  const Vector3 GetPosition() const;
  void SetPosition(Vector3 position);

  const Camera GetCamera() const;
private:
  void CalculateForward();
private:
  Vector3 position_;

  Vector3 up_;
  Vector3 forward_;
  Vector3 right_; //normalized so long as up vec is normalized

  float fov_;

  float yaw_;
  float pitch_;
};

#endif
