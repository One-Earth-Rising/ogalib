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

#include <Prime/Skeleton/SkeletonContent.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

#define PRIME_CONTENT_SKELETON_FPS_DEFAULT 60.0f

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

SkeletonContent::SkeletonContent():
fps(PRIME_CONTENT_SKELETON_FPS_DEFAULT),
bones(nullptr),
boneCount(0),
poses(nullptr),
poseCount(0),
actions(nullptr),
actionCount(0),
orderedBoneHierarchy(nullptr),
orderedBoneHierarchyRev(nullptr) {

}

SkeletonContent::~SkeletonContent() {
  if(actions) {
    for(size_t i = 0; i < actionCount; i++) {
      SkeletonContentAction& action = actions[i];

      if(action.keyFrames) {
        for(size_t j = 0; j < action.keyFrameCount; j++) {
          SkeletonContentActionKeyFrame& keyFrame = action.keyFrames[j];

          PrimeSafeDeleteArray(keyFrame.pieceActionMappings);
        }

        PrimeSafeDeleteArray(action.keyFrames);
      }
    }

    PrimeSafeDeleteArray(actions);
  }

  if(poses) {
    for(size_t i = 0; i < poseCount; i++) {
      SkeletonContentPose& pose = poses[i];

      PrimeSafeDeleteArray(pose.bones);
      PrimeSafeDeleteArray(pose.boneTransforms);
    }

    PrimeSafeDeleteArray(poses);
  }

  PrimeSafeDeleteArray(bones);

  PrimeSafeFree(orderedBoneHierarchy);
  PrimeSafeFree(orderedBoneHierarchyRev);
}

bool SkeletonContent::Load(const json& data, const json& info) {
  if(!Content::Load(data, info))
    return false;

  if(!data.IsObject())
    return false;

  if(auto itSkinset = data.find("skinset")) {
    skinset = itSkinset.GetString();
  }

  fps = PRIME_CONTENT_SKELETON_FPS_DEFAULT;
  if(auto itFPS = data.find("fps")) {
    auto& value = itFPS.value();
    if(value.IsNumber()) {
      fps = value.GetFloat();
    }
  }

  Stack<SkeletonContentBone*> parsedBones;
  Stack<SkeletonContentPose*> parsedPoses;
  Dictionary<SkeletonContentPose*, Stack<SkeletonContentPoseBone*>*> parsedPoseBonesLookup;
  Dictionary<SkeletonContentPoseBone*, SkeletonContentPoseBoneTransform*> parsedPoseBoneTransformsLookup;
  Stack<SkeletonContentAction*> parsedActions;
  Dictionary<SkeletonContentAction*, Stack<SkeletonContentActionKeyFrame*>*> parsedActionKeyFramesLookup;
  Dictionary<SkeletonContentActionKeyFrame*, Stack<SkeletonContentActionKeyFramePieceActionMapping*>*> parsedActionKeyFramePieceActionMappingLookup;
  Stack<size_t> parsedOrderedBoneHierarchy;
  Stack<size_t> parsedOrderedBoneHierarchyRev;

  if(auto itBones = data.find("bones")) {
    if(itBones.IsArray()) {
      for(auto& bone: itBones) {
        if(bone.IsObject()) {
          SkeletonContentBone* parsedBone = new SkeletonContentBone();
          if(parsedBone) {
            if(auto it = bone.find("name"))
              parsedBone->name = it.GetString();

            if(auto it = bone.find("parent"))
              parsedBone->parent = it.GetString();

            if(auto it = bone.find("parentIndex")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedBone->parentIndex = value.GetInt();
              }
              else {
                parsedBone->parentIndex = atoi(it.c_str());
              }
            }

            if(auto it = bone.find("tip")) {
              auto& value = it.value();
              if(value.IsBool()) {
                parsedBone->tip = value.GetBool();
              }
            }

            if(auto it = bone.find("size")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedBone->size = value.GetFloat();
              }
            }

            if(auto it = bone.find("depth")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedBone->depth = value.GetFloat();
              }
            }

            if(auto it = bone.find("cancelActionBlend")) {
              auto& value = it.value();
              if(value.IsBool()) {
                parsedBone->cancelActionBlend = value.GetBool();
              }
            }

            parsedBones.Add(parsedBone);
          }
        }
      }
    }
  }

  if(auto itPoses = data.find("poses")) {
    if(itPoses.IsArray()) {
      for(auto& pose: itPoses) {
        if(pose.IsObject()) {
          SkeletonContentPose* parsedPose = new SkeletonContentPose();
          if(parsedPose) {
            if(auto it = pose.find("name"))
              parsedPose->name = it.GetString();

            if(auto itPoseBones = pose.find("bones")) {
              if(itPoseBones.IsArray()) {
                Stack<SkeletonContentPoseBone*>* parsedPoseBones = new Stack<SkeletonContentPoseBone*>();
                if(parsedPoseBones) {
                  parsedPoseBonesLookup[parsedPose] = parsedPoseBones;

                  for(auto& poseBone: itPoseBones) {
                    if(poseBone.IsObject()) {
                      SkeletonContentPoseBone* parsedPoseBone = new SkeletonContentPoseBone();
                      if(parsedPoseBone) {
                        if(auto it = poseBone.find("name"))
                          parsedPoseBone->name = it.GetString();

                        if(auto it = poseBone.find("angle")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedPoseBone->angle = value.GetFloat();
                          }
                        }

                        if(auto it = poseBone.find("scaleX")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedPoseBone->scaleX = value.GetFloat();
                          }
                        }

                        if(auto it = poseBone.find("scaleY")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedPoseBone->scaleY = value.GetFloat();
                          }
                        }

                        if(auto it = poseBone.find("x")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedPoseBone->x = value.GetFloat();
                          }
                        }

                        if(auto it = poseBone.find("y")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedPoseBone->y = value.GetFloat();
                          }
                        }

                        if(auto it = poseBone.find("depth")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedPoseBone->depth = value.GetFloat();
                          }
                        }

                        if(auto it = poseBone.find("alpha")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedPoseBone->alpha = value.GetFloat();
                          }
                        }

                        if(auto it = poseBone.find("alphaInterpolate")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedPoseBone->alphaInterpolate = value.GetFloat();
                          }
                        }

                        if(auto it = poseBone.find("alphaInterpolateAnchor")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedPoseBone->alphaInterpolateAnchor = (SkeletonPoseInterpolateAnchor) value.GetInt();
                          }
                          else if(value.IsString()) {
                            parsedPoseBone->alphaInterpolateAnchor = GetEnumSkeletonPoseInterpolateAnchorFromString(it.GetString());
                          }
                        }

                        if(auto itPoseBoneTransform = poseBone.find("transform")) {
                          if(itPoseBoneTransform.IsObject()) {
                            SkeletonContentPoseBoneTransform* parsedPoseBoneTransform = new SkeletonContentPoseBoneTransform();
                            if(parsedPoseBoneTransform) {
                              if(auto it = itPoseBoneTransform.find("x")) {
                                auto& value = it.value();
                                if(value.IsNumber()) {
                                  parsedPoseBoneTransform->x = value.GetFloat();
                                }
                              }

                              if(auto it = itPoseBoneTransform.find("y")) {
                                auto& value = it.value();
                                if(value.IsNumber()) {
                                  parsedPoseBoneTransform->y = value.GetFloat();
                                }
                              }

                              if(auto it = itPoseBoneTransform.find("dx")) {
                                auto& value = it.value();
                                if(value.IsNumber()) {
                                  parsedPoseBoneTransform->dx = value.GetFloat();
                                }
                              }

                              if(auto it = itPoseBoneTransform.find("dy")) {
                                auto& value = it.value();
                                if(value.IsNumber()) {
                                  parsedPoseBoneTransform->dy = value.GetFloat();
                                }
                              }

                              if(auto it = itPoseBoneTransform.find("angle")) {
                                auto& value = it.value();
                                if(value.IsNumber()) {
                                  parsedPoseBoneTransform->angle = value.GetFloat();
                                }
                              }

                              if(auto it = itPoseBoneTransform.find("scaleX")) {
                                auto& value = it.value();
                                if(value.IsNumber()) {
                                  parsedPoseBoneTransform->scaleX = value.GetFloat();
                                }
                              }

                              if(auto it = itPoseBoneTransform.find("scaleY")) {
                                auto& value = it.value();
                                if(value.IsNumber()) {
                                  parsedPoseBoneTransform->scaleY = value.GetFloat();
                                }
                              }

                              if(auto it = itPoseBoneTransform.find("alpha")) {
                                auto& value = it.value();
                                if(value.IsNumber()) {
                                  parsedPoseBoneTransform->alpha = value.GetFloat();
                                }
                              }

                              parsedPoseBoneTransformsLookup[parsedPoseBone] = parsedPoseBoneTransform;
                            }
                          }
                        }

                        parsedPoseBones->Add(parsedPoseBone);
                      }
                    }
                  }
                }
              }
            }

            parsedPoses.Add(parsedPose);
          }
        }
      }
    }
  }

  if(auto itActions = data.find("actions")) {
    if(itActions.IsArray()) {
      for(auto& action: itActions) {
        if(action.IsObject()) {
          SkeletonContentAction* parsedAction = new SkeletonContentAction();
          if(parsedAction) {
            if(auto it = action.find("name"))
              parsedAction->name = it.GetString();

            if(auto it = action.find("x")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedAction->x = value.GetFloat();
              }
            }

            if(auto it = action.find("y")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedAction->y = value.GetFloat();
              }
            }

            if(auto it = action.find("z")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedAction->z = value.GetFloat();
              }
            }

            if(auto it = action.find("loop")) {
              auto& value = it.value();
              if(value.IsBool()) {
                parsedAction->loop = value.GetBool();
              }
            }

            if(auto it = action.find("interruptible")) {
              auto& value = it.value();
              if(value.IsBool()) {
                parsedAction->interruptible = value.GetBool();
              }
            }

            if(auto it = action.find("interruptTime")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedAction->interruptTime = value.GetFloat();
              }
            }

            if(auto it = action.find("skipRecoil")) {
              auto& value = it.value();
              if(value.IsBool()) {
                parsedAction->skipRecoil = value.GetBool();
              }
            }

            if(auto it = action.find("nextAction"))
              parsedAction->nextAction = it.GetString();

            if(auto it = action.find("lastPoseBlendTimeSpecified")) {
              auto& value = it.value();
              if(value.IsBool()) {
                parsedAction->lastPoseBlendTimeSpecified = value.GetBool();
              }
            }

            if(auto it = action.find("lastPoseBlendTime")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedAction->lastPoseBlendTime = value.GetFloat();
              }
            }

            if(auto it = action.find("nextPoseBlendAllowed")) {
              auto& value = it.value();
              if(value.IsBool()) {
                parsedAction->nextPoseBlendAllowed = value.GetBool();
              }
            }

            if(auto itKeyFrames = action.find("keyFrames")) {
              if(itKeyFrames.IsArray()) {
                Stack<SkeletonContentActionKeyFrame*>* parsedActionKeyFrames = new Stack<SkeletonContentActionKeyFrame*>();
                if(parsedActionKeyFrames) {
                  parsedActionKeyFramesLookup[parsedAction] = parsedActionKeyFrames;

                  for(auto& keyFrame: itKeyFrames) {
                    if(keyFrame.IsObject()) {
                      SkeletonContentActionKeyFrame* parsedActionKeyFrame = new SkeletonContentActionKeyFrame();
                      if(parsedActionKeyFrame) {
                        if(auto it = keyFrame.find("len")) {
                          auto& value = it.value();
                          if(value.IsUint()) {
                            parsedActionKeyFrame->len = value.GetUint();
                          }
                        }

                        if(auto it = keyFrame.find("pose"))
                          parsedActionKeyFrame->pose = it.GetString();

                        if(auto itPieceActionMappings = keyFrame.find("pieceActionMappings")) {
                          if(itPieceActionMappings.IsArray()) {
                            Stack<SkeletonContentActionKeyFramePieceActionMapping*>* parsedActionKeyFramePieceActionMappings = new Stack<SkeletonContentActionKeyFramePieceActionMapping*>();
                            if(parsedActionKeyFramePieceActionMappings) {
                              parsedActionKeyFramePieceActionMappingLookup[parsedActionKeyFrame] = parsedActionKeyFramePieceActionMappings;

                              for(auto& pieceActionMapping: itPieceActionMappings) {
                                if(pieceActionMapping.IsObject()) {
                                  SkeletonContentActionKeyFramePieceActionMapping* parsedActionKeyFramePieceActionMapping = new SkeletonContentActionKeyFramePieceActionMapping();
                                  if(parsedActionKeyFramePieceActionMapping) {
                                    if(auto it = pieceActionMapping.find("piece"))
                                      parsedActionKeyFramePieceActionMapping->piece = it.GetString();

                                    if(auto it = pieceActionMapping.find("action"))
                                      parsedActionKeyFramePieceActionMapping->action = it.GetString();

                                    parsedActionKeyFramePieceActionMappings->Add(parsedActionKeyFramePieceActionMapping);
                                  }
                                }
                              }
                            }
                          }
                        }

                        parsedActionKeyFrames->Add(parsedActionKeyFrame);
                      }
                    }
                  }
                }
              }
            }

            parsedActions.Add(parsedAction);
          }
        }
      }
    }
  }

  if(auto itOrderedBoneHierarchy = data.find("orderedBoneHierarchy")) {
    if(itOrderedBoneHierarchy.IsArray()) {
      for(auto& it: itOrderedBoneHierarchy) {
        if(it.IsSizeT()) {
          size_t index = it.GetSizeT();
          parsedOrderedBoneHierarchy.Add(index);
        }
      }
    }
  }

  if(auto itOrderedBoneHierarchyRev = data.find("orderedBoneHierarchyRev")) {
    if(itOrderedBoneHierarchyRev.IsArray()) {
      for(auto& it: itOrderedBoneHierarchyRev) {
        if(it.IsSizeT()) {
          size_t index = it.GetSizeT();
          parsedOrderedBoneHierarchyRev.Add(index);
        }
      }
    }
  }

  boneCount = parsedBones.GetCount();
  if(boneCount) {
    bones = new SkeletonContentBone[boneCount];
    orderedBoneHierarchy = (size_t*) calloc(boneCount, sizeof(size_t));
    orderedBoneHierarchyRev = (size_t*) calloc(boneCount, sizeof(size_t));

    for(size_t i = 0; i < boneCount; i++) {
      SkeletonContentBone& bone = bones[i];
      SkeletonContentBone* parsedBone = parsedBones[i];

      bone = *parsedBone;

      PrimeSafeDelete(parsedBone);
    }
  }

  poseCount = parsedPoses.GetCount();
  if(poseCount) {
    poses = new SkeletonContentPose[poseCount];

    for(size_t i = 0; i < poseCount; i++) {
      SkeletonContentPose& pose = poses[i];
      SkeletonContentPose* parsedPose = parsedPoses[i];

      pose = *parsedPose;

      if(auto it = parsedPoseBonesLookup.Find(parsedPose)) {
        Stack<SkeletonContentPoseBone*>* parsedPoseBones = it.value();

        size_t poseBoneCount = parsedPoseBones->GetCount();
        if(poseBoneCount) {
          pose.bones = new SkeletonContentPoseBone[poseBoneCount];
          pose.boneTransforms = new SkeletonContentPoseBoneTransform[poseBoneCount];

          for(size_t j = 0; j < poseBoneCount; j++) {
            SkeletonContentPoseBone& poseBone = pose.bones[j];
            SkeletonContentPoseBone* parsedPoseBone = (*parsedPoseBones)[j];

            poseBone = *parsedPoseBone;

            if(auto it = parsedPoseBoneTransformsLookup.Find(parsedPoseBone)) {
              SkeletonContentPoseBoneTransform& poseBoneTransform = pose.boneTransforms[j];
              SkeletonContentPoseBoneTransform* parsedPoseBoneTransform = it.value();

              poseBoneTransform = *parsedPoseBoneTransform;

              PrimeSafeDelete(parsedPoseBoneTransform);
            }

            PrimeSafeDelete(parsedPoseBone);
          }
        }

        PrimeSafeDelete(parsedPoseBones);
      }

      PrimeSafeDelete(parsedPose);
    }
  }

  actionCount = parsedActions.GetCount();
  if(actionCount) {
    actions = new SkeletonContentAction[actionCount];

    for(size_t i = 0; i < actionCount; i++) {
      SkeletonContentAction& action = actions[i];
      SkeletonContentAction* parsedAction = parsedActions[i];

      action = *parsedAction;

      if(auto it = parsedActionKeyFramesLookup.Find(parsedAction)) {
        Stack<SkeletonContentActionKeyFrame*>* parsedActionKeyFrames = it.value();

        action.keyFrameCount = parsedActionKeyFrames->GetCount();
        if(action.keyFrameCount) {
          action.keyFrames = new SkeletonContentActionKeyFrame[action.keyFrameCount];

          for(size_t j = 0; j < action.keyFrameCount; j++) {
            SkeletonContentActionKeyFrame& actionKeyFrame = action.keyFrames[j];
            SkeletonContentActionKeyFrame* parsedActionKeyFrame = (*parsedActionKeyFrames)[j];

            actionKeyFrame = *parsedActionKeyFrame;

            if(auto it = parsedActionKeyFramePieceActionMappingLookup.Find(parsedActionKeyFrame)) {
              Stack<SkeletonContentActionKeyFramePieceActionMapping*>* parsedActionKeyFramePieceActionMappings = it.value();
              actionKeyFrame.pieceActionMappingCount = parsedActionKeyFramePieceActionMappings->GetCount();

              if(actionKeyFrame.pieceActionMappingCount) {
                actionKeyFrame.pieceActionMappings = new SkeletonContentActionKeyFramePieceActionMapping[actionKeyFrame.pieceActionMappingCount];

                for(size_t k = 0; k < actionKeyFrame.pieceActionMappingCount; k++) {
                  SkeletonContentActionKeyFramePieceActionMapping& actionKeyFramePieceActionMapping = actionKeyFrame.pieceActionMappings[k];
                  SkeletonContentActionKeyFramePieceActionMapping* parsedActionKeyFramePieceActionMapping = (*parsedActionKeyFramePieceActionMappings)[k];

                  actionKeyFramePieceActionMapping = *parsedActionKeyFramePieceActionMapping;

                  PrimeSafeDelete(parsedActionKeyFramePieceActionMapping);
                }
              }

              PrimeSafeDelete(parsedActionKeyFramePieceActionMappings);
            }

            PrimeSafeDelete(parsedActionKeyFrame);
          }
        }

        PrimeSafeDelete(parsedActionKeyFrames);
      }

      PrimeSafeDelete(parsedAction);
    }
  }

  {
    size_t i = 0;
    for(auto value: parsedOrderedBoneHierarchy) {
      if(value < boneCount) {
        orderedBoneHierarchy[i] = value;
      }
      i++;
    }
  }

  {
    size_t i = 0;
    for(auto value: parsedOrderedBoneHierarchyRev) {
      if(value < boneCount) {
        orderedBoneHierarchyRev[i] = value;
      }
      i++;
    }
  }

  // Loading complete at this point.  Perform value indexing below for optimizations.
  for(size_t i = 0; i < actionCount; i++) {
    SkeletonContentAction& action = actions[i];
    if(action.keyFrameCount) {
      for(size_t j = 0; j < action.keyFrameCount; j++) {
        SkeletonContentActionKeyFrame& keyFrame = action.keyFrames[j];
        keyFrame.poseIndex = GetPoseIndex(keyFrame.pose);
        PrimeAssert(keyFrame.poseIndex != PrimeNotFound, "Could not find index.");
      }
    }
  }

  if(boneCount) {
    for(size_t i = 0; i < poseCount; i++) {
      SkeletonContentPose& pose = poses[i];
      for(size_t j = 0; j < boneCount; j++) {
        SkeletonContentPoseBone& poseBone = pose.bones[j];
        pose.bones[j].boneLookupIndex = GetPoseBoneIndex(&pose, poseBone.name);
        PrimeAssert(pose.bones[j].boneLookupIndex != PrimeNotFound, "Could not find index.");
      }
    }
  }

  return true;
}

const SkeletonContentBone* SkeletonContent::FindBone(const std::string& name) const {
  for(size_t i = 0; i < boneCount; i++) {
    if(name == bones[i].name) {
      return &bones[i];
    }
  }

  return NULL;
}

const SkeletonContentPose* SkeletonContent::FindPose(const std::string& name) const {
  for(size_t i = 0; i < poseCount; i++) {
    if(name == poses[i].name) {
      return &poses[i];
    }
  }

  return NULL;
}

const SkeletonContentPoseBone* SkeletonContent::FindPoseBone(const SkeletonContentPose* pose, const std::string& name) const {
  if(!name.empty())
    return NULL;

  for(size_t i = 0; i < boneCount; i++) {
    if(name == pose->bones[i].name) {
      return &pose->bones[i];
    }
  }

  return NULL;
}

bool SkeletonContent::IsBoneDescendant(size_t boneIndex, size_t ancestorIndex) const {
  size_t index = boneIndex;
  while(index != PrimeNotFound) {
    const SkeletonContentBone& bone = bones[index];
    if(bone.parentIndex == ancestorIndex) {
      return true;
    }
    else {
      index = bone.parentIndex;
    }
  }

  return false;
}

size_t SkeletonContent::GetBoneIndex(const std::string& name) const {
  if(!name.empty()) {
    for(size_t i = 0; i < boneCount; i++) {
      if(name == bones[i].name)
        return i;
    }
  }

  return PrimeNotFound;
}

size_t SkeletonContent::GetPoseIndex(const std::string& name) const {
  if(!name.empty()) {
    for(size_t i = 0; i < poseCount; i++) {
      if(name == poses[i].name)
        return i;
    }
  }

  return PrimeNotFound;
}

size_t SkeletonContent::GetPoseBoneIndex(const SkeletonContentPose* pose, const std::string& name) const {
  if(!name.empty()) {
    for(size_t i = 0; i < boneCount; i++) {
      if(name == pose->bones[i].name) {
        return i;
      }
    }
  }

  return PrimeNotFound;
}

const size_t SkeletonContent::GetBoneIndexFromOrderedHierarchy(size_t index, bool rev) const {
  if(orderedBoneHierarchy && orderedBoneHierarchyRev)
    return rev ? orderedBoneHierarchyRev[index % boneCount] : orderedBoneHierarchy[index % boneCount];
  else
    return 0;
}
