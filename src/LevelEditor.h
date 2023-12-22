#ifndef LEVEL_EDITOR_H_
#define LEVEL_EDITOR_H_

#include <raylib.h>

#include <vector>

#include "FlyCamera.h"
#include "Model.h"

struct LevelAsset {
  ModelComponent model_;
  RenderTexture thumbnail_;
};

class LevelEditor {
public:
  LevelEditor();
  ~LevelEditor();

  void UpdateCamera(FlyCamera& camera);
  void UpdateThumbnails();

  void DrawThumbnails();
private:
  bool show_thumbnail_ = false;

  std::vector<LevelAsset> assets_;
};



#endif
