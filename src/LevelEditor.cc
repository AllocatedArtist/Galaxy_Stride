#include "LevelEditor.h"
#include "raylib.h"

LevelEditor::LevelEditor() {
  model_cursor_pos_ = Vector3Zero();
  FilePathList model_paths = LoadDirectoryFiles("assets\\models");

  for (int i = 0; i < model_paths.count; ++i) {
    assets_.emplace_back(LevelAsset {
      ModelComponent(model_paths.paths[i], WHITE),
      LoadRenderTexture(100, 100),     
    });
  }
}

LevelEditor::~LevelEditor() {
  assets_.clear();
}

void LevelEditor::UpdateCamera(FlyCamera& camera) {
  if (IsKeyDown(KEY_LEFT_SHIFT)) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      show_thumbnail_ = false;
      DisableCursor();
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      EnableCursor();
    }
  }

  if (
    IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) 
    && !IsCursorHidden() 
    && !show_thumbnail_
  ) {
    Vector3 pos = camera.GetCamera().GetPosition();
    Vector3 right = camera.GetCamera().GetRight();
    right.y = 1.0;

    Vector2 delta = GetMouseDelta();

    Vector3 offset = { 
      delta.x, 
      -delta.y, 
      delta.x
    }; 

    offset = Vector3Normalize(Vector3Multiply(offset, right));

    pos = Vector3Add(pos, Vector3Scale(offset, 15.0 * GetFrameTime()));

    camera.GetCamera().SetPosition(pos);
  } 

  if (IsCursorHidden()) {
    camera.LookAround();
  }

  camera.Move();
}

void LevelEditor::PlaceObjects(std::vector<LevelMesh>& meshes, FlyCamera& camera) {
  if (selected_asset_ > -1 && !IsCursorHidden()) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {

      if (Vector3Equals(model_cursor_pos_, Vector3Zero())) {
        Vector3 mouse_pos = { (float)GetMouseX(), (float)GetMouseY(), 1.f };

        Vector3 camera_pos = camera.GetCamera().GetPosition();
        Vector3 forward = camera.GetCamera().GetForward();
        Vector3 right = camera.GetCamera().GetRight();
      }

      Ray mouse_ray = GetMouseRay(
        GetMousePosition(), 
        camera.GetCamera().GetCamera()
      );
   
      model_cursor_pos_ = Vector3Add(
        mouse_ray.position, 
        Vector3Scale(mouse_ray.direction, 5.0)
      );

 
      if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        meshes.emplace_back(LevelMesh {
          selected_asset_,
          model_cursor_pos_
        }); 
      }

      assets_[selected_asset_].model_.Draw(model_cursor_pos_);
    } else {
      model_cursor_pos_ = Vector3Zero();
    }
  }
}

void LevelEditor::DrawAsset(const LevelMesh& mesh) {
  assets_[mesh.index_].model_.Draw(mesh.pos_);
}

void LevelEditor::UpdateThumbnails() {
  for (LevelAsset& mesh : assets_) {
      Camera3D thumbnail_camera {
        .position = { 0.0, 1.0, -1.f },
        .target = Vector3Zero(),
        .up = Vector3 { 0.f, 1.f, 0.f },
        .fovy = 90.f,
        .projection = CAMERA_PERSPECTIVE
      };


      BeginTextureMode(mesh.thumbnail_);

      BoundingBox bounds = mesh.model_.GetBoundingBox();
      Ray camera_ray { 
        .position = { 0.0, 1.0, -1.f }, 
        .direction = Vector3Normalize(
          Vector3Subtract(
            Vector3Zero(), { 0.0, 1.0, -1.f }
          )
        )
      };

      RayCollision ray_info = GetRayCollisionBox(camera_ray, bounds);
      thumbnail_camera.position = 
        Vector3Add(ray_info.point, Vector3 { 0.0, 0.5, -0.5 });

      BeginMode3D(thumbnail_camera);
      ClearBackground(ColorBrightness(DARKBLUE, -0.2f)); 
      
      mesh.model_.Draw(Vector3Zero());

      EndMode3D();

      EndTextureMode();
    }
}

void LevelEditor::DrawThumbnails() {
  if (IsKeyPressed(KEY_TAB)) {
    show_thumbnail_ = !show_thumbnail_;
  }

  if (!show_thumbnail_ || IsCursorHidden()) {
    return;
  }

  float y = 200.0;
  float x = 0.f;

  Vector2 mouse_pos = GetMousePosition();

  for (int i = 0; i < assets_.size(); ++i) {
    if (i % 16 == 0) {
      x = 0.0;
      y += 100.0; 
    }
       
    DrawTextureRec(
      assets_[i].thumbnail_.texture,
      Rectangle { .x = 0.0, .y = 0.0, .width = 100.0, .height = -100.0 },
      { x, y },
      WHITE
    );
  
    if (selected_asset_ == i) {
      DrawRectangleLines(x, y, 100.0, 100.0, YELLOW);
    } else {
      DrawRectangleLines(x, y, 100.0, 100.0, BLACK);
    }
    
    if (CheckCollisionPointRec(mouse_pos, { x, y, 100.0, 100.0 })) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))  {
        if (selected_asset_ == i) {
          selected_asset_ = NO_SELECTED_ASSET;
        } else {
          selected_asset_ = i;
        }
      }
    }
    x += 100.0;
  } 
}
