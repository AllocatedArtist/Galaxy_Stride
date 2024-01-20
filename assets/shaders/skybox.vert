#version 330 core

layout (location = 0) in vec3 vertexPosition;

uniform mat4 matProjection;
uniform mat4 matView;

out vec3 fragPosition;

void main() {
  fragPosition = vertexPosition; 

  mat4 rot_view = mat4(mat3(matView));
  vec4 pos = matProjection * rot_view * vec4(vertexPosition, 1.0);
  gl_Position = pos.xyww;
}
