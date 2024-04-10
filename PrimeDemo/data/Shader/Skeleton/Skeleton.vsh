#version 410

#define MAX_BONE_COUNT 200
#define BT1_ALPHA   2
#define BT2_ANGLE   2
#define BT_BONE     2
#define BT_TEXTURE  3

in vec2 vPos;
in vec4 vUVBoneTexture;

out vec2 tc;
out vec4 bc;
out float alpha;

uniform ShaderUniformBlock {
  mat4 mvp;
  vec3 boneTransform1[MAX_BONE_COUNT];
  vec3 boneTransform2[MAX_BONE_COUNT];
  mat4 boneTransform[MAX_BONE_COUNT];
};

void main() {
  vec4 p = vec4(vPos, 0.0, 1.0);
  int bone = int(floor(vUVBoneTexture[BT_BONE] + 0.5));
  int texture = int(floor(vUVBoneTexture[BT_TEXTURE] + 0.5));
  vec3 bt1 = boneTransform1[bone];
  vec3 bt2 = boneTransform2[bone];

  mat4 scale = mat4(
    vec4(bt2.x, 0.0, 0.0, 0.0),
    vec4(0.0, bt2.y, 0.0, 0.0),
    vec4(0.0, 0.0, 1.0, 0.0),
    vec4(0.0, 0.0, 0.0, 1.0));

  float boneAngle = bt2[BT2_ANGLE];
  float boneAngleCos = cos(boneAngle);
  float boneAngleSin = sin(boneAngle);

  mat4 rotate = mat4(
    vec4(boneAngleCos, boneAngleSin, 0.0, 0.0),
    vec4(-boneAngleSin, boneAngleCos, 0.0, 0.0),
    vec4(0.0, 0.0, 1.0, 0.0),
    vec4(0.0, 0.0, 0.0, 1.0));

  mat4 translate = mat4(
    vec4(1.0, 0.0, 0.0, 0.0),
    vec4(0.0, 1.0, 0.0, 0.0),
    vec4(0.0, 0.0, 1.0, 0.0),
    vec4(bt1.x, bt1.y, 0.0, 1.0));

  gl_Position = mvp * boneTransform[bone] * translate * rotate * scale * p;
  
  tc = vUVBoneTexture.xy;
  
  bc = vec4(
    texture == 0 ? 1.0 : 0.0,
    texture == 1 ? 1.0 : 0.0,
    texture == 2 ? 1.0 : 0.0,
    texture == 3 ? 1.0 : 0.0);
  
  alpha = bt1[BT1_ALPHA];
}
