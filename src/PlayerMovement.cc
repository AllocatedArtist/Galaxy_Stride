#include "PlayerMovement.h"

#include <iostream>

PlayerMovement::PlayerMovement() {
  walk_ = Vector3Zero();  

  walk_speed_ = 1.5;
  run_speed_ = 4.0;
  crouch_speed_ = 0.3;
  current_speed_ = walk_speed_;

  sprint_ = false;
  
  stand_jump_height_ = 5.0;
  crouched_jump_height_ = 2.5;
  current_jump_height_ = stand_jump_height_;

  sliding_ = false;

  ground_friction_ = 0.4 / 10.0;
  slide_friction_ = 0.35 / 10.0;
  air_friction_ = 0.08 / 10.0;

  max_stamina_ = 50.0;
  stamina_ = max_stamina_;

  sprint_stamina_drain_ = 2.5;
  slide_stamina_drain_ = 5.0;
  slide_jump_stamina_drain_ = 4.0;

  crouched_capsule_ = 
    std::make_unique<btCapsuleShape>(0.25, 0.75);
}


void PlayerMovement::Update(CharacterController& player, FlyCamera& camera) {
    Vector3 forward = camera.GetCamera().GetForward();
    forward.y = 0.0;
    Vector3 right = camera.GetCamera().GetRight(); 
  
    Vector3 move_dir = Vector3Zero();

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

    if (IsKeyPressed(KEY_LEFT_SHIFT) && stamina_ > 1.0) {
      sprint_ = true;   
    } else if (IsKeyUp(KEY_LEFT_SHIFT) || stamina_ <= 1.0) {
      sprint_ = false;
    }

    if (sprint_ && player.controller_->onGround()) { 
      current_speed_ = Lerp(current_speed_, run_speed_, 0.2 / 10.0); 
    } else {
      if (player.controller_->onGround()) { 
        current_speed_ = Lerp(current_speed_, walk_speed_, 0.2 / 10.0);
      }
    }

    if (sprint_) {
      stamina_ = Lerp(stamina_, stamina_ - sprint_stamina_drain_, 0.1);
    }

    float speed_magnitude = Vector3Length(walk_);

    // bunch of hard values for comparing to speed magnitude. need to change
    // if speed is changed. bunch of conditions for certain movement effects

    float speed_slide_threshold = 0.018f;
    float slide_stop_threshold = 0.002f;
    float slide_jump_threshold = 0.01f;
    float stamina_regen_threshold = 0.013f;

    float camera_sprint_zoom_threshold = 0.018f;
    float camera_walk_zoom_threshold = 0.014f;

    if (IsKeyDown(KEY_LEFT_CONTROL) && player.controller_->onGround()) {
      current_jump_height_ = crouched_jump_height_;
      current_speed_ = Lerp(current_speed_, crouch_speed_, 0.5 / 10.0);
      player.ghost_object_->setCollisionShape(crouched_capsule_.get()); 

      if (!Vector3Equals(move_dir, Vector3Zero())) {
        btVector3 bt_forward = btVector3(move_dir.x, 0.0, move_dir.z);

        if (speed_magnitude > speed_slide_threshold 
          && !sliding_ && stamina_ > 1.0) {
          stamina_ -= slide_stamina_drain_; 
          player.controller_->applyImpulse(bt_forward * 3.f);
          sliding_ = true;
        }
      }
    } else if (IsKeyUp(KEY_LEFT_CONTROL)) {
      current_jump_height_ = stand_jump_height_;
      player.ghost_object_->setCollisionShape(player.convex_.get());
    }

    if (IsKeyUp(KEY_LEFT_CONTROL) && speed_magnitude < slide_stop_threshold) {
      sliding_ = false;
    }

    if (IsKeyPressed(KEY_SPACE) && player.controller_->onGround()) {
      if (sliding_ && speed_magnitude > slide_jump_threshold) { 
        stamina_ -= slide_jump_stamina_drain_;
        Vector3 forward_force = Vector3Scale(forward, 5.0);
        player.controller_->applyImpulse(
          btVector3(forward_force.x, stand_jump_height_ * 1.2, forward_force.z)
        );
      } else {
        player.controller_->jump(btVector3(0.0, current_jump_height_, 0.0)); 
      }
    }

    stamina_ = Clamp(stamina_, 0.0, max_stamina_);

    if (stamina_ <= 1.0) {
      current_speed_ = walk_speed_; 
    }

    if (speed_magnitude < stamina_regen_threshold && !sliding_) {
      stamina_ = Lerp(stamina_, max_stamina_ + 1, 1.0 * GetFrameTime());
    }  

    if (!Vector3Equals(move_dir, Vector3Zero()) && !sliding_) {
      move_dir = Vector3Normalize(move_dir);
      walk_ = Vector3Scale(move_dir, current_speed_ * GetFrameTime());
    } else {
      if (player.controller_->onGround()) {
        if (!sliding_) {
          walk_ = Vector3Lerp(walk_, Vector3Zero(), ground_friction_);
        } else {
          walk_ = Vector3Lerp(walk_, Vector3Zero(), slide_friction_);
        }
      } else {
        walk_ = Vector3Lerp(walk_, Vector3Zero(), air_friction_);
      }
    } 

    if (speed_magnitude > camera_sprint_zoom_threshold) {
      float fov = camera.GetCamera().GetFOV();
      fov = Lerp(fov, 100.f, 0.8f / 10.0);
      camera.GetCamera().SetFOV(fov);
    } else if (speed_magnitude < camera_walk_zoom_threshold) {
      float fov = camera.GetCamera().GetFOV();
      fov = Lerp(fov, 90.f, 0.8f / 10.0);
      camera.GetCamera().SetFOV(fov);
    }


    player.controller_->setFallSpeed(8.f);

    player.controller_->setWalkDirection(
      btVector3(
        walk_.x,
        0.0f, 
        walk_.z
      )
    ); 

    btCapsuleShape* capsule = 
      (btCapsuleShape*)player.ghost_object_->getCollisionShape();

    camera.GetCamera().SetPosition(
      Vector3Add(
        conv::PosFromController(player),
        { 0.f, capsule->getHalfHeight(), 0.f }
      )
    );
}

const float PlayerMovement::GetStamina() const {
  return stamina_;
}

void DrawStamina(const PlayerMovement& player_movement) {
  DrawRectangle(20, 50, 500, 30, GRAY);
  DrawRectangle(20, 50, (int)player_movement.GetStamina() * 10, 30, BLUE);
}

