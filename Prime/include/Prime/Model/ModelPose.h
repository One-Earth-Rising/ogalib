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

#include <Prime/Model/ModelContent.h>
#include <Prime/Model/ModelContentSkeletonPose.h>
#include <Prime/Types/Set.h>

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _ModelPoseBone {
  Vec3 translation;
  Quat rotation;
  Vec3 scaling;
  bool poseValid;

  _ModelPoseBone(): translation(Vec3(0.0f, 0.0f, 0.0f)), rotation(Quat(0.0f, 0.0f, 0.0f, 1.0f)), scaling(Vec3(1.0f, 1.0f, 1.0f)), poseValid(false) {}
  _ModelPoseBone(const struct _ModelPoseBone& other): translation(other.translation), rotation(other.rotation), scaling(other.scaling), poseValid(other.poseValid) {}

} ModelPoseBone;

typedef struct _ModelBoneOverride {
  Vec3 translation;
  Vec3 scaling;
  Quat rotation;
  bool overrideTranslation;
  bool overrideRotation;
  bool overrideScaling;

  _ModelBoneOverride(): translation(Vec3(0.0f, 0.0f, 0.0f)), rotation(Quat(0.0f, 0.0f, 0.0f, 1.0f)), scaling(Vec3(1.0f, 1.0f, 1.0f)), overrideTranslation(false), overrideRotation(false), overrideScaling(false) {}
  _ModelBoneOverride(const struct _ModelPoseBone& other): translation(other.translation), scaling(other.scaling), rotation(other.rotation),
    overrideTranslation(false), overrideRotation(false), overrideScaling(false) {}

} ModelBoneOverride;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class ModelPose {

  refptr<ModelContent> content;
  size_t actionIndex;
  ModelPoseBone* bones;
  ModelBoneOverride* boneOverrides;
  size_t boneCount;

public:

  refptr<ModelContent> GetModelContent() const {return content;}
  bool HasContent() const {return (bool) content;}

  size_t GetBoneCount() const {return boneCount;}

public:

  ModelPose();
  ~ModelPose();

public:

  void SetContent(refptr<ModelContent> content, size_t actionIndex = 0);

  void SetBoneOverrides(ModelBoneOverride* boneOverrides);

  void Copy(const ModelContentSkeletonPose& pose);
  void Copy(const ModelPose& pose);

  void Interpolate(const ModelPose& pose1, const ModelPose& pose2, f32 weight, const Set<std::string>* boneCancelInterpolate = nullptr);

  const ModelPoseBone* GetBone(size_t index) const;

  bool GetBonePointPos(const std::string& name, const Vec2& point, Vec2& pos) const;

  const ModelContentSkeleton* GetSkeleton() const;

};

};
