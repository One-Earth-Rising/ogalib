#version 410

out vec4 color;

uniform ShaderUniformBlock {
  mat4 mvp;
  vec4 colorScale;
};

void main() {
  color = colorScale;
}
