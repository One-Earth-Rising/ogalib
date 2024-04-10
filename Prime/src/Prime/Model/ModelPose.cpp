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

#include <Prime/Model/ModelPose.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ModelPose::ModelPose():
actionIndex(PrimeNotFound),
bones(nullptr),
boneOverrides(nullptr),
boneCount(0) {

}

ModelPose::~ModelPose() {
  PrimeSafeFree(bones);
}

void ModelPose::SetContent(refptr<ModelContent> content, size_t actionIndex) {
  PrimeSafeFree(bones);
  this->actionIndex = PrimeNotFound;
  bones = nullptr;
  boneOverrides = nullptr;
  boneCount = 0;

  if(!content)
    return;

  if(actionIndex == PrimeNotFound)
    return;

  if(actionIndex >= content->GetActionCount())
    return;

  const ModelContentAction& action = content->GetAction(actionIndex);

  size_t sceneIndex = content->GetSceneIndexByName(action.scene);
  if(sceneIndex == PrimeNotFound)
    return;

  const ModelContentScene& scene = content->GetScene(sceneIndex);

  const ModelContentSkeleton* skeleton = nullptr;
  if(scene.GetSkeletonCount() > 0) {
    skeleton = &scene.GetSkeleton(0);
  }

  if(!skeleton)
    return;

  this->content = content;

  if(!HasContent())
    return;

  this->actionIndex = actionIndex;
  boneCount = skeleton->GetBoneCount();
  bones = (ModelPoseBone*) calloc(boneCount, sizeof(ModelPoseBone));

  const ModelContentSkeletonPose& pose = skeleton->GetPose(0);
  Copy(pose);
}

void ModelPose::Copy(const ModelContentSkeletonPose& pose) {
  if(!HasContent())
    return;
  
  const ModelContentSkeleton* skeleton = GetSkeleton();
  if(!skeleton)
    return;

  size_t boneCount = skeleton->GetBoneCount();

  for(size_t i = 0; i < boneCount; i++) {
    const ModelContentSkeletonPoseBone& skeletonPoseBone = pose.GetPoseBone(i);
    ModelPoseBone& bone = bones[i];

    size_t boneIndex = skeletonPoseBone.GetBoneIndex();
    if(boneIndex != PrimeNotFound) {
      bone.translation = skeletonPoseBone.GetTranslation();
      bone.rotation = skeletonPoseBone.GetRotation();
      bone.scaling = skeletonPoseBone.GetScaling();
      bone.poseValid = true;
    }
    else {
      bone.poseValid = false;
    }
  }
}

void ModelPose::SetBoneOverrides(ModelBoneOverride* boneOverrides) {
  this->boneOverrides = boneOverrides;
}

void ModelPose::Copy(const ModelPose& pose) {
  if(!HasContent() || !pose.HasContent() || content != pose.content)
    return;

  for(size_t i = 0; i < boneCount; i++) {
    bones[i] = pose.bones[i];
  }
}

void ModelPose::Interpolate(const ModelPose& pose1, const ModelPose& pose2, f32 weight, const Set<std::string>* boneCancelInterpolate) {
  if(!HasContent())
    return;
  
  const ModelContentSkeleton* skeleton = GetSkeleton();
  if(!skeleton)
    return;

  for(size_t i = 0; i < boneCount; i++) {
    const ModelPoseBone& poseBone1 = pose1.bones[i];
    const ModelPoseBone& poseBone2 = pose2.bones[i];
    ModelPoseBone& bone = bones[i];

    if(poseBone1.poseValid && poseBone2.poseValid) {
      bone.translation = poseBone1.translation.GetLerp(poseBone2.translation, weight);
      bone.rotation = poseBone1.rotation.Interpolate(poseBone2.rotation, weight);
      bone.scaling = poseBone1.scaling.GetLerp(poseBone2.scaling, weight);
      bone.poseValid = true;
    }
    else {
      bone.poseValid = false;
    }
  }
}

const ModelPoseBone* ModelPose::GetBone(size_t index) const {
  if(!boneCount || !bones)
    return nullptr;

  if(index < boneCount)
    return &bones[index];
  else
    return nullptr;
}

const ModelContentSkeleton* ModelPose::GetSkeleton() const {
  if(actionIndex == PrimeNotFound)
    return nullptr;

  if(!content)
    return nullptr;

  if(actionIndex >= content->GetActionCount())
    return nullptr;

  const ModelContentAction& action = content->GetAction(actionIndex);

  size_t sceneIndex = content->GetSceneIndexByName(action.scene);
  if(sceneIndex == PrimeNotFound)
    return nullptr;

  const ModelContentScene& scene = content->GetScene(sceneIndex);

  const ModelContentSkeleton* skeleton = nullptr;
  if(scene.GetSkeletonCount() > 0) {
    skeleton = &scene.GetSkeleton(0);
  }

  return skeleton;
}
