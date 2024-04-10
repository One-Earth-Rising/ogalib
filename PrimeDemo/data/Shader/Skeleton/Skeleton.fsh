#version 410

#define MAX_BONE_COUNT 200

in vec2 tc;
in vec4 bc;
in float alpha;

out vec4 color;

uniform ShaderUniformBlock {
  mat4 mvp;
  vec3 boneTransform1[MAX_BONE_COUNT];
  vec3 boneTransform2[MAX_BONE_COUNT];
  mat4 boneTransform[MAX_BONE_COUNT];
};

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

void main() {
  vec4 color0 = bc[0] > 0.0 ? texture2D(tex0, tc) : vec4(0.0);
  vec4 color1 = bc[1] > 0.0 ? texture2D(tex1, tc) : vec4(0.0);
  vec4 color2 = bc[2] > 0.0 ? texture2D(tex2, tc) : vec4(0.0);
  vec4 color3 = bc[3] > 0.0 ? texture2D(tex3, tc) : vec4(0.0);

  vec4 mixColor = color0 + color1 + color2 + color3;

  color = vec4(mixColor.rgb, mixColor.a * alpha);
}
