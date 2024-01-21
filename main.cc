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

  SetConfigFlags(ConfigFlags::FLAG_BORDERLESS_WINDOWED_MODE);
  InitWindow(window_width, window_height, "Galaxy Stride");

  InitAudioDevice();

  Music song = LoadMusicStream("assets\\sounds\\Blustery_Night.mp3");
  SetMusicVolume(song, 0.2);

  Skybox skybox;

  FlyCamera camera({ 0.0, 2.0, -5.0 }, 0.1, 5.0);
  camera.GetCamera().SetYaw(90.0);

  SetTargetFPS(120); 

  LevelEditor level_editor;
  level_editor.UpdateThumbnails();

  bool saved = false;
  float save_timer = 0.f;
  float save_delay = 1.0f;

  bool is_play_mode = kIsGameOnly;
  bool create_collision = true;
 
  Game game(level_editor);

  game.SetLevels({ 
    "assets\\levels\\level_0.json", 
    "assets\\levels\\level_2.json",
    "assets\\levels\\level_1.json",
    "assets\\levels\\level_3.json",
    "assets\\levels\\level_4.json",
  });


  int max_score = 0;

  bool menu = true;
  bool game_started = false;
  bool release_resources_game_over = false;
  int final_game_score = 0;

  SetExitKey(KEY_ESCAPE);
  level_editor.Load(
    game.GetFlag(), 
    game.GetMeshes(), 
    game.GetCoins(), 
    game.NextLevel().c_str()
  );

 
  while (!WindowShouldClose()) { 

    if (game_started && kIsGameOnly) {
      game_started = false;  
      max_score += game.GetCoins().size();
      song.looping = true;
      PlayMusicStream(song);
    }

    if (kIsGameOnly && !menu) {
      float pan = sinf((float)GetTime() * 0.3);
      SetMusicPan(song, pan);
    }

    UpdateMusicStream(song);

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
      } else {
        level_editor.ResetModes();
      }
    }

    if (IsKeyPressed(KEY_F2) && !kIsGameOnly) {
      level_editor.ResetLoadedFile();
    }

    if (IsKeyPressed(KEY_F3) && !kIsGameOnly) {
      ToggleFullscreen();
    }

    if (is_play_mode && create_collision) {
      create_collision = false;
      game.Setup(level_editor);
    }


    if (is_play_mode && !menu) {
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
      if (!level_editor.GetCurrentLoadedFileSaveName().empty()) {
        filename = level_editor.GetCurrentLoadedFileSaveName().c_str();
      }
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

    if (menu) {
      ShowCursor();
      DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), RAYWHITE);
      int screen_width = GetScreenWidth();
      int screen_height = GetScreenHeight();

      int center_x_1 = MeasureText("Galaxy Stride", 128);
      int center_x_2 = MeasureText("Collect Coins", 48);

      int center_x_3 = MeasureText("WASD - Move", 48);
      int center_x_4 = MeasureText("Mouse - Look Around", 48);
      int center_x_5 = MeasureText("Left Shift to Run", 48);
      int center_x_6 = MeasureText("Left Control to Crouch", 48);

      int center_x_7 = MeasureText(
        "Running and then crouching performs a slide.", 
        48
      );

      int center_x_8 = MeasureText(
        "Sliding and then jumping allows you to jump farther.",
        48
      );

      int center_x_9 = MeasureText(
        "Watch your stamina.",
        48
      );

      int center_x_10 = MeasureText(
        "Press Enter to start the game.",
        48
      );


      int center_x_11 = MeasureText(
        "Press Escape at any point to Quit.",
        48
      );


      DrawText(
        "Galaxy Stride", 
        (GetScreenWidth() / 2) - (center_x_1 / 2), 
        100, 
        128, 
        BLACK
      );

      DrawText(
        "WASD - Move", 
        (GetScreenWidth() / 2) - (center_x_3 / 2),
        300, 
        48,
        BLACK 
      );

      DrawText(
        "Mouse - Look Around", 
        (GetScreenWidth() / 2) - (center_x_4 / 2),
        400, 
        48,
        BLACK 
      );

      DrawText(
        "Left Shift to Run", 
        (GetScreenWidth() / 2) - (center_x_5 / 2),
        500, 
        48,
        BLACK 
      );

      DrawText(
        "Left Control to Crouch", 
        (GetScreenWidth() / 2) - (center_x_6 / 2),
        600, 
        48,
        BLACK 
      );

      DrawText(
        "Running and then crouching performs a slide.", 
        (GetScreenWidth() / 2) - (center_x_7 / 2),
        700, 
        48,
        BLACK 
      );

      DrawText(
        "Sliding and then jumping allows you to jump farther.",
        (GetScreenWidth() / 2) - (center_x_8 / 2),
        800, 
        48,
        BLACK 
      );

      DrawText(
        "Watch your stamina.",
        (GetScreenWidth() / 2) - (center_x_9 / 2),
        900, 
        48,
        RED 
      );

      DrawText(
        "Press Enter to start the game.",
        (GetScreenWidth() / 2) - (center_x_10 / 2),
        1100, 
        48,
        BLACK 
      );

      DrawText(
        "Press Escape at any point to Quit.",
        (GetScreenWidth() / 2) - (center_x_11 / 2),
        1200, 
        48,
        BLACK 
      );

      if (IsKeyPressed(KEY_ENTER)) {
        DisableCursor();
        menu = false;
        game_started = true;
      }
    }

    if (level_editor.IsCoinMode()) {
      DrawText("COIN MODE ON", 400, 200, 32, RED);
    } else if (level_editor.IsFlagMode()) {
      DrawText("FLAG MODE ON", 400, 200, 32, RED);
    }

    if (release_resources_game_over) {
      StopMusicStream(song);
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

  UnloadMusicStream(song);
  CloseAudioDevice();
  CloseWindow();

  return 0;
}
