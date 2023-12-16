#include "Camera.h"

CameraComponent::CameraComponent() {
  position_ = Vector3Zero();
  up_ = { 0.0, 1.0, 0.0 };

  yaw_ = 0.0;
  pitch_ = 0.0;

  fov_ = 90.0;
   
  CalculateForward();

  right_ = Vector3CrossProduct(forward_, up_);
}


CameraComponent::CameraComponent(
  Vector3 position, 
  float yaw, 
  float pitch, 
  float fov
) : position_(position), fov_(fov), yaw_(yaw), pitch_(pitch) {
  up_ = { 0.0, 1.0, 0.0 };
  
  CalculateForward();

  right_ = Vector3CrossProduct(forward_, up_);
}

void CameraComponent::CalculateForward() {
  forward_.x = cosf(yaw_ * DEG2RAD) * cosf(pitch_ * DEG2RAD);
  forward_.y = sinf(pitch_ * DEG2RAD);
  forward_.z = sinf(yaw_ * DEG2RAD) * cosf(pitch_ * DEG2RAD);
  forward_ = Vector3Normalize(forward_);
}

const Vector3 CameraComponent::GetForward() const {
  return forward_;
}

const Vector3 CameraComponent::GetRight() const {
  return right_;
}

const Vector3 CameraComponent::GetPosition() const {
  return position_;
}

void CameraComponent::SetPosition(Vector3 position) {
  position_ = position;
}

const float CameraComponent::GetYaw() const {
  return yaw_;
}

const float CameraComponent::GetPitch() const {
  return pitch_;
}

void CameraComponent::SetYaw(float yaw) {
  yaw_ = yaw;
  CalculateForward();
  right_ = Vector3CrossProduct(forward_, up_);
}

void CameraComponent::SetPitch(float pitch) {
  pitch_ = pitch;
  CalculateForward();
  right_ = Vector3CrossProduct(forward_, up_);
}

const Camera CameraComponent::GetCamera() const {
  return Camera {
    position_,
    Vector3Add(position_, forward_),
    up_,
    fov_,
    CAMERA_PERSPECTIVE
  };
}

