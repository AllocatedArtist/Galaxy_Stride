#include "LevelEditor.h"

#include <fstream>

LevelEditor::LevelEditor() {
  model_cursor_pos_ = Vector3Zero();
  prev_cursor_pos_ = Vector3Zero();
  rot_angle_ = 0.f;


  FilePathList model_paths = LoadDirectoryFiles("assets\\models");

  for (int i = 0; i < model_paths.count; ++i) {
    assets_.emplace_back(LevelAsset {
      ModelComponent(model_paths.paths[i], WHITE),
      LoadRenderTexture(100, 100),     
    });
  }

  snap_ = Snap::kNone;
  selection_snap_ = Snap::kNone;
  set_player_ = false;

  player_angle_ = 0.0f;
  player_position_ = Vector3Zero();

  coin_mode_ = false;
  flag_mode_ = false;
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

void LevelEditor::PlaceObjects(
  Flag& flag,
  std::vector<LevelCoin>& coins,
  std::vector<LevelMesh>& meshes, 
  FlyCamera& camera
) {
  if (
    IsKeyDown(KEY_LEFT_CONTROL) && 
    IsKeyPressed(KEY_Z) && 
    !meshes.empty() &&
    !coin_mode_) {
    meshes.pop_back();
  } else if (
    IsKeyDown(KEY_LEFT_CONTROL) && 
    IsKeyPressed(KEY_Z) && 
    !coins.empty() &&
    coin_mode_
  ) {
    coins.pop_back();
  }

  if (
    IsKeyDown(KEY_LEFT_CONTROL) && 
    IsKeyPressed(KEY_C) && 
    !flag_mode_
  ) {
    coin_mode_ = !coin_mode_;
  } 

  if (
    IsKeyDown(KEY_LEFT_CONTROL) && 
    IsKeyPressed(KEY_F) &&
    !coin_mode_
  ) {
    flag_mode_ = !flag_mode_;
  }
  
  if (
    (selected_asset_ > -1 && !IsCursorHidden()) || 
    (flag_mode_ && !IsCursorHidden())
  ) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || flag_mode_) {
      if (IsKeyPressed(KEY_E)) {
        snap_ = Snap::kNone; 
      }
      if (IsKeyPressed(KEY_R)) {
        snap_ = Snap::kSnapX; 
      }
      if (IsKeyPressed(KEY_T)) {
        snap_ = Snap::kSnapY; 
      }
      if (IsKeyPressed(KEY_Y)) {
        snap_ = Snap::kSnapZ; 
      }

      Ray mouse_ray = GetMouseRay(
        GetMousePosition(), 
        camera.GetCamera().GetCamera()
      );

      mouse_ray.direction = Vector3Scale(mouse_ray.direction, 5.0);

      Vector3 offset = Vector3Add(
        mouse_ray.position, 
        mouse_ray.direction
      ); 

      Vector3 snap_offset = Vector3Zero();
      switch (snap_) {
        case Snap::kNone: {
          prev_cursor_pos_ = Vector3Zero();
          offset = Vector3Add(mouse_ray.position, mouse_ray.direction);
          break;
        }
        case Snap::kSnapX: {
          snap_offset = Vector3Project(
            offset, 
            { 1.0, 0.0, 0.0 }
          );
          if (Vector3Equals(prev_cursor_pos_, Vector3Zero())) {
            prev_cursor_pos_ = model_cursor_pos_;
          }
          offset.x = snap_offset.x;
          offset.y = prev_cursor_pos_.y;
          offset.z = prev_cursor_pos_.z;
          break;
        }
        case Snap::kSnapY: {
          snap_offset = Vector3Project(
            offset, 
            { 0.0, 1.0, 0.0 }
          );
          if (Vector3Equals(prev_cursor_pos_, Vector3Zero())) {
            prev_cursor_pos_ = model_cursor_pos_;
          }
          offset.x = prev_cursor_pos_.x;
          offset.y = snap_offset.y;
          offset.z = prev_cursor_pos_.z;
          break;
        }
        case Snap::kSnapZ: {
          snap_offset = Vector3Project(
            offset, 
            { 0.0, 0.0, 1.0 }
          );
          if (Vector3Equals(prev_cursor_pos_, Vector3Zero())) {
            prev_cursor_pos_ = model_cursor_pos_;
          }
          offset.x = prev_cursor_pos_.x;
          offset.y = prev_cursor_pos_.y;
          offset.z = snap_offset.z;
          break;
        }
      }

      if (IsKeyDown(KEY_ONE)) {
        rot_angle_ -= 25.f * GetFrameTime();
      }
      if (IsKeyDown(KEY_THREE)) {
        rot_angle_ += 25.f * GetFrameTime();
      }
  
      model_cursor_pos_ = offset;
      prev_cursor_pos_ = model_cursor_pos_;
 
      if (
        IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && 
        !coin_mode_ &&
        !flag_mode_
      ) {
        meshes.emplace_back(LevelMesh {
          selected_asset_,
          model_cursor_pos_,
          QuaternionFromAxisAngle({ 0.f, 1.f, 0.f }, rot_angle_ * DEG2RAD),
          false
        }); 
      } 
      if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && coin_mode_) {
        coins.emplace_back(LevelCoin {
          selected_asset_,
          model_cursor_pos_,
          QuaternionFromAxisAngle({ 0.f, 1.f, 0.f }, rot_angle_ * DEG2RAD),
          false
        });
      } 
      if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && flag_mode_) {
        flag.flag_position_ = model_cursor_pos_;
        flag.flag_rotation_ = 
          QuaternionFromAxisAngle({ 0.f, 1.f, 0.f }, rot_angle_ * DEG2RAD);
        flag_mode_ = false;
      }
      
      int asset_draw = flag_mode_ ? kFlagModelIndex : selected_asset_;

      assets_[asset_draw].model_.Draw(
        model_cursor_pos_, 
        {1.0, 1.0, 1.0 }, 
        QuaternionFromAxisAngle({ 0.f, 1.f, 0.f }, rot_angle_ * DEG2RAD)
      ); 
    } else {
      snap_ = Snap::kNone;
      model_cursor_pos_ = Vector3Zero();
      prev_cursor_pos_ = Vector3Zero();
      rot_angle_ = 0.f;
    }
  }
}

void LevelEditor::DrawObjectBounds(const LevelMesh& mesh) {
  BoundingBox bounding_box = assets_[mesh.index_].model_.GetBoundingBox(); 
     
  // min start
  Vector3 p1 = Vector3Add(mesh.pos_, bounding_box.min);
  // max start
  Vector3 p2 = Vector3Add(mesh.pos_, bounding_box.max);

  Vector3 p3 = { p2.x, p1.y, p2.z };
  Vector3 p4 = { p1.x, p1.y, p2.z };
  Vector3 p5 = { p2.x, p1.y, p1.z };

  Vector3 p6 = { p1.x, p2.y, p2.z };
  Vector3 p7 = { p1.x, p2.y, p1.z };
  Vector3 p8 = { p2.x, p2.y, p1.z }; 
    
  DrawLine3D(p1, p5, GREEN);
  DrawLine3D(p1, p4, GREEN);
  DrawLine3D(p4, p3, GREEN);
  DrawLine3D(p3, p5, GREEN);

  DrawLine3D(p5, p8, GREEN);
  DrawLine3D(p4, p6, GREEN);
  DrawLine3D(p1, p7, GREEN);
  DrawLine3D(p2, p3, GREEN);

  DrawLine3D(p7, p8, GREEN);
  DrawLine3D(p7, p6, GREEN);
  DrawLine3D(p6, p2, GREEN);
  DrawLine3D(p2, p8, GREEN);
}

void LevelEditor::DrawAsset(const LevelMesh& mesh, bool play_mode) {
  assets_[mesh.index_].model_.Draw(
    mesh.pos_, 
    { 1.0, 1.0, 1.0 }, 
    mesh.rotation_
  );

  if (mesh.selected_) {
    DrawObjectBounds(mesh);
  }

  if (IsKeyDown(KEY_C) && !play_mode) {
    DrawObjectBounds(mesh);
  }
}

void LevelEditor::DrawCoins(const LevelCoin& coin) {
  LevelAsset& model = assets_[coin.index_];
  model.model_.Draw(coin.pos_, { 1.0, 1.0, 1.0 }, coin.rotation_);  
}

void LevelEditor::SelectObject(LevelMesh& mesh, FlyCamera& camera) {
  BoundingBox bounding_box = assets_[mesh.index_].model_.GetBoundingBox(); 
     
  // min start
  Vector3 p1 = Vector3Add(mesh.pos_, bounding_box.min);
  // max start
  Vector3 p2 = Vector3Add(mesh.pos_, bounding_box.max);

  Ray mouse_ray = GetMouseRay(
    GetMousePosition(), camera.GetCamera().GetCamera()
  );

  RayCollision ray = GetRayCollisionBox(
    mouse_ray,
    BoundingBox { .min = p1, .max = p2 }
  );

  if (
    ray.hit && 
    IsKeyDown(KEY_LEFT_CONTROL) && 
    IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)
  ) {
    mesh.selected_ = !mesh.selected_; 
  }

  if (mesh.selected_) {
    Vector3 x_axis = Vector3Add(mesh.pos_, { 3.0, 0.0, 0.0 });
    Vector3 y_axis = Vector3Add(mesh.pos_, { 0.0, 3.0, 0.0 });
    Vector3 z_axis = Vector3Add(mesh.pos_, { 0.0, 0.0, 3.0 });

    DrawLine3D(mesh.pos_, y_axis, BLUE);
    DrawLine3D(mesh.pos_, x_axis, RED);
    DrawLine3D(mesh.pos_, z_axis, GREEN);

    if (GetRayCollisionSphere(mouse_ray, x_axis, 0.1f).hit) {
      selection_snap_ = Snap::kSnapX;
      DrawSphere(x_axis, 0.1f, RED);
    } else {
      DrawSphere(x_axis, 0.1f, Color { 100, 0, 0, 255 });
    }

    if (GetRayCollisionSphere(mouse_ray, y_axis, 0.1f).hit) {
      selection_snap_ = Snap::kSnapY;
      DrawSphere(y_axis, 0.1f, BLUE);
    } else {
      DrawSphere(y_axis, 0.1f, DARKBLUE);
    }

    if (GetRayCollisionSphere(mouse_ray, z_axis, 0.1f).hit) {
      selection_snap_ = Snap::kSnapZ;
      DrawSphere(z_axis, 0.1f, GREEN); 
    } else {
      DrawSphere(z_axis, 0.1f, DARKGREEN); 
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
      prev_cursor_pos_ = mesh.pos_;
      SelectionMove(mesh, camera, selection_snap_);
    } else {
      prev_cursor_pos_ = Vector3Zero();
      selection_snap_ = Snap::kNone;
    }
  
  }
}

void LevelEditor::SelectionMove(
  LevelMesh& mesh, 
  FlyCamera& camera, 
  Snap snap
) {
  Ray mouse_ray = GetMouseRay(
    GetMousePosition(), 
    camera.GetCamera().GetCamera()
  );

  mouse_ray.direction = Vector3Scale(mouse_ray.direction, 5.0);

  Vector3 offset = Vector3Add(
    mouse_ray.position, 
    mouse_ray.direction
  ); 

  Vector3 snap_offset = Vector3Zero();
  switch (snap) {
    case Snap::kNone: {
      prev_cursor_pos_ = Vector3Zero();
      offset = mesh.pos_;
      break;
    }
    case Snap::kSnapX: {
      snap_offset = Vector3Project(
        offset, 
        { 1.0, 0.0, 0.0 }
      );
      if (Vector3Equals(prev_cursor_pos_, Vector3Zero())) {
        prev_cursor_pos_ = mesh.pos_;
      }
      offset.x = snap_offset.x;
      offset.y = prev_cursor_pos_.y;
      offset.z = prev_cursor_pos_.z;
      break;
    }
    case Snap::kSnapY: {
      snap_offset = Vector3Project(
        offset, 
        { 0.0, 1.0, 0.0 }
      );
      if (Vector3Equals(prev_cursor_pos_, Vector3Zero())) {
        prev_cursor_pos_ = mesh.pos_;
      }
      offset.x = prev_cursor_pos_.x;
      offset.y = snap_offset.y;
      offset.z = prev_cursor_pos_.z;
      break;
    }
    case Snap::kSnapZ: {
      snap_offset = Vector3Project(
        offset, 
        { 0.0, 0.0, 1.0 }
      );
      if (Vector3Equals(prev_cursor_pos_, Vector3Zero())) {
        prev_cursor_pos_ = mesh.pos_;
      }
      offset.x = prev_cursor_pos_.x;
      offset.y = prev_cursor_pos_.y;
      offset.z = snap_offset.z;
      break;
    }
  } 

  mesh.pos_ = offset; 
  prev_cursor_pos_ = mesh.pos_;
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

void LevelEditor::PlacePlayer(FlyCamera& camera) {
  if (IsKeyPressed(KEY_P) && !IsCoinMode()) {
    set_player_ = !set_player_;
  }

  if (set_player_) {
    Ray mouse_ray = GetMouseRay(
      GetMousePosition(), 
      camera.GetCamera().GetCamera()
    );

    mouse_ray.direction = Vector3Scale(mouse_ray.direction, 3.0);
    Vector3 pos = Vector3Add(mouse_ray.position, mouse_ray.direction);


    if (IsKeyPressed(KEY_E)) {
      player_angle_ += 90.0;
    } else if (IsKeyPressed(KEY_Q)) {
      player_angle_ -= 90.0;
    }

    player_angle_ = Clamp(player_angle_, 0.0, 270.0);

    Vector3 dir = Vector3Add(pos, { 0.0, 0.25, 0.0 });
    Vector3 dir_end = Vector3Zero();

    if (player_angle_ == 90.0f)
      dir_end = Vector3Add(dir, { 0.0, 0.0, 1.0 });
    else if (player_angle_ == 0.0f)
      dir_end = Vector3Add(dir, { 1.0, 0.0, 0.0 });
    else if (player_angle_ == 180.0f)
      dir_end = Vector3Add(dir, { -1.0, 0.0, 0.0 });
    else if (player_angle_ == 270.0f)
      dir_end = Vector3Add(dir, { 0.0, 0.0, -1.0 });

    DrawCapsule(pos, Vector3Add(pos, { 0.0, 0.5, 0.0 }), 0.1, 8, 8, BLUE); 
    DrawLine3D(dir, dir_end, GREEN);

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      player_position_ = pos;
      set_player_ = false;
    }
  } else {
    DrawCapsule(
      player_position_, 
      Vector3Add(player_position_, { 0.0, 0.5, 0.0 }), 
      0.1, 
      8, 
      8, 
      BLUE
    ); 

    Vector3 dir = Vector3Add(player_position_, { 0.0, 0.25, 0.0 });
    Vector3 dir_end = Vector3Zero();

    if (player_angle_ == 90.0f)
      dir_end = Vector3Add(dir, { 0.0, 0.0, 1.0 });
    else if (player_angle_ == 0.0f)
      dir_end = Vector3Add(dir, { 1.0, 0.0, 0.0 });
    else if (player_angle_ == 180.0f)
      dir_end = Vector3Add(dir, { -1.0, 0.0, 0.0 });
    else if (player_angle_ == 270.0f)
      dir_end = Vector3Add(dir, { 0.0, 0.0, -1.0 });

    DrawLine3D(dir, dir_end, GREEN);
  }
}

const bool LevelEditor::IsPlayerSetMode() const {
  return set_player_;
}

const float LevelEditor::GetPlayerYaw() const {
  return player_angle_;
}

const Vector3 LevelEditor::GetPlayerPosition() const {
  return player_position_;
}

void LevelEditor::SetPlayerYaw(float yaw) {
  player_angle_ = yaw;
}

void LevelEditor::SetPlayerPosition(Vector3 position) {
  player_position_ = position;
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

void LevelEditor::Save(
  const Flag& flag,
  const std::vector<LevelMesh>& meshes,
  const std::vector<LevelCoin>& coins
) {
  if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
    int index = 0;

    Vector3 player_pos = GetPlayerPosition();
    float player_rot = GetPlayerYaw();
  
    level_content_[index] = {
      { "type", kPlayer },
      { "position", { player_pos.x, player_pos.y, player_pos.z }},
      { "rotation", { player_rot } }
    };

    index += 1;
    
    for (const LevelCoin& coin : coins) {
      level_content_[index] = {
        { "type", kCoin },
        { "position", { coin.pos_.x, coin.pos_.y, coin.pos_.z } },
        { "rotation", 
          { 
            coin.rotation_.x, 
            coin.rotation_.y, 
            coin.rotation_.z, 
            coin.rotation_.w 
          } 
        },
        { "mesh", coin.index_ }
      };
      index += 1;
    }

    level_content_[index] = {
      { "type", kFlag },
      { 
        "position", 
        { 
          flag.flag_position_.x, 
          flag.flag_position_.y, 
          flag.flag_position_.z 
        }
      },
      { 
        "rotation",
        {
          flag.flag_rotation_.x,
          flag.flag_rotation_.y,
          flag.flag_rotation_.z,
          flag.flag_rotation_.w,
        }
      }
    };

    index += 1;

    for (const LevelMesh& mesh : meshes) {
      level_content_[index] = {
        { "type", kStaticModel },
        { "position", { mesh.pos_.x, mesh.pos_.y, mesh.pos_.z } },
        { "rotation", 
          { 
            mesh.rotation_.x, 
            mesh.rotation_.y, 
            mesh.rotation_.z, 
            mesh.rotation_.w 
          } 
        },
        { "mesh", mesh.index_ }
      };
        index += 1;
      }

      int level_file_index = 0;
      while (FileExists(TextFormat("level_%d.json", level_file_index))) {
        level_file_index += 1;
      }

      current_file_save_ = TextFormat("level_%d.json", level_file_index);
      
      std::ofstream file;
      file.open(loaded_file_.empty() ? current_file_save_ : loaded_file_);
      file << level_content_.dump(2);
      file.close();
    }
}

void LevelEditor::Load(
  Flag& flag,
  std::vector<LevelMesh>& meshes, 
  std::vector<LevelCoin>& coins,
  const char* filename
) {

  const char* load_file = nullptr;

  if (IsFileDropped() && filename == nullptr) {
    FilePathList dropped = LoadDroppedFiles();
    if (IsFileExtension(dropped.paths[0], ".json")) {
      load_file = dropped.paths[0];
    }
    UnloadDroppedFiles(dropped);
  }

  if (filename != nullptr) {
    load_file = filename;
  }

  if (load_file == nullptr) {
    return;
  }

  loaded_file_ = load_file;

  meshes.clear();
  coins.clear();
  std::ifstream level_file(load_file);
  nlohmann::json contents = nlohmann::json::parse(level_file);

  int index = 0;
  for (auto& [key, value] : contents.items()) {
    ObjectType type = value["type"];
    switch (type) {
      case kPlayer: {
        std::vector<float> position = value["position"];
        std::vector<float> rotation = value["rotation"];
        player_position_ = Vector3 { 
          position[0],
          position[1],
          position[2]
        };
        player_angle_ = rotation[0];
        break;
      }
      case kStaticModel: {
        std::vector<float> position = value["position"];
        std::vector<float> rotation = value["rotation"];
        int index = value["mesh"];

        meshes.emplace_back(LevelMesh {
          .index_ = index,
          .pos_ = Vector3 { position[0], position[1], position[2] },
          .rotation_ = Quaternion { 
            rotation[0], 
            rotation[1], 
            rotation[2], 
            rotation[3] 
          },
          .selected_ = false
        });
        break;
      }
      case kCoin: {
        std::vector<float> position = value["position"];
        std::vector<float> rotation = value["rotation"];
        int index = value["mesh"];

        coins.emplace_back(LevelCoin {
          .index_ = index,
          .pos_ = Vector3 { position[0], position[1], position[2] },
          .rotation_ = Quaternion { 
            rotation[0], 
            rotation[1], 
            rotation[2], 
            rotation[3] 
          },
          .collected_ = false
        });
        break;
      }
      case kFlag: {
        std::vector<float> position = value["position"];
        std::vector<float> rotation = value["rotation"];

        flag.flag_position_ = { position[0], position[1], position[2] };
        flag.flag_rotation_ = { 
          rotation[0], 
          rotation[1], 
          rotation[2], 
          rotation[3] 
        };
        flag.is_touched_ = false;
        break;
      }
    }
  }    
}  

const std::string& LevelEditor::GetCurrentFileSaveName() const {
  return current_file_save_; 
}

const std::string& LevelEditor::GetCurrentLoadedFileSaveName() const {
  return loaded_file_;
}


LevelAsset& LevelEditor::GetAsset(int index) {
  return assets_[index];
}

const bool LevelEditor::IsCoinMode() const {
  return coin_mode_;
}

void LevelEditor::DrawFlag(const Flag& flag) {
  assets_[kFlagModelIndex].model_
    .Draw(
      flag.flag_position_, 
      { 1.0, 1.0, 1.0 }, 
      flag.flag_rotation_
    );
}

const bool LevelEditor::IsFlagMode() const {
  return flag_mode_;
}

void LevelEditor::ResetModes() {
  coin_mode_ = false;
  set_player_ = false;
  flag_mode_ = false;
}

void LevelEditor::ResetLoadedFile() {
  loaded_file_ = "";
}

