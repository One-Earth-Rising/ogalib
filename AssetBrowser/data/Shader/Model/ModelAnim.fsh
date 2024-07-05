#version 410

#define MAX_BONE_COUNT 500

in vec2 tc;
in vec3 normal;

out vec4 color;

uniform ShaderUniformBlock {
  mat4 mvp;
  vec3 lightDir;
  mat4 boneTransform[MAX_BONE_COUNT];
};

uniform sampler2D tex;

void main() {
  float cosAngle = dot(normal, -lightDir);
  float brightness = max(cosAngle, 0.0);
  vec4 c = texture2D(tex, tc);

  color = vec4(c.rgb * brightness, c.a);
}
