#include "FlyCamera.h"

#include <cmath>
#include <raylib.h>


FlyCamera::FlyCamera(Vector3 position, float sensitivity, float speed) {
  camera_ = CameraComponent(position, -90.0, 0.0, 90.0);
  speed_ = speed;
  sensitivity_ = sensitivity; 
}


CameraComponent& FlyCamera::GetCamera() {
  return camera_;
}

void FlyCamera::LookAround() {
  float mouse_delta_x = GetMouseDelta().x; 
  float mouse_delta_y = GetMouseDelta().y; 

  float yaw = camera_.GetYaw();
  float pitch = camera_.GetPitch();

  camera_.SetYaw(fmod(yaw + mouse_delta_x * sensitivity_, 360.0));
  camera_.SetPitch(pitch - mouse_delta_y * sensitivity_);

  if (camera_.GetPitch() < -89.0) {
    camera_.SetPitch(-89.0);
  } else if (camera_.GetPitch() > 89.0) {
    camera_.SetPitch(89.0);
  }
}

void FlyCamera::Move() {
  Vector3 move_dir = Vector3Zero();

  Vector3 forward = camera_.GetForward();
  Vector3 right = camera_.GetRight();

  if (IsKeyDown(KEY_W)) {
    move_dir = Vector3Add(move_dir, forward);
  }
  if (IsKeyDown(KEY_S)) {
    move_dir = Vector3Subtract(move_dir, forward);
  }
  if (IsKeyDown(KEY_A)) {
    move_dir = Vector3Subtract(move_dir, right);
  }
  if (IsKeyDown(KEY_D)) {
    move_dir = Vector3Add(move_dir, right);
  }
  
  if (!Vector3Equals(move_dir, Vector3Zero())) {
    move_dir = Vector3Normalize(move_dir);
    move_dir = Vector3Scale(move_dir, speed_ * GetFrameTime());

    camera_.SetPosition(Vector3Add(camera_.GetPosition(), move_dir));
  }

}


