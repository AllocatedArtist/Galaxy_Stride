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

struct Flag {
  Vector3 flag_position_;
  Quaternion flag_rotation_;
  bool is_touched_;
};

enum ObjectType {
  kPlayer,
  kCoin,
  kFlag,
  kStaticModel
};

constexpr int kFlagModelIndex = 82;

class LevelEditor {
public:
  LevelEditor();
  ~LevelEditor();

  void UpdateCamera(FlyCamera& camera);
  void UpdateThumbnails();

  void PlaceObjects(
    Flag& flag,
    std::vector<LevelCoin>& coins, 
    std::vector<LevelMesh>& meshes, 
    FlyCamera& camera
  );

  void DrawThumbnails();
  void DrawAsset(const LevelMesh& mesh, bool play_mode);
  void DrawCoins(const LevelCoin& coin);
  void DrawFlag(const Flag& flag);
  
  void SelectObject(LevelMesh& mesh, FlyCamera& camera);
  void PlacePlayer(FlyCamera& camera);

  const bool IsPlayerSetMode() const;
  const bool IsCoinMode() const;
  const bool IsFlagMode() const;

  const float GetPlayerYaw() const;
  const Vector3 GetPlayerPosition() const;

  // 0, 90, 180, or 270
  void SetPlayerYaw(float yaw);

  void SetPlayerPosition(Vector3 position);

  void Save(
    const Flag& flag,
    const std::vector<LevelMesh>& meshes, 
    const std::vector<LevelCoin>& coins
  );

  void Load(
    Flag& flag,
    std::vector<LevelMesh>& meshes,
    std::vector<LevelCoin>& coins,
    const char* filename = nullptr
  );

  const std::string& GetCurrentFileSaveName() const;
  const std::string& GetCurrentLoadedFileSaveName() const;

  LevelAsset& GetAsset(int index);

  void ResetLoadedFile();

  void ResetModes();
private:
  nlohmann::json level_content_;
  std::string current_file_save_;
  std::string loaded_file_;
private:
  bool coin_mode_;
  bool flag_mode_;
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
