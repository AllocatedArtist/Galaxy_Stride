#define GLSL_VERSION 330
#include <raylib.h>

#include "src/Game.h"
#include "src/FlyCamera.h"
#include "src/LevelEditor.h"
#include "src/Skybox.h"

int main(void) {

  constexpr bool kIsGameOnly = true;

  ConfigFlags flags;

  int window_width = 0;
  int window_height = 0;

  if (kIsGameOnly) {
    flags = ConfigFlags::FLAG_FULLSCREEN_MODE;
  } else {
    window_width = 1600;
    window_height = 1480;
    flags = ConfigFlags::FLAG_WINDOW_RESIZABLE;
  }

  SetConfigFlags(flags);
  InitWindow(window_width, window_height, "Platformer");

  InitAudioDevice();

  Skybox skybox;

  FlyCamera camera({ 0.0, 2.0, -5.0 }, 0.1, 5.0);
  camera.GetCamera().SetYaw(90.0);

  SetTargetFPS(120); 

  LevelEditor level_editor;
  level_editor.UpdateThumbnails();

  SetExitKey(KEY_NULL);

  bool saved = false;
  float save_timer = 0.f;
  float save_delay = 1.0f;

  bool is_play_mode = kIsGameOnly;
  bool create_collision = true;
 
  Game game(level_editor);

  game.SetLevels({ "level_0.json", "level_1.json"});

  int max_score = 0;

  if (kIsGameOnly) {
    SetExitKey(KEY_ESCAPE);
    level_editor.Load(
      game.GetFlag(), 
      game.GetMeshes(), 
      game.GetCoins(), 
      game.NextLevel().c_str()
    );
    max_score += game.GetCoins().size();
  }

  bool release_resources_game_over = false;
  int final_game_score = 0;
 
  while (!WindowShouldClose()) { 
    if (
      game.IsGameOver() && 
      kIsGameOnly && 
      game.GetFlag().is_touched_ && 
      !release_resources_game_over
    ) {
      release_resources_game_over = true;
      final_game_score += game.GetScore();
      game.Unload();
    }

    if (game.GetFlag().is_touched_ && kIsGameOnly) {
      final_game_score += game.GetScore();
      game.Unload();
      level_editor.Load(
        game.GetFlag(), 
        game.GetMeshes(), 
        game.GetCoins(), 
        game.NextLevel().c_str()
      );
      max_score += game.GetCoins().size();
      game.Setup(level_editor);
    } else if (game.GetFlag().is_touched_ && !kIsGameOnly) {
      is_play_mode = false;
      create_collision = true;
      game.Unload();
    }

    if (IsKeyPressed(KEY_F1) && !kIsGameOnly) {
      is_play_mode = !is_play_mode;
      if (!is_play_mode) {
        create_collision = true;
        game.Unload();
      }
    }

    if (is_play_mode && create_collision) {
      create_collision = false;
      game.Setup(level_editor);
    }


    if (is_play_mode) {
      game.Update(level_editor);
    }

    if (!is_play_mode) {
      level_editor.UpdateCamera(camera); 
      level_editor.Save(game.GetFlag(), game.GetMeshes(), game.GetCoins());

      if (
        IsKeyDown(KEY_LEFT_CONTROL) && 
        IsKeyPressed(KEY_S) && 
        saved == false
      ) {
        saved = true; 
      }

      level_editor.Load(game.GetFlag(), game.GetMeshes(), game.GetCoins());
    }
     
    BeginDrawing(); 
    ClearBackground(BLACK);

    Camera main_camera = 
      is_play_mode ? game.GetCamera() : camera.GetCamera().GetCamera();
  
    BeginMode3D(main_camera); 

    if (!is_play_mode) {
      level_editor.PlacePlayer(camera);
    
      DrawGrid(20, 1.0);

      if (!level_editor.IsPlayerSetMode()) {
        level_editor.PlaceObjects(
          game.GetFlag(), 
          game.GetCoins(), 
          game.GetMeshes(), 
          camera
        );

        for (LevelMesh& mesh : game.GetMeshes()) {
          level_editor.SelectObject(mesh, camera);
        }
      }
    }

    for (const LevelCoin& coin : game.GetCoins()) {
      if (!coin.collected_) {
        level_editor.DrawCoins(coin);
      }
    }

    for (const LevelMesh& mesh : game.GetMeshes()) {
      level_editor.DrawAsset(mesh, is_play_mode);
    }

    if (!level_editor.IsFlagMode()) {
      level_editor.DrawFlag(game.GetFlag());
    }

    rlDisableBackfaceCulling();
    skybox.Draw(is_play_mode ? game.GetFlyCamera() : camera);
    rlEnableBackfaceCulling();
 
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
      game.DrawUI();
    }

    if (level_editor.IsCoinMode()) {
      DrawText("COIN MODE ON", 400, 200, 32, RED);
    } else if (level_editor.IsFlagMode()) {
      DrawText("FLAG MODE ON", 400, 200, 32, RED);
    }

    if (release_resources_game_over) {
      DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), RAYWHITE);

      int screen_width = GetScreenWidth();
      int screen_height = GetScreenHeight();

      const char* score_text = 
        TextFormat("Score: %d/%d", final_game_score, max_score);


      int center_x_1 = MeasureText("CONGRATULATIONS!", 48);
      int center_x_2 = MeasureText(score_text, 48);
      int center_x_3 = MeasureText("You can press Escape to exit.", 64);
      DrawText(
        "CONGRATULATIONS!", 
        (screen_width / 2) - (center_x_1 / 2), 
        (screen_height / 2) - 40, 
        48, 
        RED
      );

      DrawText(
        score_text,
        (screen_width / 2) - (center_x_2 / 2), 
        (screen_height / 2), 
        48, 
        BLACK
      );
      DrawText(
        "You can press Escape to exit.",
        (screen_width / 2) - (center_x_3 / 2),
        (screen_height / 2) + 200,
        64,
        BLACK
      );
    }

    DrawFPS(0, 0);
    
    EndDrawing(); 
  }

  CloseAudioDevice();
  CloseWindow();

  return 0;
}
