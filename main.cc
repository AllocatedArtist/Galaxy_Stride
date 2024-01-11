#include <fstream>
#include <raylib.h>

#include "src/Model.h"
#include "src/FlyCamera.h"
#include "src/PhysicsWorld.h"
#include "src/PlayerMovement.h"
#include "src/LevelEditor.h"

#include <iostream>

#include <json.hpp>

int main(void) {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
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
    
  while (!WindowShouldClose()) { 

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
     
    BeginDrawing(); 
    ClearBackground(BLACK);
 
    BeginMode3D(camera.GetCamera().GetCamera()); 

    level_editor.PlacePlayer(camera);
    
    DrawGrid(20, 1.0);

    if (!level_editor.IsPlayerSetMode()) {
      level_editor.PlaceObjects(meshes, camera);

      for (LevelMesh& mesh : meshes) {
        level_editor.SelectObject(mesh, camera);
      }
    }

    for (const LevelMesh& mesh : meshes) {
      level_editor.DrawAsset(mesh);
    }
 
    EndMode3D();

    if (saved) {
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

    level_editor.DrawThumbnails();

    DrawFPS(0, 0);
    
    EndDrawing(); 
  }

  CloseWindow();

  return 0;
}
