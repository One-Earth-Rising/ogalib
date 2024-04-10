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
#include <Prime/Enum/SkeletonPoseInterpolateAnchor.h>
#include <Prime/Enum/CollisionType.h>
#include <Prime/Enum/CollisionTypeParam.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _SkeletonContentPoseBone {
  std::string name;
  f32 angle;
  f32 scaleX;
  f32 scaleY;
  f32 x;
  f32 y;
  f32 depth;
  f32 alpha;
  f32 alphaInterpolate;
  size_t boneLookupIndex;
  SkeletonPoseInterpolateAnchor alphaInterpolateAnchor;

  _SkeletonContentPoseBone():
    angle(0.0f),
    scaleX(0.0f),
    scaleY(0.0f),
    x(0.0f),
    y(0.0f),
    depth(0.0f),
    alpha(0.0f),
    alphaInterpolate(0.0f),
    boneLookupIndex(0),
    alphaInterpolateAnchor(SkeletonPoseInterpolateAnchor()) {

  }
} SkeletonContentPoseBone;

typedef struct _SkeletonContentPoseBoneTransform {
  std::string name;
  f32 x;
  f32 y;
  f32 dx;
  f32 dy;
  f32 angle;
  f32 scaleX;
  f32 scaleY;
  f32 alpha;

  _SkeletonContentPoseBoneTransform():
    x(0.0f),
    y(0.0f),
    dx(0.0f),
    dy(0.0f),
    angle(0.0f),
    scaleX(0.0f),
    scaleY(0.0f),
    alpha(0.0f) {

  }
} SkeletonContentPoseBoneTransform;

typedef struct _SkeletonContentActionKeyFramePieceActionMapping {
  std::string piece;
  std::string action;

  _SkeletonContentActionKeyFramePieceActionMapping() {

  }
} SkeletonContentActionKeyFramePieceActionMapping;

typedef struct _SkeletonContentActionKeyFrame {
  std::string pose;
  size_t len;
  size_t poseIndex;
  size_t pieceActionMappingCount;
  SkeletonContentActionKeyFramePieceActionMapping* pieceActionMappings;

  _SkeletonContentActionKeyFrame():
    len(0),
    poseIndex(0),
    pieceActionMappingCount(0),
    pieceActionMappings(NULL) {

  }
} SkeletonContentActionKeyFrame;

typedef struct _SkeletonContentBone {
  std::string name;
  std::string parent;
  size_t parentIndex;
  f32 size;
  f32 depth;
  bool tip;
  bool cancelActionBlend;

  _SkeletonContentBone():
    parentIndex(PrimeNotFound),
    size(0.0f),
    depth(0.0f),
    tip(false),
    cancelActionBlend(false) {

  }
} SkeletonContentBone;

typedef struct _SkeletonContentPose {
  std::string name;
  SkeletonContentPoseBone* bones;
  SkeletonContentPoseBoneTransform* boneTransforms;

  _SkeletonContentPose():
    bones(NULL),
    boneTransforms(NULL) {

  }
} SkeletonContentPose;

typedef struct _SkeletonContentAction {
  std::string name;
  std::string nextAction;
  f32 x;
  f32 y;
  f32 z;
  f32 lastPoseBlendTime;
  f32 interruptTime;
  size_t keyFrameCount;
  SkeletonContentActionKeyFrame* keyFrames;
  bool loop;
  bool lastPoseBlendTimeSpecified;
  bool nextPoseBlendAllowed;
  bool interruptible;
  bool skipRecoil;

  _SkeletonContentAction():
    x(0.0f),
    y(0.0f),
    z(0.0f),
    lastPoseBlendTime(0.0f),
    interruptTime(0.0f),
    keyFrameCount(0),
    keyFrames(NULL),
    loop(false),
    lastPoseBlendTimeSpecified(false),
    nextPoseBlendAllowed(false),
    interruptible(false),
    skipRecoil(false) {

  }
} SkeletonContentAction;

class SkeletonContent: public Content {
private:

  std::string skinset;

  f32 fps;

  SkeletonContentBone* bones;
  size_t boneCount;

  SkeletonContentPose* poses;
  size_t poseCount;

  SkeletonContentAction* actions;
  size_t actionCount;

  size_t* orderedBoneHierarchy;
  size_t* orderedBoneHierarchyRev;

public:

  const std::string& GetSkinset() const {return skinset;}

public:

  SkeletonContent();
  ~SkeletonContent();

public:

  f32 GetFPS() const {return fps;}

  const SkeletonContentBone& GetBone(size_t index) const {PrimeAssert(index < boneCount, "Invalid bone index."); return bones[index];}
  const SkeletonContentBone* GetBones() const {return bones;}
  size_t GetBoneCount() const {return boneCount;}

  const SkeletonContentPose& GetPose(size_t index) const {PrimeAssert(index < poseCount, "Invalid pose index."); return poses[index];}
  const SkeletonContentPose* GetPoses() const {return poses;}
  size_t GetPoseCount() const {return poseCount;}

  const SkeletonContentAction& GetAction(size_t index) const {PrimeAssert(index < actionCount, "Invalid action index."); return actions[index];}
  const SkeletonContentAction* GetActions() const {return actions;}
  size_t GetActionCount() const {return actionCount;}

public:

  bool Load(const json& data, const json& info) override;

  virtual const SkeletonContentBone* FindBone(const std::string& name) const;
  virtual const SkeletonContentPose* FindPose(const std::string& name) const;
  virtual const SkeletonContentPoseBone* FindPoseBone(const SkeletonContentPose* pose, const std::string& name) const;
  virtual bool IsBoneDescendant(size_t boneIndex, size_t ancestorIndex) const;

  virtual size_t GetBoneIndex(const std::string& name) const;
  virtual size_t GetPoseIndex(const std::string& name) const;
  virtual size_t GetPoseBoneIndex(const SkeletonContentPose* pose, const std::string& name) const;

  const size_t GetBoneIndexFromOrderedHierarchy(size_t index, bool rev = false) const;

};

};
