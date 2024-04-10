#version 410

in vec3 vPos;
in vec2 vUV;
in vec3 vNormal;

out vec2 tc;
out vec3 normal;

uniform ShaderUniformBlock {
  mat4 mvp;
};

void main() {
  gl_Position = mvp * vec4(vPos, 1.0);
  tc = vUV;
  normal = vNormal;
}
