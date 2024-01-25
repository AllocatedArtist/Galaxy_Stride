#version 330 core

out vec4 fragColor;

uniform vec3 base_color;

void main() {
  fragColor = vec4(base_color, 1.0);
}
