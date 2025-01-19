#version 330 core

in vec4 ciPosition;
in vec2 ciTexCoord0;
in vec3 ciNormal;

uniform mat4 model;

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;

void main() {
  FragPos = (model * ciPosition).xyz;

  gl_Position = projection * view * vec4(FragPos, 1.0);
}