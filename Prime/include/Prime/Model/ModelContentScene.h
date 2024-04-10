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

#include <Prime/Graphics/ArrayBuffer.h>
#include <Prime/Graphics/IndexBuffer.h>
#include <Prime/Graphics/Tex.h>
#include <Prime/Model/ModelContentMesh.h>
#include <Prime/Model/ModelContentSkeleton.h>
#include <Prime/Model/ModelContentAnimation.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class ModelContentScene {
friend class Model;
friend class ModelContent;
private:

  ModelContent* content;

  std::string name;
  std::string modelPath;
  std::string skeletonRootBone;

  ModelContentMesh* meshes;
  size_t meshCount;

  ModelContentSkeleton* skeletons;
  size_t skeletonCount;

  ModelContentAnimation* animations;
  size_t animationCount;

  Mat44 baseTransform;
  Mat44 baseTransformScaleInv;

  Stack<refptr<Tex>> textures;
  bool loadTextures;

  Vec3 vertexMin;
  Vec3 vertexMax;

public:

  const std::string& GetName() const {return name;}
  const std::string& GetModelPath() const {return modelPath;}

  const ModelContentMesh& GetMesh(size_t index) const {PrimeAssert(index < meshCount, "Invalid mesh index."); return meshes[index];}
  size_t GetMeshCount() const {return meshCount;}

  const ModelContentSkeleton& GetSkeleton(size_t index) const {PrimeAssert(index < skeletonCount, "Invalid skeleton index."); return skeletons[index];}
  size_t GetSkeletonCount() const {return skeletonCount;}

  const ModelContentAnimation& GetAnimation(size_t index) const {PrimeAssert(index < animationCount, "Invalid animation index."); return animations[index];}
  size_t GetAnimationCount() const {return animationCount;}

  refptr<Tex> GetTexture(size_t index) const {PrimeAssert(index < textures.GetCount(), "Invalid texture index."); return textures[index];}
  size_t GetTextureCount() const {return textures.GetCount();}

  const Mat44& GetBaseTransform() const {return baseTransform;}
  const Mat44& GetBaseTransformScaleInv() const {return baseTransformScaleInv;}

  const Vec3& GetVertexMin() const {return vertexMin;}
  const Vec3& GetVertexMax() const {return vertexMax;}

public:

  ModelContentScene();
  ~ModelContentScene();

public:

  void SetLoadTextures(bool loadTextures);
  size_t GetMeshIndexByName(const std::string& name) const;

protected:

  void ReadModelUsingTinyGLTF(const void* data, size_t dataSize);
  void ReadModelUsingAssimp(const void* data, size_t dataSize);

  void DestroyMeshes();
  void DestroySkeletons();
  void DestroyAnimations();
  void DestroyTextures();

};

};
