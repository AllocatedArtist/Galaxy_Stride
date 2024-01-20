#version 330 core

out vec4 fragColor;

in vec3 fragPosition;

uniform samplerCube environmentMap;

void main() {
  fragColor = texture(environmentMap, fragPosition);
}
