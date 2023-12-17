#include <raylib.h>

#include "src/FlyCamera.h"

int main(void) {
  
  InitWindow(1600, 1480, "Platformer");

  FlyCamera camera({ 0.0, 2.0, -4.0 }, 0.1, 5.0);

  DisableCursor();
  while (!WindowShouldClose()) { 

    camera.LookAround();
    camera.Move();

    BeginDrawing(); 
    ClearBackground(BLACK);

    BeginMode3D(camera.GetCamera().GetCamera());
    DrawGrid(20, 1.0);
    DrawCube(Vector3Zero(), 1.0, 1.0, 1.0, BLUE);

    EndMode3D();
    
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
