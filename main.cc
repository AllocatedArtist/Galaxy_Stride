#include <raylib.h>

#include "src/Model.h"
#include "src/FlyCamera.h"
#include "src/PhysicsWorld.h"
#include "src/PlayerMovement.h"
#include "src/LevelEditor.h"

#include <algorithm>
#include <iostream>

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1600, 1480, "Platformer");

  FlyCamera camera({ 0.0, 2.0, -5.0 }, 0.1, 5.0);
  camera.GetCamera().SetYaw(90.0);

  SetTargetFPS(120); 

  LevelEditor level_editor;
  level_editor.UpdateThumbnails();

  std::vector<LevelMesh> meshes;
  std::vector<LevelCoin> coins;
  SetExitKey(KEY_NULL);

  bool saved = false;
  float save_timer = 0.f;
  float save_delay = 1.0f;

  bool is_play_mode = false;
  bool create_collision = true;

  PhysicsWorld physics;
  std::vector<RigidBody> mesh_bodies;
  std::vector<std::unique_ptr<btCollisionShape>> mesh_colliders;

  std::vector<RigidBody> coin_bodies;
  std::vector<std::unique_ptr<btCollisionShape>> coin_colliders;

  CharacterController player;
  PlayerMovement player_movement;

  int score = 0;
    
  while (!WindowShouldClose()) { 

    if (IsKeyPressed(KEY_F1)) {
      is_play_mode = !is_play_mode;
      if (!is_play_mode) {
        create_collision = true;

        mesh_colliders.clear();
        coin_colliders.clear();

        for (RigidBody& body : mesh_bodies) {
          physics.ReleaseBody(&body);
        }

        for (RigidBody& body : coin_bodies) {
          physics.ReleaseBody(&body);
        }

        for (LevelCoin& coin : coins) {
          coin.collected_ = false;
        }


        mesh_bodies.clear();
        coin_bodies.clear();

        physics.ReleaseController(&player);

        EnableCursor();
        camera.GetCamera().SetPosition({ 0.f, 0.f, 0.f });
      } else {
        player_movement.ResetStamina();
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

      for (LevelCoin& coin : coins) {
        ModelComponent& model = level_editor.GetAsset(coin.index_).model_;
        BoundingBox bounds = model.GetBoundingBox();

        Vector3 size = Vector3Subtract(bounds.max, bounds.min);
        coin_colliders.emplace_back(physics.CreateBoxShape(size));

        std::unique_ptr<btCollisionShape>& box = coin_colliders.back();
        
        coin_bodies.emplace_back(physics.CreateRigidBody(
          coin.pos_,
          box,
          coin.rotation_,
          0.f
        ));

        RigidBody& body = coin_bodies.back();
        body.rigid_body_->setCollisionFlags(
          btCollisionObject::CF_NO_CONTACT_RESPONSE |
          btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK
        );
        body.rigid_body_->setUserIndex(PhysicsLayer::kCoinLayer);
        body.rigid_body_->setUserPointer(&coins[coin_bodies.size() - 1]);
      }

      player = physics.CreateController(
        0.25, 
        1.5,
        0.1, 
        level_editor.GetPlayerPosition()
      );

      player.ghost_object_->setCollisionFlags(
        player.ghost_object_->getCollisionFlags() |
        btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK
      );
      player.ghost_object_->setUserIndex(PhysicsLayer::kPlayerLayer);

      camera.GetCamera().SetPitch(0.f);
      camera.GetCamera().SetYaw(level_editor.GetPlayerYaw());
    }


    if (is_play_mode) {
      physics.Update(1.0 / 60.0);
      camera.LookAround();
      player_movement.Update(player, camera);

      if (camera.GetCamera().GetPosition().y <= -10.f) {
        player_movement.ResetStamina();

        btTransform transform;
        transform.setIdentity();

        Vector3 player_pos = level_editor.GetPlayerPosition();
        transform.setOrigin(btVector3(player_pos.x, player_pos.y, player_pos.z));

        player.ghost_object_->setWorldTransform(transform);

        camera.GetCamera().SetPitch(0.f);
        camera.GetCamera().SetYaw(level_editor.GetPlayerYaw());

        for (LevelCoin& coin : coins) {
          coin.collected_ = false;
        }
      }   

      score = std::count_if(
        coins.cbegin(), 
        coins.cend(), 
        [](const LevelCoin& coin){
          return coin.collected_;
        }
      );
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
        level_editor.PlaceObjects(coins, meshes, camera);

        for (LevelMesh& mesh : meshes) {
          level_editor.SelectObject(mesh, camera);
        }
      }
    }

    for (const LevelCoin& coin : coins) {
      if (!coin.collected_) {
        level_editor.DrawCoins(coin);
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
      DrawStamina(player_movement);
      DrawText(TextFormat("SCORE: %d", score), 20, 90, 32, WHITE);
    }

    if (level_editor.IsCoinMode()) {
      DrawText("COIN MODE ON", 400, 200, 32, RED);
    }


    DrawFPS(0, 0);
    
    EndDrawing(); 
  }

  mesh_colliders.clear();
  coin_colliders.clear();

  for (RigidBody& body : mesh_bodies) {
    physics.ReleaseBody(&body);
  }

  for (RigidBody& body : coin_bodies) {
    physics.ReleaseBody(&body);
  }

  mesh_bodies.clear();
  coin_bodies.clear();

  physics.ReleaseController(&player);
 

  CloseWindow();

  return 0;
}
