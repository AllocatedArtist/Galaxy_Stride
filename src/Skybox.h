#ifndef SKYBOX_H_
#define SKYBOX_H_

#include <raylib.h>
#include <rlgl.h>
#include <stdint.h>

#include "FlyCamera.h"

class Skybox {
public:
  Skybox();
  void Draw(FlyCamera& camera);
private:
  uint32_t texture_id_;
  Shader skybox_shader_;

  uint32_t cube_vao_;
  uint32_t cube_vbo_;

  int uniform_projection_;
  int uniform_view_;
  int uniform_environment_map_;
};


#endif
