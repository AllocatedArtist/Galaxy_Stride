#include <algorithm>
#include <raylib.h>

#include "src/Model.h"
#include "src/FlyCamera.h"
#include "src/PhysicsWorld.h"
#include "src/PlayerMovement.h"
#include "src/LevelEditor.h"

#include <iostream>

int main(void) {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(1600, 1480, "Platformer");

  FlyCamera camera({ 0.0, 2.0, -5.0 }, 0.1, 5.0);
  camera.GetCamera().SetYaw(90.0);

  SetTargetFPS(120); 

  LevelEditor level_editor;
  level_editor.UpdateThumbnails();

  std::vector<LevelMesh> meshes;

  while (!WindowShouldClose()) { 

    level_editor.UpdateCamera(camera);
           
    BeginDrawing(); 
    ClearBackground(BLACK);
 
    BeginMode3D(camera.GetCamera().GetCamera()); 
    
    level_editor.PlaceObjects(meshes, camera);
    DrawGrid(20, 1.0);

    for (const LevelMesh& mesh : meshes) {
      level_editor.DrawAsset(mesh);
    }
 
    EndMode3D();

    level_editor.DrawThumbnails();

    DrawFPS(0, 0);
    
    EndDrawing(); 
  }

  CloseWindow();

  return 0;
}
