#include <fstream>
#include <raylib.h>

#include "src/Model.h"
#include "src/FlyCamera.h"
#include "src/PhysicsWorld.h"
#include "src/PlayerMovement.h"
#include "src/LevelEditor.h"

#include <iostream>

int main(void) {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(1600, 1480, "Platformer");

  FlyCamera camera({ 0.0, 2.0, -5.0 }, 0.1, 5.0);
  camera.GetCamera().SetYaw(90.0);

  SetTargetFPS(120); 

  LevelEditor level_editor;
  level_editor.UpdateThumbnails();

  std::vector<LevelMesh> meshes;
  SetExitKey(KEY_NULL);

  bool saved = false;
  float save_timer = 0.f;
  float save_delay = 1.0f;

  bool is_play_mode = false;
  bool create_collision = true;

  PhysicsWorld physics;
  std::vector<RigidBody> mesh_bodies;
  std::vector<std::unique_ptr<btCollisionShape>> mesh_colliders;

  CharacterController player;
  PlayerMovement player_movement;
    
  while (!WindowShouldClose()) { 

    if (IsKeyPressed(KEY_F1)) {
      is_play_mode = !is_play_mode;
      if (!is_play_mode) {
        create_collision = true;
        mesh_colliders.clear();
        for (RigidBody& body : mesh_bodies) {
          physics.ReleaseBody(&body);
        }
        mesh_bodies.clear();
        physics.ReleaseController(&player);

        EnableCursor();
        camera.GetCamera().SetPosition({ 0.f, 0.f, 0.f });
      } else {
        DisableCursor();
      }
    }

    if (is_play_mode && create_collision) {
      create_collision = false;
      for (const LevelMesh& mesh : meshes) {
        ModelComponent& model = level_editor.GetAsset(mesh.index_).model_;
        BoundingBox bounds = model.GetBoundingBox();

        Vector3 size = Vector3Subtract(bounds.max, bounds.min);
        mesh_colliders.emplace_back(physics.CreateBoxShape(size));

        std::unique_ptr<btCollisionShape>& box = mesh_colliders.back();
        
        mesh_bodies.emplace_back(physics.CreateRigidBody(
          mesh.pos_,
          box,
          mesh.rotation_,
          0.f
        ));
      }

      player = physics.CreateController(
        0.25, 
        1.5,
        0.1, 
        level_editor.GetPlayerPosition()
      );

      camera.GetCamera().SetPitch(0.f);
      camera.GetCamera().SetYaw(level_editor.GetPlayerYaw());
    }


    if (is_play_mode) {
      physics.Update(1.0 / 60.0);
      camera.LookAround();
      player_movement.Update(player, camera);

      if (camera.GetCamera().GetPosition().y <= -10.f) {
        btTransform transform;
        transform.setIdentity();

        Vector3 player_pos = level_editor.GetPlayerPosition();
        transform.setOrigin(btVector3(player_pos.x, player_pos.y, player_pos.z));

        player.ghost_object_->setWorldTransform(transform);

        camera.GetCamera().SetPitch(0.f);
        camera.GetCamera().SetYaw(level_editor.GetPlayerYaw());
      }
    }

    if (!is_play_mode) {
      level_editor.UpdateCamera(camera); 

      level_editor.Save(meshes);

      if (
        IsKeyDown(KEY_LEFT_CONTROL) && 
        IsKeyPressed(KEY_S) && 
        saved == false
      ) {
        saved = true; 
      }

      level_editor.Load(meshes);
    }
     
    BeginDrawing(); 
    ClearBackground(BLACK);

    BeginMode3D(camera.GetCamera().GetCamera()); 


    if (!is_play_mode) {
      level_editor.PlacePlayer(camera);
    
      DrawGrid(20, 1.0);

      if (!level_editor.IsPlayerSetMode()) {
        level_editor.PlaceObjects(meshes, camera);

        for (LevelMesh& mesh : meshes) {
          level_editor.SelectObject(mesh, camera);
        }
      }
    }

    for (const LevelMesh& mesh : meshes) {
      level_editor.DrawAsset(mesh);
    }
 
    EndMode3D();

    if (saved && !is_play_mode) {
      save_timer += GetFrameTime();
      if (save_timer >= save_delay) {
        save_timer = 0.f;
        saved = false;
      }

      const char* filename = level_editor.GetCurrentFileSaveName().c_str();
      DrawText(
        TextFormat("Saved to: %s", filename),
        500, 100, 24, RED
      );
    }

    if (!is_play_mode) {
      level_editor.DrawThumbnails();
    }

    if (is_play_mode) {
      DrawRectangle(10, 100, 500, 50, GRAY);
      DrawRectangle(10, 100, player_movement.GetStamina() * 10.0, 50, RED);
    }


    DrawFPS(0, 0);
    
    EndDrawing(); 
  }

  CloseWindow();

  return 0;
}
