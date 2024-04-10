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

#pragma once

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Content/Content.h>
#include <Prime/Model/ModelContentScene.h>

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _ModelContentAction {
  std::string name;
  std::string scene;
  std::string sceneActionName;
  std::string nextAction;
  f32 speedScale;
  f32 interruptTime;
  f32 lastPoseBlendTime;
  bool nextPoseBlendAllowed;
  bool lastPoseBlendTimeSpecified;
  bool interruptible;
  bool loop;
  bool skipRecoil;

  _ModelContentAction():
    speedScale(1.0f),
    interruptTime(0.0f),
    lastPoseBlendTime(0.0f),
    nextPoseBlendAllowed(false),
    lastPoseBlendTimeSpecified(false),
    interruptible(false),
    loop(false),
    skipRecoil(false) {

  }
} ModelContentAction;

typedef struct _ModelContentTexture {
  std::string name;
  std::string applyToMesh;
  std::string imagemap;
  bool invertY;

  _ModelContentTexture():
    invertY(false) {

  }
} ModelContentTexture;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class ModelContent: public Content {
friend class ModelContentScene;
private:

  ModelContentScene* scenes;
  std::unordered_map<std::string, size_t> sceneLookup;
  size_t sceneCount;

  ModelContentAction* actions;
  std::unordered_map<std::string, size_t> actionLookup;
  size_t actionCount;

  ModelContentTexture* textures;
  std::unordered_map<std::string, size_t> textureLookup;
  size_t textureCount;

public:

  const ModelContentScene& GetScene(size_t index) const {PrimeAssert(index < sceneCount, "Invalid scene index."); return scenes[index];}
  const ModelContentScene* GetScenes() const {return scenes;}
  size_t GetSceneCount() const {return sceneCount;}

  const ModelContentAction& GetAction(size_t index) const {PrimeAssert(index < actionCount, "Invalid action index."); return actions[index];}
  const ModelContentAction* GetActions() const {return actions;}
  size_t GetActionCount() const {return actionCount;}

  const ModelContentTexture& GetTexture(size_t index) const {PrimeAssert(index < textureCount, "Invalid texture index."); return textures[index];}
  const ModelContentTexture* GetTextures() const {return textures;}
  size_t GetTextureCount() const {return textureCount;}

public:

  ModelContent();
  ~ModelContent();

public:

  bool Load(const void* data, size_t dataSize, const json& info) override;
  virtual bool LoadFromGLTF(const void* data, size_t dataSize, const json& info);
  virtual bool LoadFromFBX(const void* data, size_t dataSize, const json& info);

  virtual size_t GetSceneIndexByName(const std::string& name) const;
  virtual size_t GetActionIndexByName(const std::string& name) const;

  virtual void ApplyTextureToMesh(ModelContentMesh& mesh);

};

};
