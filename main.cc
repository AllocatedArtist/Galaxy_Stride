#include <algorithm>
#include <raylib.h>

#include "src/Model.h"
#include "src/FlyCamera.h"
#include "src/PhysicsWorld.h"


#include <iostream>

struct ContactInfo {
  Vector3 pos_on_a_;
  Vector3 pos_on_b_;
  Vector3 normal_;
};

bool CustomContactAdd(
  btManifoldPoint& cp, 
  const btCollisionObjectWrapper* colObj0Wrap, 
  int partId0, 
  int index0, 
  const btCollisionObjectWrapper* colObj1Wrap, 
  int partId1, 
  int index1
) {
  const btGhostObject* shape_1 = btGhostObject::upcast(
    colObj1Wrap->getCollisionObject()
  );

  if (shape_1 != nullptr) {
    ContactInfo* contact = 
      reinterpret_cast<ContactInfo*>(shape_1->getUserPointer());

    if (contact != nullptr) {
      contact->pos_on_a_ = conv::GetVec3(cp.m_positionWorldOnA);
      contact->pos_on_b_ = conv::GetVec3(cp.m_positionWorldOnB);
      contact->normal_ = conv::GetVec3(-cp.m_normalWorldOnB);
    }
  }
  return false;
}


int main(void) {
  
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(1600, 1480, "Platformer");

  FlyCamera camera({ 0.0, 2.0, -5.0 }, 0.1, 5.0);
  camera.GetCamera().SetYaw(90.0);

  ModelComponent box({ 0.5, 0.5, 0.5 }, WHITE);
  ModelComponent ground({ 15.0, 0.1, 15.0 }, WHITE);

  Texture white_tex = LoadTexture("assets/prototype/Light/texture_01.png");
  Texture green_tex = LoadTexture("assets/prototype/Green/texture_01.png");

  GenTextureMipmaps(&white_tex);
  GenTextureMipmaps(&green_tex);

  box.SetTexture(green_tex);
  ground.SetTexture(white_tex);

  DisableCursor();

  PhysicsWorld world;

  gContactAddedCallback = CustomContactAdd;

  std::unique_ptr<btCollisionShape> floor_shape = world.CreateBoxShape(
    { 15.0, 0.1, 15.0 }
  );

  std::unique_ptr<btCollisionShape> box_shape = world.CreateBoxShape(
    { 0.5, 0.5, 0.5 }
  );

  std::unique_ptr<btConvexShape> crouched_capsule = 
    std::make_unique<btCapsuleShape>(0.2, 0.25);

  RigidBody floor = world.CreateRigidBody(
    Vector3Zero(), 
    floor_shape,
    QuaternionIdentity(),
    0.0
  );

  RigidBody floor_2 = world.CreateRigidBody(
    { 10.0, -1.7, 0.0 },
    floor_shape,
    QuaternionIdentity(),
    0.0
  );

  /*

  floor.rigid_body_->setCollisionFlags(
    floor.rigid_body_->getCollisionFlags() |
    btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK
  );
  */

  std::vector<RigidBody> cubes;

  for (int x = 0; x < 5; ++x) {
    for (int y = 5; y < 10; ++y) {
      for (int z = 0; z < 5; ++z) {
        cubes.emplace_back(
          world.CreateRigidBody(
            { (float)x * 0.5f - 2.5f, (float)y * 0.5f, (float)z * 0.5f }, 
            box_shape, 
            QuaternionIdentity(), 
            1.0
          )
        ); 
      }
    }
  }

  CharacterController player = world.CreateController(
    0.2,
    0.5,
    0.05,
    camera.GetCamera().GetPosition(),
    QuaternionFromEuler(
      camera.GetCamera().GetYaw(), 
      camera.GetCamera().GetPitch(), 
      0.0
    )
  );

  player.ghost_object_->setCollisionFlags(
    player.ghost_object_->getCollisionFlags() | 
    btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK
  );

  ContactInfo info;
  player.ghost_object_->setUserPointer(&info);

  SetTargetFPS(120);

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

  while (!WindowShouldClose()) { 

    world.Update(1.0 / 60.0);

    camera.LookAround();

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
     
    BeginDrawing(); 
    ClearBackground(BLACK);

    BeginMode3D(camera.GetCamera().GetCamera());
    DrawGrid(20, 1.0);

    for (RigidBody& body : cubes) {
      box.Draw(
        conv::PosFromBody(body),
        { 1.0, 1.0, 1.0 }, 
        conv::RotFromBody(body)
      );
    }

    ground.SetColor(WHITE);

    ground.Draw(
      conv::PosFromBody(floor), 
      { 1.0, 1.0, 1.0 }, 
      conv::RotFromBody(floor)
    );

    ground.SetColor(RED);

    ground.Draw(
      conv::PosFromBody(floor_2), 
      { 1.0, 1.0, 1.0 }, 
      conv::RotFromBody(floor_2)
    );

    EndMode3D();

    DrawFPS(0, 0);
    
    EndDrawing();

    auto cube_to_erase = std::find_if(
      cubes.begin(), 
      cubes.end(), 
      [](RigidBody& body){
        if (conv::PosFromBody(body).y < -100.0) {
          return true;
        }
        return false;
      }
    );

  

    if (cube_to_erase != cubes.cend()) {
      RigidBody& body = *cube_to_erase;   
      world.ReleaseBody(&body);
      cubes.erase(cube_to_erase);
    }

  }

  world.ReleaseController(&player);

  world.ReleaseBody(&floor);
  for (RigidBody& body : cubes) {
    world.ReleaseBody(&body);
  }

  UnloadTexture(white_tex);
  UnloadTexture(green_tex);

  CloseWindow();

  return 0;
}
