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

#include <Prime/Skeleton/SkeletonPose.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

SkeletonPose::SkeletonPose():
bones(nullptr),
boneOverrides(nullptr),
boneCount(0) {

}

SkeletonPose::~SkeletonPose() {
  PrimeSafeFree(bones);
}

void SkeletonPose::SetContent(refptr<SkeletonContent> content) {
  PrimeSafeFree(bones);
  boneOverrides = nullptr;
  boneCount = 0;

  this->content = content;

  if(!content)
    return;

  boneCount = content->GetBoneCount();
  bones = (SkeletonPoseBone*) calloc(boneCount, sizeof(SkeletonPoseBone));

  if(content->GetPoseCount() > 0) {
    const SkeletonContentPose* poses = content->GetPoses();
    if(poses) {
      Copy(*poses);
    }
  }
}

void SkeletonPose::Copy(const SkeletonContentPose& pose, const SkeletonPoseBone* rootBone) {
  if(!HasContent())
    return;
  
  size_t boneCount = content->GetBoneCount();
  const SkeletonContentBone* contentBones = content->GetBones();

  for(size_t i = 0; i < boneCount; i++) {
    size_t index = content->GetBoneIndexFromOrderedHierarchy(i);
    const SkeletonContentBone& contentBone = contentBones[index];
    const SkeletonContentPoseBone& contentPoseBone = pose.bones[pose.bones[index].boneLookupIndex];
    SkeletonPoseBone& bone = bones[i];

    if(contentBone.parentIndex == -1) {
      if(rootBone) {
        bone = *rootBone;
      }
      else {
        InitPoseBone(bone);
      }
    }
    else {
      size_t rev = content->GetBoneIndexFromOrderedHierarchy(contentBone.parentIndex, true);
      memcpy(&bone, &bones[rev], sizeof(SkeletonPoseBone));
      bone.alpha = 1.0f;
      bone.alphaInterpolate = 0.0f;
      bone.alphaInterpolateAnchor = (SkeletonPoseInterpolateAnchor) 0;
    }

    bone.poseAngle = contentPoseBone.angle;
    bone.poseScaleX = contentPoseBone.scaleX;
    bone.poseScaleY = contentPoseBone.scaleY;
    bone.poseX = contentPoseBone.x;
    bone.poseY = contentPoseBone.y;

    f32 useParentAngle = bone.angle;

    f32 scaleXWeight = contentPoseBone.scaleX;
    f32 scaleYWeight = contentPoseBone.scaleY;
    f32 angleWeight = contentPoseBone.angle;

    if(boneOverrides) {
      SkeletonBoneOverride* boneOverride = &boneOverrides[index];
      if(boneOverride) {
        if(boneOverride->overrideScale) {
          scaleXWeight = boneOverride->scaleX;
          scaleYWeight = boneOverride->scaleY;
        }
        if(boneOverride->overrideAngle) {
          angleWeight = boneOverride->angle;
        }
      }
    }

    f32 endX, endY;
    ScalePoint(endX, endY, contentBone.size, 0.0f, scaleXWeight, scaleYWeight);
    RotatePoint(endX, endY, endX, endY, angleWeight + useParentAngle);

    f32 baseXWeight = contentPoseBone.x;
    f32 baseYWeight = contentPoseBone.y;

    if(boneOverrides) {
      SkeletonBoneOverride* boneOverride = &boneOverrides[index];
      if(boneOverride) {
        if(boneOverride->overrideTranslation) {
          baseXWeight = boneOverride->x;
          baseYWeight = boneOverride->y;
        }
      }
    }

    f32 baseX, baseY;
    ScalePoint(baseX, baseY, baseXWeight, baseYWeight, bone.scaleX, bone.scaleY);
    RotatePoint(baseX, baseY, baseX, baseY, useParentAngle);

    f32 startX = bone.x + baseX;
    f32 startY = bone.y + baseY;

    if(contentBone.tip) {
      startX += bone.dx;
      startY += bone.dy;
    }

    bone.x = startX;
    bone.y = startY;
    bone.dx = endX;
    bone.dy = endY;
    bone.angleParent = useParentAngle;
    bone.angle = angleWeight + useParentAngle;
    bone.scaleX = scaleXWeight;
    bone.scaleY = scaleYWeight;
    bone.alpha = contentPoseBone.alpha;
    bone.alphaInterpolate = contentPoseBone.alphaInterpolate;
    bone.alphaInterpolateAnchor = contentPoseBone.alphaInterpolateAnchor;
  }
}

void SkeletonPose::SetBoneOverrides(SkeletonBoneOverride* boneOverrides) {
  this->boneOverrides = boneOverrides;
}

void SkeletonPose::Copy(const SkeletonPose& pose) {
  if(!HasContent() || !pose.HasContent() || content != pose.content)
    return;

  size_t boneCount = content->GetBoneCount();
  memcpy(bones, pose.bones, sizeof(SkeletonPoseBone) * boneCount);
}

void SkeletonPose::CopyPoseFromContent(const char* name) {
  if(!HasContent())
    return;

  const SkeletonContentPose* pose = content->FindPose(name);
  if(pose) {
    Copy(*pose);
  }
}

void SkeletonPose::Interpolate(const SkeletonPose& pose1, const SkeletonPose& pose2, f32 weight, const SkeletonPoseBone* rootBone, const Set<std::string>* boneCancelInterpolate) {
  if(!HasContent())
    return;

  size_t boneCount = content->GetBoneCount();
  const SkeletonContentBone* contentBones = content->GetBones();

  for(size_t i = 0; i < boneCount; i++) {
    size_t index = content->GetBoneIndexFromOrderedHierarchy(i);
    const SkeletonContentBone& contentBone = contentBones[index];
    const SkeletonPoseBone& poseBone1 = pose1.bones[i];
    const SkeletonPoseBone& poseBone2 = pose2.bones[i];
    SkeletonPoseBone& bone = bones[i];

    if(contentBone.parentIndex == -1) {
      if(rootBone) {
        bone = *rootBone;
      }
      else {
        InitPoseBone(bone);
      }
    }
    else {
      size_t rev = content->GetBoneIndexFromOrderedHierarchy(contentBone.parentIndex, true);
      memcpy(&bone, &bones[rev], sizeof(SkeletonPoseBone));
      bone.alpha = 1.0f;
      bone.alphaInterpolate = 0.0f;
      bone.alphaInterpolateAnchor = (SkeletonPoseInterpolateAnchor) 0;
    }

    f32 useWeight = weight;

    if(boneCancelInterpolate && boneCancelInterpolate->Find(contentBone.name)) {
      useWeight = 0.0f;
    }

    bone.poseAngle = poseBone1.poseAngle * (1.0f - useWeight) + poseBone2.poseAngle * useWeight;
    bone.poseScaleX = poseBone1.poseScaleX * (1.0f - useWeight) + poseBone2.poseScaleX * useWeight;
    bone.poseScaleY = poseBone1.poseScaleY * (1.0f - useWeight) + poseBone2.poseScaleY * useWeight;
    bone.poseX = poseBone1.poseX * (1.0f - useWeight) + poseBone2.poseX * useWeight;
    bone.poseY = poseBone1.poseY * (1.0f - useWeight) + poseBone2.poseY * useWeight;

    f32 useParentAngleEnd = bone.angle;
    f32 useParentAngleBase = bone.angle;

    if(boneOverrides) {
      SkeletonBoneOverride* boneOverride = &boneOverrides[index];
      if(boneOverride) {
        if(boneOverride->overrideTranslation) {
          bone.poseX = boneOverride->x;
          bone.poseY = boneOverride->y;
        }
        if(boneOverride->overrideScale) {
          bone.poseScaleX = boneOverride->scaleX;
          bone.poseScaleY = boneOverride->scaleY;
        }
        if(boneOverride->overrideAngle) {
          bone.poseAngle = boneOverride->angle;
          if(boneOverride->overrideAngleAbsolute) {
            useParentAngleEnd = 0.0f;
          }
        }
      }
    }

    f32 endX, endY;
    ScalePoint(endX, endY, contentBone.size, 0.0f, bone.poseScaleX, bone.poseScaleY);
    RotatePoint(endX, endY, endX, endY, bone.poseAngle + useParentAngleEnd);

    f32 baseX, baseY;
    ScalePoint(baseX, baseY, bone.poseX, bone.poseY, bone.scaleX, bone.scaleY);
    RotatePoint(baseX, baseY, baseX, baseY, useParentAngleBase);

    f32 startX = bone.x + baseX;
    f32 startY = bone.y + baseY;

    if(contentBone.tip) {
      startX += bone.dx;
      startY += bone.dy;
    }

    bone.x = startX;
    bone.y = startY;
    bone.dx = endX;
    bone.dy = endY;
    bone.angleParent = useParentAngleEnd;
    bone.angle = bone.poseAngle + useParentAngleEnd;
    bone.scaleX = bone.poseScaleX;
    bone.scaleY = bone.poseScaleY;

    if(poseBone1.alpha != 1.0f || poseBone2.alpha != 1.0f) {
      f32 interpolate1;
      f32 interpolate2;

      if(poseBone1.alphaInterpolateAnchor == SkeletonPoseInterpolateAnchorLeft) {
        interpolate1 = 0.0f;
        interpolate2 = poseBone1.alphaInterpolate;
      }
      else if(poseBone1.alphaInterpolateAnchor == SkeletonPoseInterpolateAnchorRight) {
        interpolate1 = 1.0f - poseBone1.alphaInterpolate;
        interpolate2 = 1.0f;
      }
      else {
        f32 extra = 1.0f - poseBone1.alphaInterpolate;
        f32 extra2 = extra / 2.0f;
        interpolate1 = extra2;
        interpolate2 = 1.0f - extra2;
      }

      f32 alphaWeight;

      if(weight <= interpolate1)
        alphaWeight = 0.0f;
      else if(weight >= interpolate2)
        alphaWeight = 1.0f;
      else
        alphaWeight = (weight - interpolate1) / poseBone1.alphaInterpolate;

      bone.alpha = poseBone1.alpha * (1.0f - alphaWeight) + poseBone2.alpha * alphaWeight;
    }
  }
}

const SkeletonPoseBone* SkeletonPose::GetBone(size_t index) const {
  if(HasContent())
    return &bones[content->GetBoneIndexFromOrderedHierarchy(index, true)];
  else
    return NULL;
}

void SkeletonPose::InitPoseBone(SkeletonPoseBone& bone) {
  bone.x = 0.0f;
  bone.y = 0.0f;
  bone.x2 = 0.0f;
  bone.y2 = 0.0f;
  bone.dx = 0.0f;
  bone.dy = 0.0f;
  bone.angle = 0.0f;
  bone.angleParent = 0.0f;
  bone.scaleX = 1.0f;
  bone.scaleY = 1.0f;
  bone.alpha = 1.0f;
  bone.alphaInterpolate = 0.0f;
  bone.alphaInterpolateAnchor = (SkeletonPoseInterpolateAnchor) 0;
}

void SkeletonPose::RotatePoint(f32& resultX, f32& resultY, f32 x, f32 y, f32 angle, f32 aboutX, f32 aboutY) {
  f32 ux = x - aboutX;
  f32 uy = y - aboutY;
  f32 angleRad = angle * PrimeDegToRadF;
  f32 ca = cosf(angleRad);
  f32 sa = sinf(angleRad);
  f32 nx = ca * ux - sa * uy;
  f32 ny = sa * ux + ca * uy;
  resultX = aboutX + nx;
  resultY = aboutY + ny;
}

void SkeletonPose::ScalePoint(f32& resultX, f32& resultY, f32 x, f32 y, f32 scaleX, f32 scaleY, f32 originX, f32 originY) {
  f32 ux = x - originX;
  f32 uy = y - originY;
  f32 nx = ux * scaleX;
  f32 ny = uy * scaleY;
  resultX = originX + nx;
  resultY = originY + ny;
}
