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

#include <Prime/Interface/IProcessable.h>
#include <Prime/Interface/IMeasurable.h>
#include <Prime/Model/ModelContent.h>
#include <Prime/Model/ModelPose.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Model: public RefObject, public IProcessable, public IMeasurable {
private:

  refptr<ModelContent> content;

  ModelPose currActionPose1;
  ModelPose currActionPose2;
  ModelPose currActionPoseI;
  ModelPose lastActionPose;
  ModelPose lastActionPoseTemp;
  f32 lastActionPoseBlendCtr;
  f32 lastActionPoseBlendTime;
  bool nextActionPoseBlendCanceled;
  const ModelContentSkeletonActionKeyFrame* knownActionKeyFrame1;
  const ModelContentSkeletonActionKeyFrame* knownActionKeyFrame2;
  f32 knownPoseBlendWeight;
  ModelBoneOverride* boneOverrides;
  Set<std::string> boneCancelActionBlend;
  Set<std::string> boneCancelNextActionBlend;

  std::string actionSceneName;
  bool actionSceneNameKnown;
  size_t actionIndex;
  bool actionChanged;
  f32 actionTimeScale;
  f32 actionCtr;
  f32 actionLoopedCtr;
  f32 actionLen;
  s32 actionLoopCount;
  bool actionPlayed;
  bool actionReverse;
  Dictionary<std::string, std::string> mappedActionName;

  Mat44** activeBoneTransforms;
  Mat44** boneTransforms;
  size_t activeMeshCount;
  size_t activeBoneCount;
  size_t totalBoneCount;

  Dictionary<std::string, refptr<Tex>> textureOverrides;
  bool textureFilteringEnabled;

  Dictionary<std::string, Mat44> meshTransforms;

  f32 uniformBaseScale;
  bool uniformBaseScaleCached;

  Vec3 vertexMin;
  Vec3 vertexMax;

public:

  refptr<ModelContent> GetModelContent() const {return content;}
  bool HasContent() const {return (bool) content;}

  const Vec3& GetVertexMin() const {return vertexMin;}
  const Vec3& GetVertexMax() const {return vertexMax;}

public:

  Model();
  ~Model();

public:

  virtual void SetContent(Content* content);
  virtual void SetContent(ModelContent* content);

  void Calc(f32 dt) override;
  void Draw() override;

  f32 GetUniformSize() const override;

  ////////////////////////////////////////
  // Actions
  ////////////////////////////////////////

  void SetAction(const std::string& name);
  bool SetActionIfNew(const std::string& name);
  void SetActionTime(f32 time);
  void SetActionT(f32 t);
  void SetActionTimeScale(f32 scale);
  void SetActionReverse(bool reverse);
  bool DoesActionExist(const std::string& name);
  bool IsInAction(const std::string& name);
  size_t GetActionIndex() const;
  const std::string& GetActionName() const;
  f32 GetActionLen() const;
  f32 GetActionTime() const;
  f32 GetActionLoopedTime() const;
  f32 GetActionT() const;

  virtual void SetActionByIndex(size_t index);
  virtual void ResetActionChanged();
  virtual bool HasActionChanged();
  virtual void CancelLastActionBlend();
  virtual void MapActionName(const std::string& name, const std::string& mappedToName);
  const ModelContentScene* GetActiveScene() const;
  const ModelContentSkeleton* GetActiveSkeleton(const ModelContentScene** associatedScene = NULL) const;
  const ModelContentScene* GetSceneByActionIndex(const ModelContent& content, size_t actionIndex) const;
  const ModelContentSkeleton* GetSkeletonByActionIndex(const ModelContent& content, size_t actionIndex) const;

  virtual void CalcPose(f32 dt);

  virtual void ApplyTextureOverride(const std::string& meshName, refptr<Tex> tex);
  virtual void RemoveTextureOverride(const std::string& meshName);
  virtual void RemoveAllTextureOverrides();
  virtual void SetTextureFilteringEnabled(bool enabled);

  virtual void SetMeshTransform(const std::string& name, const Mat44& mat);
  virtual void ClearMeshTransform(const std::string& name);
  virtual void DrawMesh(const ModelContentMesh& mesh, size_t meshIndex);

  virtual const Mat44* GetActiveBoneTransform(size_t meshIndex, size_t activePoseBoneIndex) const;
  virtual const Mat44* GetBoneTransform(size_t meshIndex, size_t boneIndex) const;

  virtual f32 GetUniformBaseScale(bool cached = true);

protected:

  void DiscardAction();
  void GetActionKeyFrames(const ModelContentSkeleton& skeleton, const ModelContentSkeletonAction& skeletonAction, const ModelContentSkeletonActionKeyFrame** keyFrame1, const ModelContentSkeletonActionKeyFrame** keyFrame2, f32* weight);

  void UpdateBoneTransformsForPoses(const ModelContent& content, const ModelContentSkeleton& skeleton, size_t meshIndex, size_t boneIndex, Mat44 transformation, const ModelContentSkeletonPose* pose1, const ModelContentSkeletonPose* pose2 = NULL, f32 t = 0.0f);
  void UpdateBoneTransformsForModelPose(const ModelContent& content, const ModelContentSkeleton& skeleton, size_t meshIndex, size_t boneIndex, Mat44 transformation, const ModelPose& pose);

  void DestroyBoneTransforms();

};

};
