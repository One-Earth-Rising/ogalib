#version 410

in vec2 vPos;

uniform ShaderUniformBlock {
  mat4 mvp;
  vec4 colorScale;
};

void main() {
  vec4 pos = mvp * vec4(vPos, 0.0, 1.0);
  gl_Position = pos;
}
