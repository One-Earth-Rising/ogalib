#version 410

#define MAX_BONE_COUNT 500
#define INDEX_BONE_COUNT 2

in vec3 vPos;
in vec3 vUVBoneCount;
in vec3 vNormal;
in vec4 vBoneIndex1;
in vec4 vBoneIndex2;
in vec4 vBoneIndex3;
in vec4 vBoneIndex4;
in vec4 vBoneWeight1;
in vec4 vBoneWeight2;
in vec4 vBoneWeight3;
in vec4 vBoneWeight4;

out vec2 tc;
out vec3 normal;

uniform ShaderUniformBlock {
  mat4 mvp;
  mat4 boneTransform[MAX_BONE_COUNT];
};

void main() {
  float boneCount = vUVBoneCount[INDEX_BONE_COUNT];
  vec4 p = vec4(vPos.x, vPos.y, vPos.z, 1.0);
  vec4 point;

  if(boneCount < 1.0) {
    point = p;
  }
  else {
    int boneIndex1 = int(floor(vBoneIndex1[0] + 0.5));
    float boneWeight1 = vBoneWeight1[0];
    mat4 transform = boneTransform[boneIndex1] * boneWeight1;

    if(boneCount >= 2.0) {
      int boneIndex2 = int(floor(vBoneIndex1[1] + 0.5));
      float boneWeight2 = vBoneWeight1[1];
      transform = transform + (boneTransform[boneIndex2] * boneWeight2);
    }

    if(boneCount >= 3.0) {
      int boneIndex3 = int(floor(vBoneIndex1[2] + 0.5));
      float boneWeight3 = vBoneWeight1[2];
      transform = transform + (boneTransform[boneIndex3] * boneWeight3);
    }

    if(boneCount >= 4.0) {
      int boneIndex4 = int(floor(vBoneIndex1[3] + 0.5));
      float boneWeight4 = vBoneWeight1[3];
      transform = transform + (boneTransform[boneIndex4] * boneWeight4);
    }

    if(boneCount >= 5.0) {
      int boneIndex5 = int(floor(vBoneIndex2[0] + 0.5));
      float boneWeight5 = vBoneWeight2[0];
      transform = transform + (boneTransform[boneIndex5] * boneWeight5);
    }

    if(boneCount >= 6.0) {
      int boneIndex6 = int(floor(vBoneIndex2[1] + 0.5));
      float boneWeight6 = vBoneWeight2[1];
      transform = transform + (boneTransform[boneIndex6] * boneWeight6);
    }

    if(boneCount >= 7.0) {
      int boneIndex7 = int(floor(vBoneIndex2[2] + 0.5));
      float boneWeight7 = vBoneWeight2[2];
      transform = transform + (boneTransform[boneIndex7] * boneWeight7);
    }

    if(boneCount >= 8.0) {
      int boneIndex8 = int(floor(vBoneIndex2[3] + 0.5));
      float boneWeight8 = vBoneWeight2[3];
      transform = transform + (boneTransform[boneIndex8] * boneWeight8);
    }

    if(boneCount >= 9.0) {
      int boneIndex9 = int(floor(vBoneIndex3[0] + 0.5));
      float boneWeight9 = vBoneWeight3[0];
      transform = transform + (boneTransform[boneIndex9] * boneWeight9);
    }

    if(boneCount >= 10.0) {
      int boneIndex10 = int(floor(vBoneIndex3[1] + 0.5));
      float boneWeight10 = vBoneWeight3[1];
      transform = transform + (boneTransform[boneIndex10] * boneWeight10);
    }

    if(boneCount >= 11.0) {
      int boneIndex11 = int(floor(vBoneIndex3[2] + 0.5));
      float boneWeight11 = vBoneWeight3[2];
      transform = transform + (boneTransform[boneIndex11] * boneWeight11);
    }

    if(boneCount >= 12.0) {
      int boneIndex12 = int(floor(vBoneIndex3[3] + 0.5));
      float boneWeight12 = vBoneWeight3[3];
      transform = transform + (boneTransform[boneIndex12] * boneWeight12);
    }

    if(boneCount >= 13.0) {
      int boneIndex13 = int(floor(vBoneIndex4[0] + 0.5));
      float boneWeight13 = vBoneWeight4[0];
      transform = transform + (boneTransform[boneIndex13] * boneWeight13);
    }

    if(boneCount >= 14.0) {
      int boneIndex14 = int(floor(vBoneIndex4[1] + 0.5));
      float boneWeight14 = vBoneWeight4[1];
      transform = transform + (boneTransform[boneIndex14] * boneWeight14);
    }

    if(boneCount >= 15.0) {
      int boneIndex15 = int(floor(vBoneIndex4[2] + 0.5));
      float boneWeight15 = vBoneWeight4[2];
      transform = transform + (boneTransform[boneIndex15] * boneWeight15);
    }

    if(boneCount >= 16.0) {
      int boneIndex16 = int(floor(vBoneIndex4[3] + 0.5));
      float boneWeight16 = vBoneWeight4[3];
      transform = transform + (boneTransform[boneIndex16] * boneWeight16);
    }

    point = transform * p;
  }

  gl_Position = mvp * point;
  tc = vUVBoneCount.xy;
  normal = vNormal;
}
