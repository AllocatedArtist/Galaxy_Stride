#include <raylib.h>

#include "src/Model.h"
#include "src/FlyCamera.h"
#include "src/PhysicsWorld.h"

#include <iostream>

int main(void) {
  
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(1600, 1480, "Platformer");

  FlyCamera camera({ 0.0, 2.0, -4.0 }, 0.1, 5.0);
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

  std::vector<RigidBody> cubes;

  for (int x = 0; x < 5; ++x) {
    for (int y = 20; y < 40; ++y) {
      for (int z = 0; z < 5; ++z) {
        cubes.emplace_back(
          world.CreateRigidBody(
            { (float)x * 0.2f, (float)y * 0.5f, (float)z * 0.2f }, 
            box_shape, 
            QuaternionIdentity(), 
            1.0
          )
        ); 
      }
    }
  }


   
  while (!WindowShouldClose()) { 

    world.Update(1.0 / 60.0);

    camera.LookAround();
    camera.Move();

    float scroll = GetMouseWheelMove();

    float fov = camera.GetCamera().GetFOV();
    
    camera.GetCamera().SetFOV(fov - scroll);
    if (fov > 130.0)
      camera.GetCamera().SetFOV(130.0);
    if (fov < 25.0)
      camera.GetCamera().SetFOV(25.0);

    BeginDrawing(); 
    ClearBackground(BLACK);

    BeginMode3D(camera.GetCamera().GetCamera());
    DrawGrid(20, 1.0);

    for (RigidBody& body : cubes) {
      box.Draw(body.GetPosition(), { 1.0, 1.0, 1.0 }, body.GetRotation());
    }
    ground.Draw(floor.GetPosition(), { 1.0, 1.0, 1.0 }, floor.GetRotation());

    EndMode3D();
    
    EndDrawing();
  }

  world.ReleaseBody(&floor);
  for (RigidBody& body : cubes) {
    world.ReleaseBody(&body);
  }

  UnloadTexture(white_tex);
  UnloadTexture(green_tex);

  CloseWindow();

  return 0;
}
