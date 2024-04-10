#version 410

in vec2 tc;
in vec3 normal;

out vec4 color;

uniform ShaderUniformBlock {
  mat4 mvp;
};

uniform sampler2D tex;

void main () { 
  color = texture2D(tex, tc);
}
