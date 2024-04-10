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

#include <Prime/Skeleton/SkeletonContent.h>
#include <Prime/Types/Set.h>

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _SkeletonPoseBone {
  f32 x, y;
  f32 x2, y2;
  f32 dx, dy;
  f32 angle;
  f32 angleParent;
  f32 scaleX, scaleY;
  f32 alpha;
  f32 alphaInterpolate;
  f32 poseAngle;
  f32 poseScaleX, poseScaleY;
  f32 poseX, poseY;
  SkeletonPoseInterpolateAnchor alphaInterpolateAnchor;
} SkeletonPoseBone;

typedef struct _SkeletonBoneOverride {
  f32 x, y;
  f32 angle;
  f32 scaleX, scaleY;
  bool overrideTranslation;
  bool overrideAngle;
  bool overrideAngleAbsolute;
  bool overrideScale;
} SkeletonBoneOverride;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class SkeletonPose {
private:

  refptr<SkeletonContent> content;
  SkeletonPoseBone* bones;
  SkeletonBoneOverride* boneOverrides;
  size_t boneCount;

public:

  refptr<SkeletonContent> GetSkeletonContent() const {return content;}
  bool HasContent() const {return (bool) content;}

  size_t GetBoneCount() const {return boneCount;}

public:

  SkeletonPose();
  ~SkeletonPose();

public:

  void SetContent(refptr<SkeletonContent> content);

  void SetBoneOverrides(SkeletonBoneOverride* boneOverrides);

  void Copy(const SkeletonContentPose& pose, const SkeletonPoseBone* rootBone = nullptr);
  void Copy(const SkeletonPose& pose);
  void CopyPoseFromContent(const char* name);

  void Interpolate(const SkeletonPose& pose1, const SkeletonPose& pose2, f32 weight, const SkeletonPoseBone* rootBone = nullptr, const Set<std::string>* boneCancelInterpolate = nullptr);

  const SkeletonPoseBone* GetBone(size_t index) const;

  bool GetBonePointPos(const std::string& name, const Vec2& point, Vec2& pos) const;

private:

  void InitPoseBone(SkeletonPoseBone& bone);
  static void RotatePoint(f32& resultX, f32& resultY, f32 x, f32 y, f32 angle, f32 aboutX = 0.0f, f32 aboutY = 0.0f);
  static void ScalePoint(f32& resultX, f32& resultY, f32 x, f32 y, f32 scaleX, f32 scaleY, f32 originX = 0.0f, f32 originY = 0.0f);

};

};
