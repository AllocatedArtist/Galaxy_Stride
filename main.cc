#include <raylib.h>

#include "src/Camera.h"

int main(void) {
  
  InitWindow(1600, 1480, "Platformer");

  CameraComponent camera;
  camera.SetYaw(-90.0);
  camera.SetPosition({ 0.0, 2.0, 8.0 });

  while (!WindowShouldClose()) {
    BeginDrawing(); 
    ClearBackground(BLACK);

    BeginMode3D(camera.GetCamera());
    DrawGrid(20, 1.0);
    DrawCube(Vector3Zero(), 1.0, 1.0, 1.0, BLUE);

    EndMode3D();
    
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
