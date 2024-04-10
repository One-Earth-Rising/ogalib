#version 410

#define MAX_BONE_COUNT 500

in vec2 tc;
in vec3 normal;

out vec4 color;

uniform ShaderUniformBlock {
  mat4 mvp;
  mat4 boneTransform[MAX_BONE_COUNT];
};

uniform sampler2D tex;

void main() {
  color = texture2D(tex, tc);
}
