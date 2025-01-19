#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
out vec4 FragColor;
uniform vec3 cameraPos;
uniform sampler2D envv;

vec2 directionToSphericalEnvmap(vec3 dir) {
  dir = normalize(dir);
  vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y));
  uv /= vec2(2.0 * 3.14159265359, 3.14159265359);
  uv += 0.5;
  //   uv.y = 1.0 - uv.y;
  return uv;
}

void main() {
  vec3 viewDir = normalize(FragPos - cameraPos);
  vec2 texCoords = directionToSphericalEnvmap(viewDir);
  FragColor = texture(envv, texCoords);
  //   FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}