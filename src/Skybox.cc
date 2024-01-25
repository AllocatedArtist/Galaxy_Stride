#include "Skybox.h"

#include <array>
#include <glad.h>

#include <raylib-physfs.h>

Skybox::Skybox() {
  glDepthFunc(GL_LEQUAL);

  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);
 
  std::array<const char*, 6> skybox_files {
    "assets/skybox/corona_rt.png",
    "assets/skybox/corona_lf.png",
    "assets/skybox/corona_up.png",
    "assets/skybox/corona_dn.png",
    "assets/skybox/corona_ft.png",
    "assets/skybox/corona_bk.png",
  };

  for (int i = 0; i < skybox_files.size(); ++i) {
    const char* filename = skybox_files[i];
    Image image = LoadImageFromPhysFS(filename);

    if (i != 2 && i != 3) {
      ImageFlipHorizontal(&image);
    } else if (i == 3){
      ImageRotate(&image, 270);
      ImageFlipHorizontal(&image);
    } else if (i == 2) {
      ImageRotate(&image, 270);
      ImageFlipVertical(&image);
    }

    glTexImage2D(
      GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
      0, 
      GL_RGB, 
      image.width, 
      image.height, 
      0,
      GL_RGB, 
      GL_UNSIGNED_BYTE,
      (unsigned char*)image.data
    );

    UnloadImage(image);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0); 

  skybox_shader_ = LoadShaderFromPhysFS(
    "assets/shaders/skybox.vert", 
    "assets/shaders/skybox.frag"
  );

  uniform_projection_ = GetShaderLocation(skybox_shader_, "matProjection");

  uniform_view_ = GetShaderLocation(skybox_shader_, "matView");

  uniform_environment_map_ = GetShaderLocation(
    skybox_shader_, 
    "environmentMap"
  );

  //from learnopengl
  float skybox_vertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
  };

  glGenVertexArrays(1, &cube_vao_);
  glBindVertexArray(cube_vao_);

  glGenBuffers(1, &cube_vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_);

  glBufferData(
    GL_ARRAY_BUFFER, 
    sizeof(skybox_vertices), 
    skybox_vertices, 
    GL_STATIC_DRAW
  );

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

Skybox::~Skybox() {
  glDeleteVertexArrays(1, &cube_vao_);
  glDeleteBuffers(1, &cube_vbo_);
}


void Skybox::Draw(FlyCamera& camera) {

  Vector3 position = camera.GetCamera().GetPosition();
  Vector3 forward = camera.GetCamera().GetForward();

  float screen_width = (float)GetScreenWidth();
  float screen_height = (float)GetScreenHeight();

  
  Matrix projection = MatrixPerspective(
    camera.GetCamera().GetFOV() * DEG2RAD,
    screen_width / screen_height, 
    0.01, 
    100.0
  );
  
  Matrix view = MatrixLookAt(
    position, 
    Vector3Add(position, forward), 
    { 0.f, 1.f, 0.f }
  );

  SetShaderValueMatrix(skybox_shader_, uniform_projection_, projection);
  SetShaderValueMatrix(skybox_shader_, uniform_view_, view);

  glDepthMask(GL_FALSE);
  glUseProgram(skybox_shader_.id);

  glBindVertexArray(cube_vao_);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  glDepthMask(GL_TRUE);
}

