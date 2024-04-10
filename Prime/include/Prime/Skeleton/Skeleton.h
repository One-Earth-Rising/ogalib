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
#include <Prime/Skeleton/SkeletonContent.h>
#include <Prime/Skeleton/SkeletonPose.h>
#include <Prime/Skinset/Skinset.h>
#include <Prime/Graphics/ArrayBuffer.h>
#include <Prime/Graphics/IndexBuffer.h>
#include <Prime/Graphics/DeviceProgram.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PRIME_SKELETON_PROGRAM_BONE_COUNT       200
#define PRIME_SKELETON_PROGRAM_TEX_UNIT_COUNT   4

////////////////////////////////////////////////////////////////////////////////
// Enums
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef enum {
  SkeletonProcessingModeNone = 0,
  SkeletonProcessingModeShaderWithPoseVariables,
  SkeletonProcessingModeShaderWithPoseVariablesInTree,
  SkeletonProcessingModeInactive,
} SkeletonProcessingMode;

};

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Skeleton;
class ImagemapContent;

typedef struct _SkeletonBonePoseData {
  f32 x, y;
  f32 x2, y2;
  f32 dx, dy;
  f32 angle;
  f32 angleParent;
  f32 scaleX, scaleY;
  f32 alpha;
} SkeletonBonePoseData;

typedef struct _SkeletonPieceSignature {
  refptr<ImagemapContent> imc;
  size_t rectIndex;
  size_t vertexIndexStart;
  bool hflip;
  bool vflip;
} SkeletonPieceSignature;

typedef struct _SkeletonGetBufferBoneTransformsParam {
  Skeleton* mainSkeleton;
  Dictionary<Skeleton*, size_t*>* boneLookup;
  Mat44* boneRootTransforms;
  SkeletonBonePoseData* bpd;
  size_t bpdCount;
} SkeletonGetBufferBoneTransformsParam;

typedef struct _SkeletonAddBufferSkeletonParam {
  Skeleton* mainSkeleton;
  ArrayBuffer* ab;
  size_t boneIndex;
  size_t updatePieceCount;
  Dictionary<refptr<ImagemapContent>, size_t>* texLookup;
  refptr<ImagemapContent>* texList;
  Dictionary<Skeleton*, size_t*>* skeletonBoneLookup;
  SkeletonPieceSignature* pieceSignatures;
  Stack<size_t>** bonePieceSignatureIndices;
  bool pieceSignaturesOutdated;
} SkeletonAddBufferSkeletonParam;

typedef struct _SkeletonAddBufferSkeletonDepthBonesParam {
  IndexBuffer* ib;
  Dictionary<Skeleton*, size_t*>* skeletonBoneLookup;
  SkeletonPieceSignature* pieceSignatures;
  Stack<size_t>** bonePieceSignatureIndices;
  u16 pieceIndex;
} SkeletonAddBufferSkeletonDepthBonesParam;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Skeleton;

class SkeletonDepthSortItem {
friend class Skeleton;
private:

  size_t boneIndex;
  f32 depth;

public:

  SkeletonDepthSortItem(size_t boneIndex = 0, f32 depth = 0.0f): boneIndex(boneIndex), depth(depth) {}

  SkeletonDepthSortItem(const SkeletonDepthSortItem& other) {
    (void) operator=(other);
  }

  SkeletonDepthSortItem& operator=(const SkeletonDepthSortItem& other) {
    boneIndex = other.boneIndex;
    depth = other.depth;
    return *this;
  }

  bool operator==(const SkeletonDepthSortItem& other) const {
    return depth == other.depth;
  }

  bool operator<(const SkeletonDepthSortItem& other) const {
    if(depth < other.depth)
      return true;
    else if(depth > other.depth)
      return false;

    return boneIndex < other.boneIndex;
  }

  bool operator>(const SkeletonDepthSortItem& other) const {
    if(depth > other.depth)
      return true;
    else if(depth < other.depth)
      return false;

    return boneIndex > other.boneIndex;
  }

};

typedef Stack<SkeletonDepthSortItem> SkeletonDepthSortItemStack;

class Skeleton: public RefObject, public IProcessable, public IMeasurable {
private:

  refptr<SkeletonContent> content;

  refptr<Skinset> skinset;

  SkeletonDepthSortItemStack depthSortedItems;
  size_t* lastDepthSortedBoneIndices;
  bool boneDepthUpdated;

  Dictionary<refptr<Skinset>, SkinsetContentAffixPieceLookupStack**> boneSkinsetAffixes;
  size_t boneSkinsetAffixesBoneCount;
  Dictionary<std::string, refptr<Skinset>>* additionalSkinsets;
  std::string* additionalSkinsetActiveBones;

  SkeletonPose currActionPose1;
  SkeletonPose currActionPose2;
  SkeletonPose currActionPoseI;
  SkeletonPose lastActionPose;
  SkeletonPose lastActionPoseTemp;
  f32 lastActionPoseBlendCtr;
  f32 lastActionPoseBlendTime;
  bool nextActionPoseBlendCanceled;
  const SkeletonContentPose* knownActionPose1;
  const SkeletonContentPose* knownActionPose2;
  f32 knownPoseBlendWeight;
  const SkeletonContentActionKeyFrame* knownActionKeyFrame;
  SkeletonBoneOverride* boneOverrides;
  Set<std::string> boneCancelActionBlend;
  Set<std::string> boneCancelNextActionBlend;

  ////////////////////////////////////////
  // Actions
  ////////////////////////////////////////

  size_t actionIndex;
  bool actionChanged;
  f32 actionTimeScale;
  f32 actionCtr;
  f32 actionLen;
  s32 actionLoopCount;
  bool actionPlayed;
  bool actionReverse;

  std::string nameBuffer;

  size_t cacheBoneTransformsFrame;

  f32 calcPoseTime;

  SkeletonProcessingMode processingMode;
  
  Mat44 localMat;

  refptr<ArrayBuffer> ab;
  refptr<IndexBuffer> ib;
  size_t thisBoneCount;
  size_t thisPieceCount;
  size_t totalBoneCount;
  size_t totalPieceCount;
  size_t totalTexCount;
  Dictionary<refptr<ImagemapContent>, size_t> bufferTexLookup;
  refptr<ImagemapContent> bufferTexList[PRIME_SKELETON_PROGRAM_TEX_UNIT_COUNT];
  Dictionary<Skeleton*, size_t*>* skeletonBoneLookup;
  Mat44* skeletonBoneRootTransforms;
  SkeletonPieceSignature* pieceSignatures;
  Stack<size_t>** bonePieceSignatureIndices;
  size_t bonePieceSignatureIndicesCount;
  bool pieceSignaturesOutdated;
  bool shaderDataReady;
  bool updateVertexSpan;

  f32 programData1[PRIME_SKELETON_PROGRAM_BONE_COUNT * 3];
  f32 programData2[PRIME_SKELETON_PROGRAM_BONE_COUNT * 3];
  size_t programDataBoneCount;

  Vec3 vertexMin;
  Vec3 vertexMax;

public:

  refptr<SkeletonContent> GetSkeletonContent() const {return content;}
  bool HasContent() const {return (bool) content;}

  const Vec3& GetVertexMin() const {return vertexMin;}
  const Vec3& GetVertexMax() const {return vertexMax;}

public:

  Skeleton();
  ~Skeleton();

public:

  virtual void SetContent(Content* content);
  virtual void SetContent(SkeletonContent* content);

  virtual void SetSkinset(refptr<Skinset> skinset);
  virtual refptr<Skinset> GetSkinset() const;
  virtual bool HasSkinset() const;
  virtual void SetAdditionalSkinset(const std::string& name, refptr<Skinset> skinset);
  virtual void SetAdditionalSkinset(const std::string& name, refptr<Skinset> skinset, const Stack<std::string>& activeBones);
  virtual bool HasAdditionalSkinsets() const;
  virtual void DestroyAdditionalSkinset(const std::string& name);
  virtual void DestroyAdditionalSkinsets();
  virtual refptr<Skinset> GetSkinsetForBone(size_t boneIndex);
  virtual void SetDefaultSkinset();

  ////////////////////////////////////////
  // Processing
  ////////////////////////////////////////

  void Calc(f32 dt) override;
  virtual void CalcPose(f32 dt);

  virtual bool GetBoneTransform(const std::string& name, Mat44& mat);
  virtual void CacheBoneTransforms(bool force = false);

  f32 GetUniformSize() const override;

  ////////////////////////////////////////
  // Actions
  ////////////////////////////////////////
  
  virtual void SetAction(const std::string& name);
  virtual bool SetActionIfNew(const std::string& name);
  virtual void SetActionTime(f32 time);
  virtual void SetActionT(f32 t);
  virtual void SetActionTimeScale(f32 scale);
  virtual void SetActionReverse(bool reverse);
  virtual bool DoesActionExist(const std::string& name);
  virtual bool IsInAction(const std::string& name);
  size_t GetActionIndex() const;
  virtual const std::string& GetActionName() const;
  virtual f32 GetActionLen() const;
  virtual f32 GetActionTime() const;
  virtual f32 GetActionT() const;

  virtual void SetActionByIndex(size_t index);
  virtual void ResetActionChanged();
  virtual bool HasActionChanged();
  virtual void CancelLastActionBlend();
  virtual void ProcessBoneCancelActionBlend(const SkeletonContentActionKeyFrame* actionKeyFrame);
  virtual void PerformBoneDepthSort();
  virtual void SetBoneOverrideTranslation(const char* bone, f32 x, f32 y);
  virtual void SetBoneOverrideAngle(const char* bone, f32 angle, bool absolute = false);
  virtual void SetBoneOverrideScale(const char* bone, f32 scaleX, f32 scaleY);
  virtual void ClearBoneOverrideTranslation(const char* bone);
  virtual void ClearBoneOverrideAngle(const char* bone);
  virtual void ClearBoneOverrideScale(const char* bone);

  ////////////////////////////////////////
  // Drawing
  ////////////////////////////////////////

  void Draw() override;

  virtual void SetLocalMat(const Mat44& mat);
  virtual const Mat44& GetLocalMat() const;

  virtual void SetProcessingMode(SkeletonProcessingMode mode);

  virtual bool IsProcessingModeUsingShader() const;
  virtual size_t GetShaderTexCount() const;
  
  virtual size_t GetTreeBoneCount() const;
  virtual size_t GetSkinsetTreePieceCount() const;

  virtual SkeletonPose& GetCurrentPose();

private:

  void DiscardAction();

  void InitBonePoseData(SkeletonBonePoseData& bpd);

  void GetActionFramePoses(const SkeletonContentPose** pose1, const SkeletonContentPose** pose2, f32* weight, const SkeletonContentActionKeyFrame** actionKeyFrame = nullptr);

  void PerformBoneDepthSort(const SkeletonContentPose* pose1, const SkeletonContentPose* pose2 = nullptr, f32 weight = 0.0f);

  SkeletonBoneOverride* GetBoneOverride(const char* bone, bool create = false);

  void DestroyAllBoneSkinsetAffixes();
  void DestroyBoneSkinsetAffixes(refptr<Skinset> skinset);
  void CreateBoneSkinsetAffixes(refptr<Skinset> skinset);
  const SkinsetContentAffixPieceLookupStack& GetBoneSkinsetAffixes(refptr<Skinset> skinset, size_t boneIndex);
  void UpdateReferencedBoneSkinsetAffixes();

  void DestroyPieceSignatures();
  void CreatePieceSignatures();
  static bool UpdatePieceSignature(SkeletonPieceSignature* signature, refptr<ImagemapContent> imc, size_t rectIndex, bool hflip = false, bool vflip = false);

  void UpdateBufferPieces();
  void UpdateBufferPose();
  void GetBufferBoneTransforms(SkeletonGetBufferBoneTransformsParam& param, const Mat44& rootTransform, f32 alpha = 1.0f);
  void AddBufferSkeleton(SkeletonAddBufferSkeletonParam& param, size_t parentBoneIndex = PrimeNotFound);
  void AddBufferSkeletonDepthBones(SkeletonAddBufferSkeletonDepthBonesParam& param);
  size_t LookupTexIndex(refptr<ImagemapContent> imc, Dictionary<refptr<ImagemapContent>, size_t>* lookup, refptr<ImagemapContent>* list);
  void SetProcessingModeInTree(SkeletonProcessingMode mode, size_t minDepth = 1, size_t depth = 0);
  bool ShouldProcessWithProgram() const;

  void UpdateProgramBoneData(DeviceProgram* deviceProgram);

};

};
