#include <algorithm>
#include <raylib.h>

#include "src/Model.h"
#include "src/FlyCamera.h"
#include "src/PhysicsWorld.h"
#include "src/PlayerMovement.h"

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

  SetTextureWrap(white_tex, TEXTURE_WRAP_CLAMP);
  SetTextureWrap(green_tex, TEXTURE_WRAP_CLAMP);

  SetTextureFilter(white_tex, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(green_tex, TEXTURE_FILTER_BILINEAR);


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

  PlayerMovement player_movement;

  SetTargetFPS(120);

  while (!WindowShouldClose()) { 

    world.Update(1.0 / 60.0);

    camera.LookAround();
    
    player_movement.Update(player, camera);
    
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


    ground.Draw(
      conv::PosFromBody(floor), 
      { 1.0, 1.0, 1.0 }, 
      conv::RotFromBody(floor)
    );

    ground.Draw(
      conv::PosFromBody(floor_2), 
      { 1.0, 1.0, 1.0 }, 
      conv::RotFromBody(floor_2)
    );

    EndMode3D();

    DrawRectangle(20, 50, 500, 30, GRAY);
    DrawRectangle(20, 50, (int)player_movement.GetStamina() * 10, 30, BLUE);
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
