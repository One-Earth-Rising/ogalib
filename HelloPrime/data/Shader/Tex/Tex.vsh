#version 410

in vec2 vPos;
in vec2 vUV;

out vec2 tc;

uniform ShaderUniformBlock {
  mat4 mvp;
};

void main() {
  vec4 pos = mvp * vec4(vPos, 0.0, 1.0);
  tc = vUV;
  gl_Position = pos;
}
