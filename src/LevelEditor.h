#ifndef LEVEL_EDITOR_H_
#define LEVEL_EDITOR_H_

#include <raylib.h>

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
};

class LevelEditor {
public:
  LevelEditor();
  ~LevelEditor();

  void UpdateCamera(FlyCamera& camera);
  void UpdateThumbnails();

  void PlaceObjects(std::vector<LevelMesh>& meshes, FlyCamera& camera);

  void DrawThumbnails();

  void DrawAsset(const LevelMesh& mesh);
private:
  bool show_thumbnail_ = false;

  Vector3 model_cursor_pos_;

  int selected_asset_ = NO_SELECTED_ASSET;
  std::vector<LevelAsset> assets_;
};



#endif
