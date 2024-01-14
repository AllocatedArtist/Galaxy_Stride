#ifndef PLAYER_MOVEMENT_H_
#define PLAYER_MOVEMENT_H_

#include "FlyCamera.h"
#include "PhysicsWorld.h"

class PlayerMovement {
public:
  PlayerMovement();
  void Update(CharacterController& player, FlyCamera& camera);
  void ResetStamina();
  const float GetStamina() const;
private:
  Vector3 walk_;

  float walk_speed_;
  float run_speed_;
  float crouch_speed_;
  float current_speed_;

  bool sprint_;
  
  float stand_jump_height_;
  float crouched_jump_height_;
  float current_jump_height_;

  bool sliding_;

  float ground_friction_;
  float slide_friction_;
  float air_friction_;

  float max_stamina_;
  float stamina_;

  float sprint_stamina_drain_;
  float slide_stamina_drain_;
  float slide_jump_stamina_drain_;

  std::unique_ptr<btConvexShape> crouched_capsule_;
};

void DrawStamina(const PlayerMovement& player_movement);

#endif
