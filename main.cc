#include <raylib.h>

#include "src/Model.h"
#include "src/FlyCamera.h"

#include <iostream>

int main(void) {
  
  InitWindow(1600, 1480, "Platformer");

  FlyCamera camera({ 0.0, 2.0, -4.0 }, 0.1, 5.0);

  ModelComponent box({ 0.5, 0.5, 0.5 }, RED);
  ModelComponent ground({ 5.0, 0.1, 5.0 }, GREEN);

  DisableCursor();
  while (!WindowShouldClose()) { 
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

    box.Draw({0.0, 0.2, 0.0});
    ground.Draw(Vector3Zero());

    EndMode3D();
    
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
