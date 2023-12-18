#ifndef PLAYER_MOVEMENT_H_
#define PLAYER_MOVEMENT_H_

#include "FlyCamera.h"
#include "PhysicsWorld.h"

class PlayerMovement {
public:
  PlayerMovement() = default;
  void Update(CharacterController& player, FlyCamera& camera);
private:
  Vector3 walk = Vector3Zero();  

  float walk_speed = 2.0;
  float run_speed = 4.0;
  float crouch_speed = 0.3;
  float current_speed = walk_speed;
  
  float stand_jump_height = 5.0;
  float crouched_jump_height = 2.5;
  float current_jump_height = stand_jump_height;

  bool sliding = false;

  float ground_friction = 0.4 / 10.0;
  float slide_friction = 0.35 / 10.0;
  float air_friction = 0.08 / 10.0;

  std::unique_ptr<btConvexShape> crouched_capsule = 
    std::make_unique<btCapsuleShape>(0.2, 0.25);
};



#endif
