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

#include <Prime/Types/Stack.h>
#include <Prime/Types/Dictionary.h>
#include <Prime/Model/ModelContentSkeletonBone.h>
#include <Prime/Model/ModelContentSkeletonPose.h>
#include <Prime/Model/ModelContentSkeletonAction.h>
#include <assimp/scene.h>

#define TINYGLTF_USE_RAPIDJSON
#define TINYGLTF_NO_FS
#define TINYGLTF_USE_RAPIDJSON_CRTALLOCATOR
#include <tinygltf/tiny_gltf.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class ModelContentSkeleton {
friend class Model;
friend class ModelContent;
friend class ModelContentScene;
private:

  ModelContentSkeletonBone* bones;
  size_t boneCount;
  Dictionary<std::string, size_t> boneLookupIndexByName;
  Dictionary<size_t, std::string> boneLookupNameByIndex;
  size_t rootBoneIndex;
  size_t actionPoseBoneCount;

  ModelContentSkeletonPose* poses;
  size_t poseCount;

  ModelContentSkeletonAction* actions;
  Dictionary<std::string, size_t> lookupActionIndexByName;
  size_t actionCount;

  Mat44 rootBoneTransform;
  Mat44 rootBoneTransformInv;

  u32 signature;

public:

  const ModelContentSkeletonBone& GetBone(size_t index) const {PrimeAssert(index < boneCount, "Invalid bone index."); return bones[index];}
  size_t GetBoneCount() const {return boneCount;}
  size_t GetRootBoneIndex() const {return rootBoneIndex;}
  size_t GetActionPoseBoneCount() const {return actionPoseBoneCount;}

  const ModelContentSkeletonPose& GetPose(size_t index) const {PrimeAssert(index < poseCount, "Invalid pose index."); return poses[index];}
  size_t GetPoseCount() const {return poseCount;}

  const ModelContentSkeletonAction& GetAction(size_t index) const {PrimeAssert(index < actionCount, "Invalid action index."); return actions[index];}
  size_t GetActionCount() const {return actionCount;}

  const Mat44& GetRootBoneTransform() const {return rootBoneTransform;}
  const Mat44& GetRootBoneTransformInv() const {return rootBoneTransformInv;}

  size_t GetSignature() const {return signature;}

public:

  ModelContentSkeleton();
  ~ModelContentSkeleton();

public:

  void Load(const aiScene& scene);
  void Load(const tinygltf::Model& model);

  size_t GetBoneIndexByName(const std::string& name) const;
  size_t GetActionPoseBoneIndexByName(const std::string& name) const;
  void ApplyBoneAffectingVertices(const std::string& name);
  const ModelContentSkeletonAction* GetActionByName(const std::string& name) const;

protected:

  const aiNode* FindRootBone(const aiNode* node);
  void TraverseBoneHierarchyForNames(const aiNode* node);
  void TraverseBoneHierarchy(const aiNode* node);

  int FindTinyGLTFNode(const tinygltf::Model& model, const std::string& name);
  int FindTinyGLTFMeshIndexByTinyGLTFNodeIndex(const tinygltf::Model& model, int nodeIndex);
  size_t FindBoneIndexByTinyGLTFJointIndex(const tinygltf::Model& model, size_t jointIndex);
  size_t FindBoneIndexByTinyGLTFMeshIndex(const tinygltf::Model& model, size_t meshIndex);
  std::string GenerateBoneNameFromTinyGLTFNodeIndex(const tinygltf::Model& model, size_t nodeIndex);
  void TraverseBoneHierarchy(const tinygltf::Model& model, int nodeIndex, Mat44 transformation, Mat44 parentTransformation, Dictionary<std::string, Mat44>& skinInverseBindTransforms);

  void EnsureKeyFramePose(ModelContentSkeletonActionKeyFrame& keyFrame, size_t keyFrameIndex, ModelContentSkeletonAction& action, Stack<ModelContentSkeletonPose>& createdPoses);
  void EnsureKeyFrameTransformations(ModelContentSkeletonActionKeyFrame& keyFrame, size_t keyFrameIndex, ModelContentSkeletonAction& action, Stack<ModelContentSkeletonPose>& createdPoses);

  void DestroyBones();
  void DestroyPoses();
  void DestroyActions();

};

static __inline void CopyMatrix(Mat44& dest, const aiMatrix4x4& src) {
  dest.e11 = src.a1;
  dest.e21 = src.b1;
  dest.e31 = src.c1;
  dest.e41 = src.d1;

  dest.e12 = src.a2;
  dest.e22 = src.b2;
  dest.e32 = src.c2;
  dest.e42 = src.d2;

  dest.e13 = src.a3;
  dest.e23 = src.b3;
  dest.e33 = src.c3;
  dest.e43 = src.d3;

  dest.e14 = src.a4;
  dest.e24 = src.b4;
  dest.e34 = src.c4;
  dest.e44 = src.d4;
}

static __inline void CopyMatrix(Mat44& dest, const std::vector<double>& src) {
  if(src.size() == 16) {
    dest.e11 = (f32) src[0];
    dest.e21 = (f32) src[1];
    dest.e31 = (f32) src[2];
    dest.e41 = (f32) src[3];

    dest.e12 = (f32) src[4];
    dest.e22 = (f32) src[5];
    dest.e32 = (f32) src[6];
    dest.e42 = (f32) src[7];

    dest.e13 = (f32) src[8];
    dest.e23 = (f32) src[9];
    dest.e33 = (f32) src[10];
    dest.e43 = (f32) src[11];

    dest.e14 = (f32) src[12];
    dest.e24 = (f32) src[13];
    dest.e34 = (f32) src[14];
    dest.e44 = (f32) src[15];
  }
}

};
