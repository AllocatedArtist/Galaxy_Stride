#ifndef LEVEL_EDITOR_H_
#define LEVEL_EDITOR_H_

#include <raylib.h>
#include <json.hpp>

#include <vector>

#include "FlyCamera.h"
#include "Model.h"

#define NO_SELECTED_ASSET -1

struct LevelAsset {
  ModelComponent model_;
  RenderTexture thumbnail_;
};

struct LevelMesh {
  int index_;
  Vector3 pos_;
  Quaternion rotation_;
  bool selected_;
};

struct LevelCoin {
  int index_;
  Vector3 pos_;
  Quaternion rotation_;
  bool collected_;
};

class LevelEditor {
public:
  LevelEditor();
  ~LevelEditor();

  void UpdateCamera(FlyCamera& camera);
  void UpdateThumbnails();

  void PlaceObjects(
    std::vector<LevelCoin>& coins, 
    std::vector<LevelMesh>& meshes, 
    FlyCamera& camera
  );

  void DrawThumbnails();
  void DrawAsset(const LevelMesh& mesh);
  void DrawCoins(const LevelCoin& coin);
  
  void SelectObject(LevelMesh& mesh, FlyCamera& camera);
  void PlacePlayer(FlyCamera& camera);

  const bool IsPlayerSetMode() const;

  const float GetPlayerYaw() const;
  const Vector3 GetPlayerPosition() const;

  const bool IsCoinMode() const;

  // 0, 90, 180, or 270
  void SetPlayerYaw(float yaw);

  void SetPlayerPosition(Vector3 position);

  void Save(const std::vector<LevelMesh>& meshes);
  void Load(std::vector<LevelMesh>& meshes);

  const std::string& GetCurrentFileSaveName() const;

  LevelAsset& GetAsset(int index);
private:
  nlohmann::json level_content_;
  std::string current_file_save_;
private:
  bool coin_mode_;
private:
  float player_angle_;
  Vector3 player_position_;
private:
  void DrawObjectBounds(const LevelMesh& mesh);
private:
  enum class Snap {
    kNone,
    kSnapX,
    kSnapY,
    kSnapZ
  };

  Snap snap_;
  Snap selection_snap_;
private:
  void SelectionMove(LevelMesh& mesh, FlyCamera& camera, Snap snap);
private:
  bool show_thumbnail_ = false;

  Vector3 model_cursor_pos_;
  Vector3 prev_cursor_pos_;
  float rot_angle_;

  bool set_player_;

  int selected_asset_ = NO_SELECTED_ASSET;
  std::vector<LevelAsset> assets_;
};



#endif
