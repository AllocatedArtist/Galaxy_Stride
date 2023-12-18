#include "PlayerMovement.h"

void PlayerMovement::Update(CharacterController& player, FlyCamera& camera) {
    Vector3 forward = camera.GetCamera().GetForward();
    forward.y = 0.0;
    Vector3 right = camera.GetCamera().GetRight();

    if (IsKeyPressed(KEY_SPACE) && player.controller_->onGround()) {
      if (sliding && Vector3Length(walk) > 0.01) {
        
        Vector3 forward_force = Vector3Scale(forward, 5.0);
        player.controller_->applyImpulse(
          btVector3(forward_force.x, stand_jump_height * 1.2, forward_force.z)
        );
      } else {
        player.controller_->jump(btVector3(0.0, current_jump_height, 0.0)); 
      }
    }
  
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

    if (IsKeyDown(KEY_LEFT_SHIFT) && player.controller_->onGround()) { 
      current_speed = Lerp(current_speed, run_speed, 0.2 / 10.0); 
    } else {
      if (player.controller_->onGround()) { 
        current_speed = Lerp(current_speed, walk_speed, 0.2 / 10.0);
      }
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && player.controller_->onGround()) {
      current_jump_height = crouched_jump_height;
      current_speed = Lerp(current_speed, crouch_speed, 0.5 / 10.0);
      player.ghost_object_->setCollisionShape(crouched_capsule.get());

      if (!Vector3Equals(move_dir, Vector3Zero())) {
        btVector3 bt_forward = btVector3(move_dir.x, 0.0, move_dir.z);
        float mag = Vector3Length(walk);

        if (mag > 0.018f && !sliding) {
          player.controller_->applyImpulse(bt_forward * 3.f);
          sliding = true;
        }
      }
    } else {
      current_jump_height = stand_jump_height;
      player.ghost_object_->setCollisionShape(player.convex_.get());
    }

    if (IsKeyUp(KEY_LEFT_CONTROL) && Vector3Length(walk) < 0.002) {
      sliding = false;
    }

 
    if (!Vector3Equals(move_dir, Vector3Zero()) && !sliding) {
      move_dir = Vector3Normalize(move_dir);
      walk = Vector3Scale(move_dir, current_speed * GetFrameTime());
    } else {
      if (player.controller_->onGround()) {
        if (!sliding) {
          walk = Vector3Lerp(walk, Vector3Zero(), ground_friction);
        } else {
          walk = Vector3Lerp(walk, Vector3Zero(), slide_friction);
        }
      } else {
        walk = Vector3Lerp(walk, Vector3Zero(), air_friction);
      }
    } 

    if (Vector3Length(walk) > 0.018f) {
      float fov = camera.GetCamera().GetFOV();
      fov = Lerp(fov, 100.f, 0.8f / 10.0);
      camera.GetCamera().SetFOV(fov);
    } else if (Vector3Length(walk) < 0.014) {
      float fov = camera.GetCamera().GetFOV();
      fov = Lerp(fov, 90.f, 0.8f / 10.0);
      camera.GetCamera().SetFOV(fov);
    }


    player.controller_->setFallSpeed(8.f);

    player.controller_->setWalkDirection(
      btVector3(
        walk.x,
        0.0f, 
        walk.z
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
