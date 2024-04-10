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

#include <Prime/Model/ModelContentSkeleton.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Types/Set.h>
#include <zlib/zlib.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ModelContentSkeleton::ModelContentSkeleton():
bones(nullptr),
boneCount(0),
rootBoneIndex(PrimeNotFound),
actionPoseBoneCount(0),
poses(nullptr),
poseCount(0),
actions(nullptr),
actionCount(0),
signature(0) {

}

ModelContentSkeleton::~ModelContentSkeleton() {
  DestroyActions();
  DestroyPoses();
  DestroyBones();
}

void ModelContentSkeleton::Load(const aiScene& scene) {
  Stack<ModelContentSkeletonPose> createdPoses;

  const struct aiNode* rootBone = scene.mRootNode;
  if(rootBone) {
    std::string rootBoneName = rootBone->mName.C_Str();

    if(!boneLookupIndexByName.HasKey(rootBoneName)) {
      size_t index = boneLookupIndexByName.GetCount();
      boneLookupIndexByName[rootBoneName] = index;
      boneLookupNameByIndex[index] = rootBoneName;
    }

    rootBoneIndex = boneLookupIndexByName[rootBoneName];
    TraverseBoneHierarchyForNames(rootBone);

    if(rootBoneIndex != PrimeNotFound) {
      boneCount = boneLookupIndexByName.GetCount();
      if(boneCount) {
        bones = new ModelContentSkeletonBone[boneCount];
        for(size_t i = 0; i < boneCount; i++) {
          ModelContentSkeletonBone& bone = bones[i];
          bone.name = boneLookupNameByIndex[i];
        }

        TraverseBoneHierarchy(rootBone);

        for(size_t i = 0; i < scene.mNumMeshes; i++) {
          const struct aiMesh* sceneMesh = scene.mMeshes[i];
          for(size_t j = 0; j < sceneMesh->mNumBones; j++) {
            const struct aiBone* sceneBone = sceneMesh->mBones[j];
            std::string boneName = sceneBone->mName.C_Str();
            if(auto it = boneLookupIndexByName.Find(boneName)) {
              size_t boneIndex = it.value();
              ModelContentSkeletonBone& bone = bones[boneIndex];

              if(!bone.meshTransformations) {
                bone.meshTransformationCount = scene.mNumMeshes;
                bone.meshTransformations = new Mat44[bone.meshTransformationCount];
                bone.meshTransformationsValid = (bool*) calloc(bone.meshTransformationCount, sizeof(bool));
                if(!bone.meshTransformations || !bone.meshTransformationsValid) {
                  PrimeSafeDeleteArray(bone.meshTransformations);
                  PrimeSafeFree(bone.meshTransformationsValid);
                  bone.meshTransformationCount = 0;
                }
              }

              if(bone.meshTransformations && bone.meshTransformationsValid) {
                CopyMatrix(bone.meshTransformations[i], sceneBone->mOffsetMatrix);
                bone.meshTransformationsValid[i] = true;
              }
            }
            else {
              PrimeAssert(false, "Expected to find bone in lookup table: %s", boneName.c_str());
            }
          }
        }

        actionCount = scene.mNumAnimations;
        actions = new ModelContentSkeletonAction[actionCount];
        for(size_t i = 0; i < actionCount; i++) {
          ModelContentSkeletonAction& action = actions[i];
          const struct aiAnimation* sceneAnimation = scene.mAnimations[i];
          action.name = sceneAnimation->mName.C_Str();
          action.keyFrameTime = (f32) (1.0 / sceneAnimation->mTicksPerSecond);
          action.len = (f32) (sceneAnimation->mDuration / sceneAnimation->mTicksPerSecond);
          lookupActionIndexByName[action.name] = i;

          size_t channelCount = sceneAnimation->mNumChannels;

          Set<double> keyFrameTimes;
          for(size_t j = 0; j < channelCount; j++) {
            const struct aiNodeAnim* sceneNodeAnim = sceneAnimation->mChannels[j];

            size_t positionKeyCount = sceneNodeAnim->mNumPositionKeys;
            for(size_t k = 0; k < positionKeyCount; k++) {
              const struct aiVectorKey& sceneKey = sceneNodeAnim->mPositionKeys[k];
              if(sceneKey.mTime > 0.0 && !keyFrameTimes.HasItem(sceneKey.mTime)) {
                keyFrameTimes.Add(sceneKey.mTime);
              }
            }

            size_t rotationKeyCount = sceneNodeAnim->mNumRotationKeys;
            for(size_t k = 0; k < rotationKeyCount; k++) {
              const struct aiQuatKey& sceneKey = sceneNodeAnim->mRotationKeys[k];
              if(sceneKey.mTime > 0.0 && !keyFrameTimes.HasItem(sceneKey.mTime)) {
                keyFrameTimes.Add(sceneKey.mTime);
              }
            }

            size_t scalingKeyCount = sceneNodeAnim->mNumScalingKeys;
            for(size_t k = 0; k < scalingKeyCount; k++) {
              const struct aiVectorKey& sceneKey = sceneNodeAnim->mScalingKeys[k];
              if(sceneKey.mTime > 0.0 && !keyFrameTimes.HasItem(sceneKey.mTime)) {
                keyFrameTimes.Add(sceneKey.mTime);
              }
            }
          }

          action.keyFrameCount = keyFrameTimes.GetCount();
          if(action.keyFrameCount) {
            Stack<double> keyFrameTimesStack;
            for(auto keyFrameTime: keyFrameTimes) {
              keyFrameTimesStack.Add(keyFrameTime);
            }
            keyFrameTimesStack.Sort();

            action.keyFrames = new ModelContentSkeletonActionKeyFrame[action.keyFrameCount];
            if(action.keyFrames) {
              Dictionary<double, size_t> keyFrameIndexByTime;

              if(action.keyFrameCount > 0) {
                for(size_t j = 0; j < action.keyFrameCount; j++) {
                  ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[j];
                  double time = keyFrameTimesStack[j];
                  keyFrame.time = (f32) (time / sceneAnimation->mTicksPerSecond);
                  keyFrameIndexByTime[time] = j;
                }

                ModelContentSkeletonActionKeyFrame& firstKeyFrame = action.keyFrames[0];
                EnsureKeyFramePose(firstKeyFrame, 0, action, createdPoses);

                size_t poseIndex = firstKeyFrame.poseIndex;
                ModelContentSkeletonPose& pose = createdPoses[poseIndex];
                for(size_t j = 0; j < pose.GetPoseBoneCount(); j++) {
                  ModelContentSkeletonPoseBone& poseBone = pose.poseBones[j];
                  poseBone.translationKnown = true;
                  poseBone.scalingKnown = true;
                  poseBone.rotationKnown = true;
                }
              }

              for(size_t j = 0; j < channelCount; j++) {
                const struct aiNodeAnim* sceneNodeAnim = sceneAnimation->mChannels[j];
                std::string boneName = sceneNodeAnim->mNodeName.C_Str();
                PrimeAssert(boneLookupIndexByName.HasKey(boneName), "Unexpected bone name: %s", boneName.c_str());
                size_t boneIndex = boneLookupIndexByName[boneName];

                size_t positionKeyCount = sceneNodeAnim->mNumPositionKeys;
                for(size_t k = 0; k < positionKeyCount; k++) {
                  const struct aiVectorKey& sceneKey = sceneNodeAnim->mPositionKeys[k];
                  size_t keyFrameIndex = keyFrameIndexByTime[sceneKey.mTime];
                  ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[keyFrameIndex];

                  EnsureKeyFramePose(keyFrame, keyFrameIndex, action, createdPoses);

                  size_t poseIndex = keyFrame.poseIndex;
                  ModelContentSkeletonPose& pose = createdPoses[poseIndex];
                  ModelContentSkeletonPoseBone& poseBone = pose.poseBones[boneIndex];
                  poseBone.boneIndex = boneIndex;

                  poseBone.translation = Vec3(sceneKey.mValue.x, sceneKey.mValue.y, sceneKey.mValue.z);
                  poseBone.translationKnown = true;
                }

                size_t scalingKeyCount = sceneNodeAnim->mNumScalingKeys;
                for(size_t k = 0; k < scalingKeyCount; k++) {
                  const struct aiVectorKey& sceneKey = sceneNodeAnim->mScalingKeys[k];
                  size_t keyFrameIndex = keyFrameIndexByTime[sceneKey.mTime];
                  ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[keyFrameIndex];

                  EnsureKeyFramePose(keyFrame, keyFrameIndex, action, createdPoses);

                  size_t poseIndex = keyFrame.poseIndex;
                  ModelContentSkeletonPose& pose = createdPoses[poseIndex];
                  ModelContentSkeletonPoseBone& poseBone = pose.poseBones[boneIndex];
                  poseBone.boneIndex = boneIndex;

                  poseBone.scaling = Vec3(sceneKey.mValue.x, sceneKey.mValue.y, sceneKey.mValue.z);
                  poseBone.scalingKnown = true;
                }

                size_t rotationKeyCount = sceneNodeAnim->mNumRotationKeys;
                for(size_t k = 0; k < rotationKeyCount; k++) {
                  const struct aiQuatKey& sceneKey = sceneNodeAnim->mRotationKeys[k];
                  size_t keyFrameIndex = keyFrameIndexByTime[sceneKey.mTime];
                  ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[keyFrameIndex];

                  EnsureKeyFramePose(keyFrame, keyFrameIndex, action, createdPoses);

                  size_t poseIndex = keyFrame.poseIndex;
                  ModelContentSkeletonPose& pose = createdPoses[poseIndex];
                  ModelContentSkeletonPoseBone& poseBone = pose.poseBones[boneIndex];
                  poseBone.boneIndex = boneIndex;

                  poseBone.rotation = Quat(sceneKey.mValue.x, sceneKey.mValue.y, sceneKey.mValue.z, sceneKey.mValue.w);
                  poseBone.rotationKnown = true;
                }
              }

              size_t knownPoseIndex = 0;
              for(size_t j = 1; j < action.keyFrameCount; j++) {
                ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[j];

                if(keyFrame.poseIndex == PrimeNotFound) {
                  keyFrame.poseIndex = knownPoseIndex;
                }
                else {
                  knownPoseIndex = keyFrame.poseIndex;
                }

                EnsureKeyFrameTransformations(keyFrame, j, action, createdPoses);
              }
            }
            else {
              action.keyFrameCount = 0;
            }
          }
        }
      }

      rootBoneTransform = GetBone(rootBoneIndex).GetTransformation();
      rootBoneTransformInv = rootBoneTransform;
      rootBoneTransformInv.Invert();
    }
  }

  poseCount = createdPoses.GetCount();
  if(poseCount) {
    poses = new ModelContentSkeletonPose[poseCount];
    if(poses) {
      size_t i = 0;
      for(const auto& createdPose: createdPoses) {
        poses[i++] = createdPose;
      }
    }
    else {
      poseCount = 0;
    }
  }

  signature = 0;
  char intBuffer[64];
  for(size_t i = 0; i < boneCount; i++) {
    ModelContentSkeletonBone& bone = bones[i];
    snprintf(intBuffer, sizeof(intBuffer) - 1, "%zu", i);

    signature = crc32(signature, (const Bytef*) intBuffer, (uInt) strlen(intBuffer));

    const char* s = bone.GetName().c_str();
    signature = crc32(signature, (const Bytef*) intBuffer, (uInt) strlen(intBuffer));
  }
}

void ModelContentSkeleton::Load(const tinygltf::Model& model) {
  Stack<ModelContentSkeletonPose> createdPoses;

  Dictionary<std::string, size_t> nodeNameLookupMeshIndex;
  bool usingInverseFromSkin = false;

  size_t nodeCount = model.nodes.size();
  for(size_t i = 0; i < nodeCount; i++) {
    const auto& node = model.nodes[i];
    std::string nodeName = node.name;
    if(nodeName.empty())
      nodeName = GenerateBoneNameFromTinyGLTFNodeIndex(model, i);
    if(!boneLookupIndexByName.HasKey(nodeName)) {
      size_t index = boneLookupIndexByName.GetCount();
      boneLookupIndexByName[nodeName] = index;
      boneLookupNameByIndex[index] = nodeName;
    }

    if(node.mesh >= 0) {
      nodeNameLookupMeshIndex[nodeName] = node.mesh;
    }
  }

  std::string rootBoneName;
  int rootBoneNodeIndex = -1;

  Dictionary<std::string, Mat44> skinInverseBindTransforms;

  size_t skinCount = model.skins.size();
  if(skinCount > 0) {
    const auto& skin = model.skins[0];

    if(skin.inverseBindMatrices != -1) {
      const auto& accessor = model.accessors[skin.inverseBindMatrices];
      const auto& bufferView = model.bufferViews[accessor.bufferView];
      const auto& buffer = model.buffers[bufferView.buffer];
      const f32* bufferData = (const f32*) &buffer.data[accessor.byteOffset + bufferView.byteOffset];
      const f32* bufferDataP = bufferData;

      if(accessor.count == skin.joints.size()) {
        for(size_t i = 0; i < accessor.count; i++) {
          int jointIndex = skin.joints[i];
          const auto& joint = model.nodes[jointIndex];

          Mat44 mat(bufferDataP);
          skinInverseBindTransforms[joint.name] = mat;
          bufferDataP += 16;
        }
      }

      usingInverseFromSkin = true;
    }
  }

  if(rootBoneName.empty()) {
    size_t sceneCount = model.scenes.size();
    for(size_t i = 0; i < sceneCount; i++) {
      const auto& scene = model.scenes[i];
      size_t nodeCount = scene.nodes.size();
      for(size_t j = 0; j < nodeCount; j++) {
        int nodeIndex = scene.nodes[j];
        if(nodeIndex >= 0) {
          const auto& node = model.nodes[nodeIndex];
          int meshIndex = FindTinyGLTFMeshIndexByTinyGLTFNodeIndex(model, nodeIndex);

          if(meshIndex != -1) {
            rootBoneName = node.name;
            if(rootBoneName.empty()) {
              rootBoneName = GenerateBoneNameFromTinyGLTFNodeIndex(model, nodeIndex);
            }
            break;
          }
        }
      }
    }
  }

  if(!rootBoneName.empty()) {
    if(!boneLookupIndexByName.HasKey(rootBoneName)) {
      size_t index = boneLookupIndexByName.GetCount();
      boneLookupIndexByName[rootBoneName] = index;
      boneLookupNameByIndex[index] = rootBoneName;
    }

    rootBoneIndex = boneLookupIndexByName[rootBoneName];
  }

  size_t meshCount = model.meshes.size();

  boneCount = boneLookupIndexByName.GetCount();
  if(boneCount) {
    bones = new ModelContentSkeletonBone[boneCount];
    for(size_t i = 0; i < boneCount; i++) {
      ModelContentSkeletonBone& bone = bones[i];
      bone.name = boneLookupNameByIndex[i];
    }

    int rootBoneNodeIndex = FindTinyGLTFNode(model, rootBoneName);
    Mat44 transformation, parentTransformation;
    Dictionary<std::string, Mat44> unusedSkinInverseBindTransforms;
    Dictionary<std::string, Mat44>* useSkinInverseBindTransforms = usingInverseFromSkin ? &unusedSkinInverseBindTransforms : &skinInverseBindTransforms;
    TraverseBoneHierarchy(model, rootBoneNodeIndex, transformation, parentTransformation, *useSkinInverseBindTransforms);

    for(size_t i = 0; i < meshCount; i++) {
      for(size_t j = 0; j < boneCount; j++) {
        size_t boneIndex = j;
        ModelContentSkeletonBone& bone = bones[boneIndex];
        if(!bone.meshTransformations) {
          bone.meshTransformationCount = meshCount;
          bone.meshTransformations = new Mat44[bone.meshTransformationCount];
          bone.meshTransformationsValid = (bool*) calloc(bone.meshTransformationCount, sizeof(bool));
          if(!bone.meshTransformations || !bone.meshTransformationsValid) {
            PrimeSafeDeleteArray(bone.meshTransformations);
            PrimeSafeFree(bone.meshTransformationsValid);
            bone.meshTransformationCount = 0;
          }
        }

        if(bone.meshTransformations && bone.meshTransformationsValid) {
          if(usingInverseFromSkin) {
            if(auto it = skinInverseBindTransforms.Find(bone.name)) {
              bone.meshTransformations[i] = it.value();
              bone.meshTransformationsValid[i] = true;
            }
          }
          else {
            if(auto it = nodeNameLookupMeshIndex.Find(bone.name)) {
              size_t meshIndex = it.value();
              if(meshIndex == i) {
                if(auto it2 = skinInverseBindTransforms.Find(bone.name)) {
                  bone.meshTransformations[i] = it2.value();
                  bone.meshTransformationsValid[i] = true;
                }
              }
            }
          }
        }
      }
    }

    actionCount = model.animations.size();
    if(actionCount > 0) {
      actions = new ModelContentSkeletonAction[actionCount];
      for(size_t i = 0; i < actionCount; i++) {
        ModelContentSkeletonAction& action = actions[i];
        const auto& dataAnimation = model.animations[i];
        action.name = dataAnimation.name;
        lookupActionIndexByName[action.name] = i;

        Set<f32> keyFrameTimes;

        size_t channelCount = dataAnimation.channels.size();
        for(size_t j = 0; j < channelCount; j++) {
          const auto& dataChannel = dataAnimation.channels[j];
          const auto& dataSampler = dataAnimation.samplers[dataChannel.sampler];
          const auto& inputAccessor = model.accessors[dataSampler.input];
          const auto& inputBufferView = model.bufferViews[inputAccessor.bufferView];
          const auto& inputBuffer = model.buffers[inputBufferView.buffer];

          const auto& dataNode = model.nodes[dataChannel.target_node];
          std::string boneName = dataNode.name;
          if(boneName.empty())
            boneName = GenerateBoneNameFromTinyGLTFNodeIndex(model, dataChannel.target_node);
          PrimeAssert(boneLookupIndexByName.HasKey(boneName), "Unexpected bone name: %s", boneName.c_str());
          size_t boneIndex = boneLookupIndexByName[boneName];

          int modelBoneNode = FindTinyGLTFNode(model, boneName.c_str());

          const f32* dataKeyFrameTimes = (const f32*) &inputBuffer.data[inputAccessor.byteOffset + inputBufferView.byteOffset];
          if(dataKeyFrameTimes) {
            for(size_t k = 0; k < inputAccessor.count; k++) {
              f32 keyFrameTime = dataKeyFrameTimes[k];
              if(!keyFrameTimes.HasItem(keyFrameTime)) {
                keyFrameTimes.Add(keyFrameTime);
              }
            }
          }
        }

        action.keyFrameCount = keyFrameTimes.GetCount();
        if(action.keyFrameCount) {
          Stack<f32> keyFrameTimesStack;
          for(auto keyFrameTime: keyFrameTimes) {
            keyFrameTimesStack.Add(keyFrameTime);
          }
          keyFrameTimesStack.Sort();

          action.keyFrames = new ModelContentSkeletonActionKeyFrame[action.keyFrameCount];
          if(action.keyFrames) {
            Dictionary<f32, size_t> keyFrameIndexByTime;

            action.len = keyFrameTimesStack[keyFrameTimesStack.GetCount() - 1] - keyFrameTimesStack[0];

            if(action.keyFrameCount > 0) {
              for(size_t k = 0; k < action.keyFrameCount; k++) {
                ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[k];
                f32 time = keyFrameTimesStack[k];
                keyFrame.time = time;
                keyFrameIndexByTime[time] = k;
              }
            }
          }
        }

        for(size_t j = 0; j < channelCount; j++) {
          const auto& dataChannel = dataAnimation.channels[j];
          const auto& dataSampler = dataAnimation.samplers[dataChannel.sampler];
          const auto& outputAccessor = model.accessors[dataSampler.output];
          const auto& outputBufferView = model.bufferViews[outputAccessor.bufferView];
          const auto& outputBuffer = model.buffers[outputBufferView.buffer];

          const auto& dataNode = model.nodes[dataChannel.target_node];
          std::string boneName = dataNode.name;
          if(boneName.empty())
            boneName = GenerateBoneNameFromTinyGLTFNodeIndex(model, dataChannel.target_node);
          PrimeAssert(boneLookupIndexByName.HasKey(boneName), "Unexpected bone name: %s", boneName.c_str());
          size_t boneIndex = boneLookupIndexByName[boneName];

          if(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
            const s16* dataKeyFrames = (const s16*) &outputBuffer.data[outputAccessor.byteOffset + outputBufferView.byteOffset];
            size_t dataKeyFramesByteStride = outputAccessor.ByteStride(outputBufferView);
            if(dataKeyFrames) {
              if(dataChannel.target_path == "translation") {
                if(outputAccessor.type == TINYGLTF_TYPE_VEC3) {
                  size_t outputAccessorCount = outputAccessor.count;
                  for(size_t k = 0; k < action.keyFrameCount; k++) {
                    size_t keyFrameIndex = k;
                    ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[keyFrameIndex];

                    EnsureKeyFramePose(keyFrame, keyFrameIndex, action, createdPoses);

                    size_t poseIndex = keyFrame.poseIndex;
                    ModelContentSkeletonPose& pose = createdPoses[poseIndex];
                    ModelContentSkeletonPoseBone& poseBone = pose.poseBones[boneIndex];
                    poseBone.boneIndex = boneIndex;

                    if(k < outputAccessorCount) {
                      f32 x = *dataKeyFrames++ / 32767.0f;
                      f32 y = *dataKeyFrames++ / 32767.0f;
                      f32 z = *dataKeyFrames++ / 32767.0f;

                      if(dataKeyFramesByteStride > 6) {
                        for(size_t m = 0; m < (dataKeyFramesByteStride - 6) / sizeof(s16); m++) {
                          dataKeyFrames++;
                        }
                      }

                      poseBone.translation = Vec3(x, y, z);
                      poseBone.translationKnown = true;
                    }
                    else if(outputAccessorCount > 0) {
                      ModelContentSkeletonActionKeyFrame& keyFrameSource = action.keyFrames[outputAccessorCount - 1];
                      size_t poseIndex = keyFrameSource.poseIndex;
                      ModelContentSkeletonPose& poseSource = createdPoses[poseIndex];
                      ModelContentSkeletonPoseBone& poseBoneSource = poseSource.poseBones[boneIndex];

                      poseBone.translation = poseBoneSource.translation;
                      poseBone.translationKnown = poseBoneSource.translationKnown;
                    }
                  }
                }
                else {
                  PrimeAssert(false, "Expected vec3 for translation.");
                }
              }
              else if(dataChannel.target_path == "rotation") {
                if(outputAccessor.type == TINYGLTF_TYPE_VEC4) {
                  size_t outputAccessorCount = outputAccessor.count;
                  for(size_t k = 0; k < action.keyFrameCount; k++) {
                    size_t keyFrameIndex = k;
                    ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[keyFrameIndex];

                    EnsureKeyFramePose(keyFrame, keyFrameIndex, action, createdPoses);

                    size_t poseIndex = keyFrame.poseIndex;
                    ModelContentSkeletonPose& pose = createdPoses[poseIndex];
                    ModelContentSkeletonPoseBone& poseBone = pose.poseBones[boneIndex];
                    poseBone.boneIndex = boneIndex;

                    if(k < outputAccessorCount) {
                      f32 x = *dataKeyFrames++ / 32767.0f;
                      f32 y = *dataKeyFrames++ / 32767.0f;
                      f32 z = *dataKeyFrames++ / 32767.0f;
                      f32 w = *dataKeyFrames++ / 32767.0f;

                      if(dataKeyFramesByteStride > 8) {
                        for(size_t m = 0; m < (dataKeyFramesByteStride - 8) / sizeof(s16); m++) {
                          dataKeyFrames++;
                        }
                      }

                      poseBone.rotation = Quat(x, y, z, w);
                      poseBone.rotationKnown = true;
                    }
                    else if(outputAccessorCount > 0) {
                      ModelContentSkeletonActionKeyFrame& keyFrameSource = action.keyFrames[outputAccessorCount - 1];
                      size_t poseIndex = keyFrameSource.poseIndex;
                      ModelContentSkeletonPose& poseSource = createdPoses[poseIndex];
                      ModelContentSkeletonPoseBone& poseBoneSource = poseSource.poseBones[boneIndex];

                      poseBone.rotation = poseBoneSource.rotation;
                      poseBone.rotationKnown = poseBoneSource.rotationKnown;
                    }
                  }
                }
                else {
                  PrimeAssert(false, "Expected vec4 for rotation.");
                }
              }
              else if(dataChannel.target_path == "scale") {
                if(outputAccessor.type == TINYGLTF_TYPE_VEC3) {
                  size_t outputAccessorCount = outputAccessor.count;
                  for(size_t k = 0; k < action.keyFrameCount; k++) {
                    size_t keyFrameIndex = k;
                    ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[keyFrameIndex];

                    EnsureKeyFramePose(keyFrame, keyFrameIndex, action, createdPoses);

                    size_t poseIndex = keyFrame.poseIndex;
                    ModelContentSkeletonPose& pose = createdPoses[poseIndex];
                    ModelContentSkeletonPoseBone& poseBone = pose.poseBones[boneIndex];
                    poseBone.boneIndex = boneIndex;

                    if(k < outputAccessorCount) {
                      f32 x = *dataKeyFrames++ / 32767.0f;
                      f32 y = *dataKeyFrames++ / 32767.0f;
                      f32 z = *dataKeyFrames++ / 32767.0f;

                      if(dataKeyFramesByteStride > 6) {
                        for(size_t m = 0; m < (dataKeyFramesByteStride - 6) / sizeof(s16); m++) {
                          dataKeyFrames++;
                        }
                      }

                      poseBone.scaling = Vec3(x, y, z);
                      poseBone.scalingKnown = true;
                    }
                    else if(outputAccessorCount > 0) {
                      ModelContentSkeletonActionKeyFrame& keyFrameSource = action.keyFrames[outputAccessorCount - 1];
                      size_t poseIndex = keyFrameSource.poseIndex;
                      ModelContentSkeletonPose& poseSource = createdPoses[poseIndex];
                      ModelContentSkeletonPoseBone& poseBoneSource = poseSource.poseBones[boneIndex];

                      poseBone.scaling = poseBoneSource.scaling;
                      poseBone.scalingKnown = poseBoneSource.scalingKnown;
                    }
                  }
                }
                else {
                  PrimeAssert(false, "Expected vec3 for scaling.");
                }
              }
            }
          }
          else {
            PrimeAssert(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Unsupported bone transform component type.");

            const f32* dataKeyFrames = (const f32*) &outputBuffer.data[outputAccessor.byteOffset + outputBufferView.byteOffset];
            if(dataKeyFrames) {
              if(dataChannel.target_path == "translation") {
                if(outputAccessor.type == TINYGLTF_TYPE_VEC3) {
                  size_t outputAccessorCount = outputAccessor.count;
                  for(size_t k = 0; k < action.keyFrameCount; k++) {
                    size_t keyFrameIndex = k;
                    ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[keyFrameIndex];

                    EnsureKeyFramePose(keyFrame, keyFrameIndex, action, createdPoses);

                    size_t poseIndex = keyFrame.poseIndex;
                    ModelContentSkeletonPose& pose = createdPoses[poseIndex];
                    ModelContentSkeletonPoseBone& poseBone = pose.poseBones[boneIndex];
                    poseBone.boneIndex = boneIndex;

                    if(k < outputAccessorCount) {
                      f32 x = *dataKeyFrames++;
                      f32 y = *dataKeyFrames++;
                      f32 z = *dataKeyFrames++;

                      poseBone.translation = Vec3(x, y, z);
                      poseBone.translationKnown = true;
                    }
                    else if(outputAccessorCount > 0) {
                      ModelContentSkeletonActionKeyFrame& keyFrameSource = action.keyFrames[outputAccessorCount - 1];
                      size_t poseIndex = keyFrameSource.poseIndex;
                      ModelContentSkeletonPose& poseSource = createdPoses[poseIndex];
                      ModelContentSkeletonPoseBone& poseBoneSource = poseSource.poseBones[boneIndex];

                      poseBone.translation = poseBoneSource.translation;
                      poseBone.translationKnown = poseBoneSource.translationKnown;
                    }
                  }
                }
                else {
                  PrimeAssert(false, "Expected vec3 for translation.");
                }
              }
              else if(dataChannel.target_path == "rotation") {
                if(outputAccessor.type == TINYGLTF_TYPE_VEC4) {
                  size_t outputAccessorCount = outputAccessor.count;
                  for(size_t k = 0; k < action.keyFrameCount; k++) {
                    size_t keyFrameIndex = k;
                    ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[keyFrameIndex];

                    EnsureKeyFramePose(keyFrame, keyFrameIndex, action, createdPoses);

                    size_t poseIndex = keyFrame.poseIndex;
                    ModelContentSkeletonPose& pose = createdPoses[poseIndex];
                    ModelContentSkeletonPoseBone& poseBone = pose.poseBones[boneIndex];
                    poseBone.boneIndex = boneIndex;

                    if(k < outputAccessorCount) {
                      f32 x = *dataKeyFrames++;
                      f32 y = *dataKeyFrames++;
                      f32 z = *dataKeyFrames++;
                      f32 w = *dataKeyFrames++;

                      poseBone.rotation = Quat(x, y, z, w);
                      poseBone.rotationKnown = true;
                    }
                    else if(outputAccessorCount > 0) {
                      ModelContentSkeletonActionKeyFrame& keyFrameSource = action.keyFrames[outputAccessorCount - 1];
                      size_t poseIndex = keyFrameSource.poseIndex;
                      ModelContentSkeletonPose& poseSource = createdPoses[poseIndex];
                      ModelContentSkeletonPoseBone& poseBoneSource = poseSource.poseBones[boneIndex];

                      poseBone.rotation = poseBoneSource.rotation;
                      poseBone.rotationKnown = poseBoneSource.rotationKnown;
                    }
                  }
                }
                else {
                  PrimeAssert(false, "Expected vec4 for rotation.");
                }
              }
              else if(dataChannel.target_path == "scale") {
                if(outputAccessor.type == TINYGLTF_TYPE_VEC3) {
                  size_t outputAccessorCount = outputAccessor.count;
                  for(size_t k = 0; k < action.keyFrameCount; k++) {
                    size_t keyFrameIndex = k;
                    ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[keyFrameIndex];

                    EnsureKeyFramePose(keyFrame, keyFrameIndex, action, createdPoses);

                    size_t poseIndex = keyFrame.poseIndex;
                    ModelContentSkeletonPose& pose = createdPoses[poseIndex];
                    ModelContentSkeletonPoseBone& poseBone = pose.poseBones[boneIndex];
                    poseBone.boneIndex = boneIndex;

                    if(k < outputAccessorCount) {
                      f32 x = *dataKeyFrames++;
                      f32 y = *dataKeyFrames++;
                      f32 z = *dataKeyFrames++;

                      poseBone.scaling = Vec3(x, y, z);
                      poseBone.scalingKnown = true;
                    }
                    else if(outputAccessorCount > 0) {
                      ModelContentSkeletonActionKeyFrame& keyFrameSource = action.keyFrames[outputAccessorCount - 1];
                      size_t poseIndex = keyFrameSource.poseIndex;
                      ModelContentSkeletonPose& poseSource = createdPoses[poseIndex];
                      ModelContentSkeletonPoseBone& poseBoneSource = poseSource.poseBones[boneIndex];

                      poseBone.scaling = poseBoneSource.scaling;
                      poseBone.scalingKnown = poseBoneSource.scalingKnown;
                    }
                  }
                }
                else {
                  PrimeAssert(false, "Expected vec3 for scaling.");
                }
              }
            }
          }
        }

        size_t knownPoseIndex = 0;
        for(size_t j = 1; j < action.keyFrameCount; j++) {
          ModelContentSkeletonActionKeyFrame& keyFrame = action.keyFrames[j];

          if(keyFrame.poseIndex == PrimeNotFound) {
            keyFrame.poseIndex = knownPoseIndex;
          }
          else {
            knownPoseIndex = keyFrame.poseIndex;
          }

          EnsureKeyFrameTransformations(keyFrame, j, action, createdPoses);
        }
      }
    }
  }

  poseCount = createdPoses.GetCount();
  if(poseCount) {
    poses = new ModelContentSkeletonPose[poseCount];
    if(poses) {
      size_t i = 0;
      for(const auto& createdPose: createdPoses) {
        poses[i++] = createdPose;
      }
    }
    else {
      poseCount = 0;
    }
  }

  signature = 0;
  char intBuffer[64];
  for(size_t i = 0; i < boneCount; i++) {
    ModelContentSkeletonBone& bone = bones[i];
    snprintf(intBuffer, sizeof(intBuffer) - 1, "%zu", i);

    signature = crc32(signature, (const Bytef*) intBuffer, (uInt) strlen(intBuffer));

    const char* s = bone.GetName().c_str();
    signature = crc32(signature, (const Bytef*) intBuffer, (uInt) strlen(intBuffer));
  }
}

size_t ModelContentSkeleton::GetBoneIndexByName(const std::string& name) const {
  if(auto it = boneLookupIndexByName.Find(name))
    return it.value();

  return PrimeNotFound;
}

size_t ModelContentSkeleton::GetActionPoseBoneIndexByName(const std::string& name) const {
  if(auto it = boneLookupIndexByName.Find(name)) {
    ModelContentSkeletonBone& bone = bones[it.value()];
    return bone.GetActionPoseBoneIndex();
  }

  return PrimeNotFound;
}

void ModelContentSkeleton::ApplyBoneAffectingVertices(const std::string& name) {
  size_t boneIndex = GetBoneIndexByName(name);
  if(boneIndex == PrimeNotFound)
    return;

  ModelContentSkeletonBone& bone = bones[boneIndex];

  if(bone.actionPoseBoneIndex == PrimeNotFound) {
    bone.actionPoseBoneIndex = actionPoseBoneCount;
    actionPoseBoneCount++;
  }
}

const ModelContentSkeletonAction* ModelContentSkeleton::GetActionByName(const std::string& name) const {
  if(auto it = lookupActionIndexByName.Find(name)) {
    return &actions[it.value()];
  }
  else {
    return nullptr;
  }
}

const aiNode* ModelContentSkeleton::FindRootBone(const aiNode* node) {
  for(size_t i = 0; i < node->mNumChildren; i++) {
    const aiNode* child = node->mChildren[i];
    std::string childName = child->mName.C_Str();

    if(boneLookupIndexByName.HasKey(childName)) {
      const aiNode* current = child;
      while(current->mParent) {
        std::string currentName = current->mName.data;

        if(!boneLookupIndexByName.HasKey(currentName)) {
          return current;
        }

        current = current->mParent;
        if(!current->mParent) {
          return current;
        }
      }
    }
    else {
      const aiNode* result = FindRootBone(child);
      if(result)
        return result;
    }
  }

  return nullptr;
}

void ModelContentSkeleton::TraverseBoneHierarchyForNames(const aiNode* node) {
  if(!node)
    return;

  for(size_t i = 0; i < node->mNumChildren; i++) {
    const aiNode* child = node->mChildren[i];
    std::string childName = child->mName.C_Str();
    if(!boneLookupIndexByName.HasKey(childName)) {
      size_t index = boneLookupIndexByName.GetCount();
      boneLookupIndexByName[childName] = index;
      boneLookupNameByIndex[index] = childName;
    }

    TraverseBoneHierarchyForNames(child);
  }
}

void ModelContentSkeleton::TraverseBoneHierarchy(const aiNode* node) {
  if(!node)
    return;

  std::string nodeName = node->mName.C_Str();
  if(auto it = boneLookupIndexByName.Find(nodeName)) {
    size_t boneIndex = it.value();
    ModelContentSkeletonBone& bone = bones[boneIndex];
    CopyMatrix(bone.transformation, node->mTransformation);

    Stack<size_t> childBoneIndices;
    Stack<const aiNode*> childNodes;

    for(size_t i = 0; i < node->mNumChildren; i++) {
      const aiNode* child = node->mChildren[i];
      std::string childName = child->mName.C_Str();
      if(auto it = boneLookupIndexByName.Find(childName)) {
        size_t childIndex = it.value();
        childBoneIndices.Add(childIndex);
        childNodes.Add(child);
      }
      else {
        PrimeAssert("Expected to find bone in lookup table: name = %s", childName.c_str());
      }
    }

    bone.childBoneIndexCount = childBoneIndices.GetCount();
    if(bone.childBoneIndexCount) {
      bone.childBoneIndices = new size_t[bone.childBoneIndexCount];
      if(bone.childBoneIndices) {
        size_t i = 0;
        for(size_t childIndex: childBoneIndices) {
          bone.childBoneIndices[i++] = childIndex;
        }
      }
      else {
        bone.childBoneIndexCount = 0;
      }
    }

    for(auto child: childNodes) {
      TraverseBoneHierarchy(child);
    }
  }
}

int ModelContentSkeleton::FindTinyGLTFNode(const tinygltf::Model& model, const std::string& name) {
  int nodeIndex = 0;
  for(const auto& node: model.nodes) {
    if(node.name.empty()) {
      std::string nodeName = GenerateBoneNameFromTinyGLTFNodeIndex(model, nodeIndex);
      if(nodeName == name) {
        return nodeIndex;
      }
    }
    else if(node.name == name) {
      return nodeIndex;
    }
    nodeIndex++;
  }

  return -1;
}

int ModelContentSkeleton::FindTinyGLTFMeshIndexByTinyGLTFNodeIndex(const tinygltf::Model& model, int nodeIndex) {
  if(nodeIndex == -1)
    return -1;

  auto const& node = model.nodes[nodeIndex];
  if(node.mesh != -1) {
    return nodeIndex;
  }
  else {
    size_t childCount = node.children.size();
    for(size_t i = 0; i < childCount; i++) {
      int meshIndex = FindTinyGLTFMeshIndexByTinyGLTFNodeIndex(model, node.children[i]);
      if(meshIndex != -1) {
        return meshIndex;
      }
    }
  }

  return -1;
}

size_t ModelContentSkeleton::FindBoneIndexByTinyGLTFJointIndex(const tinygltf::Model& model, size_t jointIndex) {
  if(model.skins.size() > 0) {
    auto const& skin = model.skins[0];
    if(jointIndex < skin.joints.size()) {
      auto const& joint = model.nodes[skin.joints[jointIndex]];
      if(auto it = boneLookupIndexByName.Find(joint.name)) {
        size_t boneIndex = it.value();
        ModelContentSkeletonBone& bone = bones[boneIndex];
        ApplyBoneAffectingVertices(bone.name);
        if(bone.actionPoseBoneIndex != PrimeNotFound) {
          return bone.actionPoseBoneIndex;
        }
        else {
          dbgprintf("[Warning] Bone does not have an action pose bone index: %s", joint.name.c_str());
          return 0;
        }
      }
      else {
        dbgprintf("[Warning] Could not find joint node by bone name: %s", joint.name.c_str());
        return 0;
      }
    }
    else {
      dbgprintf("[Warning] Invalid joint index for skin");
      return 0;
    }
  }
  else {
    dbgprintf("[Warning] Model has no skins.");
    return 0;
  }
}

size_t ModelContentSkeleton::FindBoneIndexByTinyGLTFMeshIndex(const tinygltf::Model& model, size_t meshIndex) {
  if(meshIndex >= model.meshes.size())
    return PrimeNotFound;

  size_t nodeCount = model.nodes.size();
  for(size_t i = 0; i < nodeCount; i++) {
    const auto& node = model.nodes[i];
    if(node.mesh == meshIndex) {
      std::string boneName = node.name;
      if(boneName.empty())
        boneName = GenerateBoneNameFromTinyGLTFNodeIndex(model, i);
      if(auto it = boneLookupIndexByName.Find(boneName)) {
        size_t boneIndex = it.value();
        ModelContentSkeletonBone& bone = bones[boneIndex];
        ApplyBoneAffectingVertices(bone.name);
        return bone.actionPoseBoneIndex;
      }
    }
  }

  return PrimeNotFound;
}

std::string ModelContentSkeleton::GenerateBoneNameFromTinyGLTFNodeIndex(const tinygltf::Model& model, size_t nodeIndex) {
  return string_printf("__node:%d", nodeIndex);
}

void ModelContentSkeleton::TraverseBoneHierarchy(const tinygltf::Model& model, int nodeIndex, Mat44 transformation, Mat44 parentTransformation, Dictionary<std::string, Mat44>& skinInverseBindTransforms) {
  if(nodeIndex < 0)
    return;

  const auto& node = model.nodes[nodeIndex];

  std::string nodeName = node.name;
  if(nodeName.empty())
    nodeName = GenerateBoneNameFromTinyGLTFNodeIndex(model, nodeIndex);

  Mat44 boneTransform = transformation;
  Mat44 thisTransformation;

  if(auto it = boneLookupIndexByName.Find(nodeName)) {
    size_t boneIndex = it.value();
    ModelContentSkeletonBone& bone = bones[boneIndex];

    if(node.translation.size() == 3) {
      bone.transformation.Translate((f32) node.translation[0], (f32) node.translation[1], (f32) node.translation[2]);
    }
    if(node.rotation.size() == 4) {
      Mat44 rotation = Quat((f32) node.rotation[0], (f32) node.rotation[1], (f32) node.rotation[2], (f32) node.rotation[3]).GetRotationMat44();
      bone.transformation.Multiply(rotation);
    }
    if(node.scale.size() == 3) {
      bone.transformation.Scale((f32) node.scale[0], (f32) node.scale[1], (f32) node.scale[2]);
    }
    if(node.matrix.size() == 16) {
      Mat44 nodeMatrix;
      CopyMatrix(nodeMatrix, node.matrix);
      bone.transformation.Multiply(nodeMatrix);
    }

    thisTransformation = bone.transformation;
    boneTransform = boneTransform * bone.transformation;
    
    if(node.mesh >= 0) {
      Mat44 transformationInv = transformation;
      transformationInv.Invert();
    }

    Stack<size_t> childBoneIndices;
    Stack<int> childNodes;

    size_t childCount = node.children.size();
    for(size_t i = 0; i < childCount; i++) {
      int nodeChildIndex = node.children[i];
      if(nodeChildIndex >= 0) {
        const auto& child = model.nodes[nodeChildIndex];
        std::string childName = child.name;
        if(childName.empty())
          childName = GenerateBoneNameFromTinyGLTFNodeIndex(model, nodeChildIndex);
        if(auto it = boneLookupIndexByName.Find(childName)) {
          size_t childIndex = it.value();
          childBoneIndices.Add(childIndex);
          childNodes.Add(nodeChildIndex);
        }
        else {
          PrimeAssert(false, "Expected to find bone in lookup table: name = %s", childName.c_str());
        }
      }
    }

    bone.childBoneIndexCount = childBoneIndices.GetCount();
    if(bone.childBoneIndexCount) {
      bone.childBoneIndices = new size_t[bone.childBoneIndexCount];
      if(bone.childBoneIndices) {
        size_t i = 0;
        for(size_t childIndex: childBoneIndices) {
          bone.childBoneIndices[i++] = childIndex;
        }
      }
      else {
        bone.childBoneIndexCount = 0;
      }
    }

    for(auto child: childNodes) {
      TraverseBoneHierarchy(model, child, boneTransform, thisTransformation, skinInverseBindTransforms);
    }
  }
}

void ModelContentSkeleton::EnsureKeyFramePose(ModelContentSkeletonActionKeyFrame& keyFrame, size_t keyFrameIndex, ModelContentSkeletonAction& action, Stack<ModelContentSkeletonPose>& createdPoses) {
  if(keyFrame.poseIndex == PrimeNotFound) {
    keyFrame.poseIndex = createdPoses.GetCount();
    ModelContentSkeletonPose createdPose;
    createdPose.name = string_printf("%s:%d", action.name.c_str(), keyFrame.poseIndex);
    createdPose.poseBoneCount = boneCount;
    createdPose.poseBones = new ModelContentSkeletonPoseBone[createdPose.poseBoneCount];
    createdPoses.Add(createdPose);
  }
}

void ModelContentSkeleton::EnsureKeyFrameTransformations(ModelContentSkeletonActionKeyFrame& keyFrame, size_t keyFrameIndex, ModelContentSkeletonAction& action, Stack<ModelContentSkeletonPose>& createdPoses) {
  if(keyFrameIndex > 0) {
    ModelContentSkeletonPose& pose = createdPoses[keyFrame.poseIndex];
    for(size_t i = 0; i < pose.poseBoneCount; i++) {
      ModelContentSkeletonPoseBone& poseBone = pose.poseBones[i];

      for(size_t j = 0; j < keyFrameIndex; j++) {
        if(poseBone.translationKnown && poseBone.scalingKnown && poseBone.rotationKnown) {
          break;
        }

        size_t prevKeyFrameIndex = keyFrameIndex - 1 - j;
        ModelContentSkeletonActionKeyFrame& prevKeyFrame = action.keyFrames[prevKeyFrameIndex];
        if(prevKeyFrame.poseIndex != PrimeNotFound) {
          ModelContentSkeletonPose& prevPose = createdPoses[prevKeyFrame.poseIndex];
          const ModelContentSkeletonPoseBone& prevBone = prevPose.GetPoseBone(i);
          if(!poseBone.translationKnown && prevBone.translationKnown) {
            poseBone.translation = prevBone.translation;
            poseBone.translationKnown = true;
          }

          if(!poseBone.scalingKnown && prevBone.scalingKnown) {
            poseBone.scaling = prevBone.scaling;
            poseBone.scalingKnown = true;
          }

          if(!poseBone.rotationKnown && prevBone.rotationKnown) {
            poseBone.rotation = prevBone.rotation;
            poseBone.rotationKnown = true;
          }
        }
      }

      if(poseBone.boneIndex == PrimeNotFound && (poseBone.translationKnown || poseBone.scalingKnown || poseBone.rotationKnown)) {
        poseBone.boneIndex = i;
      }
    }
  }
}

void ModelContentSkeleton::DestroyBones() {
  PrimeSafeDeleteArray(bones);
  boneCount = 0;
}

void ModelContentSkeleton::DestroyPoses() {
  PrimeSafeDeleteArray(poses);
  poseCount = 0;
}

void ModelContentSkeleton::DestroyActions() {
  PrimeSafeDeleteArray(actions);
  actionCount = 0;
}
