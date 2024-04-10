/*
Prime Engine

MIT License

Copyright (c) 2024 Sean Reid (email@seanreid.ca)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Prime/Model/ModelContent.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <srell/srell.hpp>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ModelContent::ModelContent():
scenes(nullptr),
sceneCount(0),
actions(nullptr),
actionCount(0),
textures(nullptr),
textureCount(0) {

}

ModelContent::~ModelContent() {
  PrimeSafeDeleteArray(textures);
  PrimeSafeDeleteArray(actions);
  PrimeSafeDeleteArray(scenes);
}

bool ModelContent::Load(const void* data, size_t dataSize, const json& info) {
  if(!data || dataSize == 0)
    return false;

  // Check format GLTF
  if(IsFormatGLTF(data, dataSize, info)) {
    return LoadFromGLTF(data, dataSize, info);
  }
  else if(IsFormatFBX(data, dataSize, info)) {
    return LoadFromFBX(data, dataSize, info);
  }

  return false;
}

bool ModelContent::LoadFromGLTF(const void* data, size_t dataSize, const json& info) {
  sceneCount = 1;
  scenes = new ModelContentScene[sceneCount];
  ModelContentScene& scene = scenes[0];
  scene.content = this;

  scene.baseTransform.LoadIdentity();
  scene.baseTransformScaleInv.LoadIdentity();

  sceneLookup[scene.name] = 0;

  scene.ReadModelUsingTinyGLTF(data, dataSize);

  actionCount = scene.GetAnimationCount();
  if(actionCount) {
    actions = new ModelContentAction[actionCount];

    for(size_t i = 0; i < actionCount; i++) {
      const ModelContentAnimation& sceneAnimation = scene.GetAnimation(i);
      ModelContentAction& action = actions[i];

      action.name = sceneAnimation.GetName();
      action.sceneActionName = action.name;
      action.nextPoseBlendAllowed = true;
      action.loop = true;

      actionLookup[action.name] = i;
    }
  }

  textureCount = scene.GetTextureCount();
  if(textureCount) {
    textures = new ModelContentTexture[textureCount];

    for(u32 i = 0; i < textureCount; i++) {
      ModelContentTexture& texture = textures[i];

      texture.name = string_printf("%d", i);

      textureLookup[texture.name] = i;
    }
  }

  return true;
}

bool ModelContent::LoadFromFBX(const void* data, size_t dataSize, const json& info) {
  sceneCount = 1;
  scenes = new ModelContentScene[sceneCount];
  ModelContentScene& scene = scenes[0];
  scene.content = this;

  scene.baseTransform.LoadIdentity();
  scene.baseTransformScaleInv.LoadIdentity();

  sceneLookup[scene.name] = 0;

  scene.ReadModelUsingAssimp(data, dataSize);

  actionCount = scene.GetAnimationCount();
  if(actionCount) {
    actions = new ModelContentAction[actionCount];

    for(size_t i = 0; i < actionCount; i++) {
      const ModelContentAnimation& sceneAnimation = scene.GetAnimation(i);
      ModelContentAction& action = actions[i];

      action.name = sceneAnimation.GetName();
      action.sceneActionName = action.name;
      action.nextPoseBlendAllowed = true;
      action.loop = true;

      actionLookup[action.name] = i;
    }
  }

  textureCount = scene.GetTextureCount();
  if(textureCount) {
    textures = new ModelContentTexture[textureCount];

    for(size_t i = 0; i < textureCount; i++) {
      ModelContentTexture& texture = textures[i];

      texture.name = string_printf("%d", i);

      textureLookup[texture.name] = i;
    }
  }

  for(size_t i = 0; i < sceneCount; i++) {
    for(size_t j = 0; j < scene.meshCount; j++) {
      ModelContentMesh& mesh = scene.meshes[j];
      size_t textureIndex = mesh.GetTextureIndex();
      if(textureIndex != PrimeNotFound && textureIndex < scene.GetTextureCount()) {
        Tex* tex = scene.GetTexture(textureIndex);
        mesh.SetDirectTex(tex);
      }
    }
  }

  return true;
}

size_t ModelContent::GetSceneIndexByName(const std::string& name) const {
  for(size_t i = 0; i < sceneCount; i++) {
    const ModelContentScene& scene = scenes[i];
    if(scene.name == name) {
      return i;
    }
  }

  return PrimeNotFound;
}

size_t ModelContent::GetActionIndexByName(const std::string& name) const {
  for(size_t i = 0; i < actionCount; i++) {
    const ModelContentAction& action = actions[i];
    if(action.name == name) {
      return i;
    }
  }

  return PrimeNotFound;
}

void ModelContent::ApplyTextureToMesh(ModelContentMesh& mesh) {
  if(textureCount == 0)
    return;

  for(size_t i = 0; i < textureCount; i++) {
    size_t index = textureCount - i - 1;
    ModelContentTexture& texture = textures[index];

    if(!texture.applyToMesh.empty()) {
      if(texture.applyToMesh == mesh.GetName()) {
        mesh.ReferenceTexture(texture.imagemap);
        return;
      }
      else {
        std::string re = texture.applyToMesh;
        srell::regex reg(re);

        if(srell::regex_search(mesh.GetName().c_str(), reg)) {
          mesh.ReferenceTexture(texture.imagemap);
          return;
        }
      }
    }
  }

  for(size_t i = 0; i < textureCount; i++) {
    size_t index = textureCount - i - 1;
    ModelContentTexture& texture = textures[index];

    if(!texture.applyToMesh.empty()) {
      std::string str = texture.applyToMesh;
      if(str.find_first_not_of("0123456789") == std::string::npos) {
        const char* s = texture.applyToMesh.c_str();
        if(s && isdigit(s[0])) {
          char* p;
          s64 value = strtol(texture.applyToMesh.c_str(), &p, 10);
          if(p && *p == 0 && (s64) mesh.GetMeshIndex() == value) {
            mesh.ReferenceTexture(texture.imagemap);
            return;
          }
        }
      }
    }
  }

  for(s32 i = (s32) textureCount - 1; i >= 0; i--) {
    ModelContentTexture& texture = textures[i];

    if(texture.applyToMesh.empty()) {
      // Apply value is empty, which means to apply to any mesh.
      mesh.ReferenceTexture(texture.imagemap);
      return;
    }
  }
}
