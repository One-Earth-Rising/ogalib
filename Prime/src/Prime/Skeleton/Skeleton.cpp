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

#include <Prime/Skeleton/Skeleton.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>
#include <Prime/Imagemap/Imagemap.h>
#include <Prime/Graphics/Graphics.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PRIME_SKELETON_DEFAULT_LAST_POSE_BLEND_TIME 0.1f

#define PRIME_SKELETON_ADD_SKINSET_STACK_CAPACITY_INITIAL 4
#define PRIME_SKELETON_ADD_SKINSET_STACK_CAPACITY_GROW PRIME_SKELETON_ADD_SKINSET_STACK_CAPACITY_INITIAL

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

typedef struct _SkeletonVertex {
  f32 x, y;
  f32 u, v;
  f32 boneIndex, texIndex;
} SkeletonVertex;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class SkeletonWeightSortItem {
public:

  refptr<Skeleton> skeleton;
  f32 weight;

  SkeletonWeightSortItem(refptr<Skeleton> skeleton, f32 weight = 0.0f): skeleton(skeleton), weight(weight) {}
  SkeletonWeightSortItem(const SkeletonWeightSortItem& other) {
    skeleton = other.skeleton;
    weight = other.weight;
  }

  SkeletonWeightSortItem& operator=(const SkeletonWeightSortItem& other) {
    skeleton = other.skeleton;
    weight = other.weight;

    return *this;
  }
  
  bool operator<(const SkeletonWeightSortItem& other) const {
    return weight > other.weight;
  }

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Skeleton::Skeleton():
lastDepthSortedBoneIndices(nullptr),
boneSkinsetAffixesBoneCount(0),
additionalSkinsets(nullptr),
additionalSkinsetActiveBones(nullptr),
lastActionPoseBlendCtr(0.0f),
lastActionPoseBlendTime(0.0f),
nextActionPoseBlendCanceled(false),
knownActionPose1(nullptr),
knownActionPose2(nullptr),
knownPoseBlendWeight(0.0f),
knownActionKeyFrame(nullptr),
boneOverrides(nullptr),
actionIndex(PrimeNotFound),
actionChanged(false),
actionTimeScale(1.0f),
actionCtr(0.0f),
actionLen(0.0f),
actionLoopCount(0),
actionPlayed(false),
actionReverse(false),
cacheBoneTransformsFrame(0),
calcPoseTime(0.0f),
processingMode(SkeletonProcessingModeNone),
thisBoneCount(0),
thisPieceCount(0),
totalBoneCount(0),
totalPieceCount(0),
totalTexCount(0),
skeletonBoneLookup(nullptr),
skeletonBoneRootTransforms(nullptr),
pieceSignatures(nullptr),
bonePieceSignatureIndices(nullptr),
bonePieceSignatureIndicesCount(0),
pieceSignaturesOutdated(false),
shaderDataReady(false),
updateVertexSpan(true),
programDataBoneCount(0),
vertexMin(Vec3(0.0f, 0.0f, 0.0f)),
vertexMax(Vec3(0.0f, 0.0f, 0.0f)) {
  localMat.LoadIdentity();
}

Skeleton::~Skeleton() {
  DestroyAllBoneSkinsetAffixes();
  DestroyPieceSignatures();

  PrimeSafeDeleteArray(lastDepthSortedBoneIndices);
  PrimeSafeDelete(additionalSkinsets);
  PrimeSafeDeleteArray(additionalSkinsetActiveBones);
  PrimeSafeDeleteArray(boneOverrides);
}

void Skeleton::SetContent(Content* content) {
  SetContent(dynamic_cast<SkeletonContent*>(content));
}

void Skeleton::SetContent(SkeletonContent* content) {
  DestroyAllBoneSkinsetAffixes();
  DestroyPieceSignatures();

  PrimeSafeDeleteArray(lastDepthSortedBoneIndices);
  PrimeSafeDelete(additionalSkinsets);
  PrimeSafeDeleteArray(additionalSkinsetActiveBones);
  PrimeSafeDeleteArray(boneOverrides);

  skinset = nullptr;

  depthSortedItems.Clear();
  boneDepthUpdated = false;

  boneSkinsetAffixes.Clear();
  boneSkinsetAffixesBoneCount = 0;

  lastActionPoseBlendCtr = 0.0f;
  lastActionPoseBlendTime = 0.0f;
  nextActionPoseBlendCanceled = false;
  knownActionPose1 = nullptr;
  knownActionPose2 = nullptr;
  knownPoseBlendWeight = 0.0f;
  knownActionKeyFrame = nullptr;
  boneCancelActionBlend.Clear();
  boneCancelNextActionBlend.Clear();
  actionIndex = PrimeNotFound;
  actionChanged = false;
  actionTimeScale = 1.0f;
  actionCtr = 0.0f;
  actionLen = 0.0f;
  actionLoopCount = 0;
  actionPlayed = false;
  actionReverse = false;

  cacheBoneTransformsFrame = 0;

  calcPoseTime = 0.0f;

  processingMode = SkeletonProcessingModeNone;

  localMat.LoadIdentity();

  ab = nullptr;
  ib = nullptr;
  thisBoneCount = 0;
  thisPieceCount = 0;
  totalBoneCount = 0;
  totalPieceCount = 0;
  totalTexCount = 0;
  bufferTexLookup.Clear();
  bonePieceSignatureIndicesCount = 0;
  pieceSignaturesOutdated = false;
  shaderDataReady = false;
  updateVertexSpan = true;

  programDataBoneCount = 0;

  vertexMin = Vec3(0.0f, 0.0f, 0.0f);
  vertexMax = Vec3(0.0f, 0.0f, 0.0f);

  this->content = content;

  if(!content)
    return;

  size_t boneCount = content->GetBoneCount();

  lastDepthSortedBoneIndices = new size_t[boneCount];

  for(size_t i = 0; i < boneCount; i++)
    depthSortedItems.Push(SkeletonDepthSortItem(i));

  currActionPose1.SetContent(content);
  currActionPose2.SetContent(content);
  currActionPoseI.SetContent(content);
  lastActionPose.SetContent(content);
  lastActionPoseTemp.SetContent(content);
  knownActionPose1 = nullptr;
  knownActionPose2 = nullptr;
  knownPoseBlendWeight = 0.0f;

  SetActionByIndex(0);

  lastActionPose.Copy(currActionPoseI);
  lastActionPoseBlendCtr = 0.0f;
  lastActionPoseBlendTime = 0.0f;

  CalcPose(0.0f);
}

void Skeleton::SetSkinset(refptr<Skinset> skinset) {
  updateVertexSpan = true;

  if(this->skinset) {
    if(this->skinset->GetSkinsetContent() == skinset->GetSkinsetContent())
      return;

    if(processingMode == SkeletonProcessingModeShaderWithPoseVariables)
      SetProcessingMode(SkeletonProcessingModeNone);

    DestroyBoneSkinsetAffixes(this->skinset);
  }

  this->skinset = nullptr;

  if(!HasContent())
    return;

  this->skinset = skinset;

  if(!skinset || !skinset->HasContent())
    return;

  skinset->SetAction(GetActionName(), knownActionKeyFrame, true);

  size_t boneCount = content->GetBoneCount();

  refptr<SkinsetContent> skinsetContent = skinset->GetSkinsetContent();
  const SkinsetContentPiece* contentPieces = skinsetContent->GetPieces();
  size_t skinsetPieceCount = skinset->GetPieceCount();

  for(size_t i = 0; i < skinsetPieceCount; i++) {
    const SkinsetContentPiece& contentPiece = contentPieces[i];
    size_t boneIndex = content->GetBoneIndex(contentPiece.affix);
    PrimeAssert(boneIndex < boneCount || boneIndex == PrimeNotFound, "Invalid bone.");
    skinset->SetPieceBoneIndex(i, boneIndex);
    if(boneIndex == PrimeNotFound) {
      skinset->SetPieceParentBoneIndex(i, PrimeNotFound);
    }
    else {
      const SkeletonContentBone* bone = &content->GetBones()[boneIndex];
      size_t parentBoneIndex = content->GetBoneIndex(bone->parent);
      skinset->SetPieceParentBoneIndex(i, parentBoneIndex);
    }
  }

  CreateBoneSkinsetAffixes(skinset);
  CacheBoneTransforms(true);

  SetProcessingMode(SkeletonProcessingModeShaderWithPoseVariables);

  Calc(0.0f);
  bufferTexLookup.Clear();
  pieceSignaturesOutdated = true;
  shaderDataReady = false;
  UpdateBufferPieces();
  UpdateBufferPose();
}

refptr<Skinset> Skeleton::GetSkinset() const {
  return skinset;
}

bool Skeleton::HasSkinset() const {
  return skinset;
}

void Skeleton::SetAdditionalSkinset(const std::string& name, refptr<Skinset> skinset) {
  SetAdditionalSkinset(name, skinset, {});
}

void Skeleton::SetAdditionalSkinset(const std::string& name, refptr<Skinset> skinset, const Stack<std::string>& activeBones) {
  PrimeAssert(!name.empty(), "Invalid name for additional skinset.");
  if(name.empty())
    return;

  if(!HasContent())
    return;

  updateVertexSpan = true;

  if(HasAdditionalSkinsets()) {
    if(additionalSkinsets->HasKey(name)) {
      size_t boneCount = content->GetBoneCount();
      PrimeAssert(boneCount, "Skeleton has no bones.");

      size_t inactiveCount = 0;

      for(size_t i = 0; i < boneCount; i++) {
        if(additionalSkinsetActiveBones[i] == name) {
          additionalSkinsetActiveBones[i].clear();
          inactiveCount++;
        }
        else if(additionalSkinsetActiveBones[i].empty()) {
          inactiveCount++;
        }
      }

      additionalSkinsets->Remove(name);

      if(additionalSkinsets->GetCount() == 0) {
        PrimeSafeDelete(additionalSkinsets);
      }

      if(inactiveCount == boneCount) {
        PrimeSafeDeleteArray(additionalSkinsetActiveBones);
      }

      UpdateReferencedBoneSkinsetAffixes();
    }
  }

  if(!skinset || !skinset->HasContent())
    return;

  if(!additionalSkinsets) {
    additionalSkinsets = new Dictionary<std::string, refptr<Skinset>>();
  }

  size_t boneCount = content->GetBoneCount();
  PrimeAssert(boneCount, "Skeleton has no bones.");

  if(!additionalSkinsetActiveBones) {
    additionalSkinsetActiveBones = new std::string[boneCount];
    for(size_t i = 0; i < boneCount; i++)
      additionalSkinsetActiveBones[i].clear();
  }

  if(additionalSkinsets && additionalSkinsetActiveBones) {
    (*additionalSkinsets)[name] = skinset;

    CreateBoneSkinsetAffixes(skinset);

    for(auto& activeBone: activeBones) {
      size_t boneIndex = content->GetBoneIndex(activeBone);
      if(boneIndex != PrimeNotFound) {
        additionalSkinsetActiveBones[boneIndex] = name;
      }
    }
  }

  if(processingMode == SkeletonProcessingModeShaderWithPoseVariables) {
    CreatePieceSignatures();
  }
  
  nextActionPoseBlendCanceled = true;
  SetActionByIndex(actionIndex);
}

bool Skeleton::HasAdditionalSkinsets() const {
  return additionalSkinsets && additionalSkinsetActiveBones && additionalSkinsets->GetCount() > 0;
}

void Skeleton::DestroyAdditionalSkinset(const std::string& name) {
  SetAdditionalSkinset(name, nullptr);
}

void Skeleton::DestroyAdditionalSkinsets() {
  PrimeSafeDelete(additionalSkinsets);
  PrimeSafeDeleteArray(additionalSkinsetActiveBones);

  UpdateReferencedBoneSkinsetAffixes();

  if(processingMode == SkeletonProcessingModeShaderWithPoseVariables) {
    CreatePieceSignatures();
  }

  nextActionPoseBlendCanceled = true;
  updateVertexSpan = true;
}

refptr<Skinset> Skeleton::GetSkinsetForBone(size_t boneIndex) {
  if(HasAdditionalSkinsets()) {
    const std::string& skinsetName = additionalSkinsetActiveBones[boneIndex];
    if(auto it = additionalSkinsets->Find(skinsetName)) {
      return it.value();
    }
  }

  return GetSkinset();
}

void Skeleton::SetDefaultSkinset() {
  skinset = nullptr;

  if(HasContent()) {
    GetContent(content->GetSkinset(), [=](Content* content) {
      if(content->IsInstance<SkinsetContent>()) {
        Skinset* skinset = new Skinset();
        skinset->SetContent(content);
        SetSkinset(skinset);
      }
    });
  }
}

void Skeleton::Calc(f32 dt) {
  if(!HasContent())
    return;

  if(content->GetActionCount() == 0)
    return;

  f32 lastActionCtr = actionCtr;
  actionCtr += dt * actionTimeScale;

  if(actionLen > 0.0f) {
    while(actionCtr >= actionLen) {
      const SkeletonContentAction& action = content->GetAction(actionIndex);
      ResetActionChanged();
      if(!HasActionChanged()) {
        if(!action.loop && !action.nextAction.empty()) {
          SetAction(action.nextAction);
          break;
        }

        if(actionCtr >= actionLen) {
          if(action.loop) {
            actionCtr -= actionLen;
            actionLoopCount++;
          }
          else {
            actionCtr = actionLen;
          }
        }

        actionPlayed = true;

        if(actionCtr >= actionLen && !action.loop) {
          break;
        }
      }
    }
  }

  CalcPose(dt);

  if(processingMode == SkeletonProcessingModeShaderWithPoseVariables) {
    size_t texCount = bufferTexLookup.GetCount();
    if(texCount > totalTexCount) {
      totalTexCount = texCount;
    }
  }
}

void Skeleton::CalcPose(f32 dt) {
  bool updatedLastActionPoseBlend = false;

  if(lastActionPoseBlendCtr) {
    updatedLastActionPoseBlend = true;
    lastActionPoseBlendCtr -= dt;
    if(lastActionPoseBlendCtr < 0.0f) {
      lastActionPoseBlendCtr = 0.0f;
      lastActionPoseBlendTime = 0.0f;
    }
  }

  if(skinset) {
    skinset->Calc(dt);

    if(HasAdditionalSkinsets()) {
      for(auto it: *additionalSkinsets) {
        refptr<Skinset> additionalSkinset = it.value();
        additionalSkinset->Calc(dt);
      }
    }
  }

  if(processingMode == SkeletonProcessingModeShaderWithPoseVariables) {
    const SkeletonContentPose* pose1 = nullptr;
    const SkeletonContentPose* pose2 = nullptr;

    GetActionFramePoses(&pose1, &pose2, &knownPoseBlendWeight, &knownActionKeyFrame);
    
    PerformBoneDepthSort(pose1, pose2, knownPoseBlendWeight);

    if(!knownActionPose1 || knownActionPose1 != pose1) {
      knownActionPose1 = pose1;
      currActionPose1.Copy(*pose1);
    }

    if(!knownActionPose2 || knownActionPose2 != pose2) {
      knownActionPose2 = pose2;
      currActionPose2.Copy(*pose2);
    }

    currActionPoseI.Interpolate(currActionPose1, currActionPose2, knownPoseBlendWeight);

    if(lastActionPoseBlendCtr > 0.0f && lastActionPoseBlendTime > 0.0f) {
      f32 t = lastActionPoseBlendCtr / lastActionPoseBlendTime;
      lastActionPoseTemp.Copy(currActionPoseI);
      currActionPoseI.Interpolate(lastActionPoseTemp, lastActionPose, t, nullptr, &boneCancelActionBlend);
    }

    UpdateBufferPieces();
    UpdateBufferPose();
  }
  else if(processingMode == SkeletonProcessingModeShaderWithPoseVariablesInTree) {
    const SkeletonContentPose* pose1 = nullptr;
    const SkeletonContentPose* pose2 = nullptr;

    GetActionFramePoses(&pose1, &pose2, &knownPoseBlendWeight, &knownActionKeyFrame);

    PerformBoneDepthSort(pose1, pose2, knownPoseBlendWeight);

    if(!knownActionPose1 || knownActionPose1 != pose1) {
      knownActionPose1 = pose1;
      currActionPose1.Copy(*pose1);
    }

    if(!knownActionPose2 || knownActionPose2 != pose2) {
      knownActionPose2 = pose2;
      currActionPose2.Copy(*pose2);
    }

    currActionPoseI.Interpolate(currActionPose1, currActionPose2, knownPoseBlendWeight);

    if(lastActionPoseBlendCtr > 0.0f && lastActionPoseBlendTime > 0.0f) {
      f32 t = lastActionPoseBlendCtr / lastActionPoseBlendTime;
      lastActionPoseTemp.Copy(currActionPoseI);
      currActionPoseI.Interpolate(lastActionPoseTemp, lastActionPose, t, nullptr, &boneCancelActionBlend);
    }
  }
  else {
    const SkeletonContentPose* pose1 = nullptr;
    const SkeletonContentPose* pose2 = nullptr;

    GetActionFramePoses(&pose1, &pose2, &knownPoseBlendWeight, &knownActionKeyFrame);

    PerformBoneDepthSort(pose1, pose2, knownPoseBlendWeight);

    if(pose1) {
      if(!knownActionPose1 || knownActionPose1 != pose1) {
        knownActionPose1 = pose1;
        currActionPose1.Copy(*pose1);
      }
    }

    if(pose2) {
      if(!knownActionPose2 || knownActionPose2 != pose2) {
        knownActionPose2 = pose2;
        currActionPose2.Copy(*pose2);
      }
    }

    if(pose1 && pose2) {
      currActionPoseI.Interpolate(currActionPose1, currActionPose2, knownPoseBlendWeight);

      if(lastActionPoseBlendCtr > 0.0f && lastActionPoseBlendTime > 0.0f) {
        f32 t = lastActionPoseBlendCtr / lastActionPoseBlendTime;
        lastActionPoseTemp.Copy(currActionPoseI);
        currActionPoseI.Interpolate(lastActionPoseTemp, lastActionPose, t, nullptr, &boneCancelActionBlend);
      }
    }
  }

  CacheBoneTransforms(true);
}

bool Skeleton::GetBoneTransform(const std::string& name, Mat44& mat) {
  if(!HasContent())
    return false;

  if(content->GetActionCount() == 0)
    return false;

  const SkeletonContentAction& action = content->GetAction(actionIndex);

  size_t boneIndex = content->GetBoneIndex(name);
  if(boneIndex == PrimeNotFound)
    return false;

  if(boneIndex >= totalBoneCount)
    return false;

  const SkeletonPoseBone* poseBone = currActionPoseI.GetBone(boneIndex);
  if(!poseBone)
    return false;

  if(poseBone->alpha > 0.0f) {
    mat.LoadIdentity();
    mat.Translate(poseBone->x - action.x, poseBone->y - action.y);
    mat.Rotate(poseBone->angle);
    mat.Scale(poseBone->scaleX, poseBone->scaleY);
  }

  return true;
}

void Skeleton::CacheBoneTransforms(bool force) {
  if(!HasContent())
    return;

  if(content->GetActionCount() == 0)
    return;

  size_t currentFrame = PxEngine.GetCurrentFrame();
  if(!force && cacheBoneTransformsFrame == currentFrame)
    return;

  cacheBoneTransformsFrame = currentFrame;

  // Cache bone transforms.
  size_t boneCount = content->GetBoneCount();
  const SkeletonContentAction& action = content->GetAction(actionIndex);

  for(size_t i = 0; i < boneCount; i++) {
    const SkeletonPoseBone* poseBone = currActionPoseI.GetBone(i);
    if(!poseBone)
      continue;

    refptr<Skinset> skinset = GetSkinsetForBone(i);
    if(skinset) {
      refptr<SkinsetContent> skinsetContent = skinset->GetSkinsetContent();
      if(skinsetContent) {
        const SkinsetContentAffixPieceLookupStack& affixes = GetBoneSkinsetAffixes(skinset, i);
        size_t affixesCount = affixes.GetCount();
        auto pieces = skinset->GetPieces();

        for(size_t j = 0; j < affixesCount; j++) {
          size_t index = affixes.GetItem(j);
          const SkinsetContentPiece& contentPiece = skinsetContent->GetPiece(index);
          if(poseBone->alpha > 0.0f) {
            Mat44 mat;
            mat.LoadTranslation(poseBone->x - action.x, poseBone->y - action.y);
            mat.Rotate(poseBone->angle);
            mat.Scale(poseBone->scaleX, poseBone->scaleY);
            mat.Multiply(contentPiece.baseTransform);

            auto piece = (*pieces)[index];
            if(piece && piece->skeleton) {
              piece->skeleton->CacheBoneTransforms(force);
              piece->skeleton->SetLocalMat(mat);
            }
          }
        }
      }
    }
  }
}

f32 Skeleton::GetUniformSize() const {
  const Vec3& vertexMin = GetVertexMin();
  const Vec3& vertexMax = GetVertexMax();

  f32 sizeX = vertexMax.x - vertexMin.x;
  f32 sizeY = vertexMax.y - vertexMin.y;
  f32 size = max(sizeX, sizeY);

  return size;
}

void Skeleton::SetAction(const std::string& name) {
  if(!HasContent())
    return;

  if(content->GetActionCount() == 0) {
    DiscardAction();
    return;
  }

  size_t actionCount = content->GetActionCount();
  if(!name.empty()) {
    for(size_t i = 0; i < actionCount; i++) {
      const SkeletonContentAction& action = content->GetAction(i);
      if(action.name == name) {
        SetActionByIndex(i);
        break;
      }
    }
  }
  else {
    SetActionByIndex(0);
  }
}

bool Skeleton::SetActionIfNew(const std::string& name) {
  if(!HasContent())
    return false;

  if(content->GetActionCount() == 0) {
    DiscardAction();
    return false;
  }

  size_t actionCount = content->GetActionCount();
  if(!name.empty()) {
    for(size_t i = 0; i < actionCount; i++) {
      const SkeletonContentAction& action = content->GetAction(i);
      if(action.name == name && actionIndex != i) {
        SetActionByIndex(i);
        return true;
      }
    }
  }

  return false;
}

void Skeleton::SetActionTime(f32 time) {
  if(actionLen > 0.0f) {
    if(time > actionCtr) {
      f32 dt = time - actionCtr;
      Calc(dt);
    }
    else if(time < actionCtr) {
      SetActionByIndex(actionIndex);
      Calc(time);
    }
  }
}

void Skeleton::SetActionT(f32 t) {
  SetActionTime(GetActionLen() * t);
}

void Skeleton::SetActionTimeScale(f32 scale) {
  actionTimeScale = scale;
  if(actionTimeScale < 0.0f)
    actionTimeScale = 0.0f;
}

void Skeleton::SetActionReverse(bool reverse) {
  actionReverse = reverse;
}

bool Skeleton::DoesActionExist(const std::string& name) {
  if(!HasContent())
    return false;

  size_t actionCount = content->GetActionCount();
  if(!name.empty()) {
    for(size_t i = 0; i < actionCount; i++) {
      const SkeletonContentAction& action = content->GetAction(i);
      if(action.name == name) {
        return true;
      }
    }
  }

  return false;
}

bool Skeleton::IsInAction(const std::string& name) {
  if(!HasContent())
    return false;

  if(actionIndex != PrimeNotFound) {
    if(content->GetActionCount()) {
      const SkeletonContentAction& action = content->GetAction(actionIndex);
      return name == action.name;
    }
  }

  return false;
}

size_t Skeleton::GetActionIndex() const {
  return actionIndex;
}

const std::string& Skeleton::GetActionName() const {
  if(HasContent()) {
    if(actionIndex != PrimeNotFound) {
      if(content->GetActionCount()) {
        const SkeletonContentAction& action = content->GetAction(actionIndex);
        return action.name;
      }
    }
  }

  static const std::string noActionName;
  return noActionName;
}

f32 Skeleton::GetActionLen() const {
  return actionLen;
}

f32 Skeleton::GetActionTime() const {
  return actionCtr;
}

f32 Skeleton::GetActionT() const {
  return actionLen ? actionCtr / actionLen : 0.0f;
}

void Skeleton::SetActionByIndex(size_t index) {
  knownActionPose1 = nullptr;
  knownActionPose2 = nullptr;
  knownPoseBlendWeight = 0.0f;

  if(!HasContent())
    return;

  size_t actionCount = content->GetActionCount();
  if(actionCount == 0) {
    DiscardAction();
    return;
  }

  PrimeAssert(index < actionCount, "Invalid action id: %d >= %d", index, actionCount);
  size_t oldActionIndex = actionIndex;
  bool oldActionPoseBlendAllowed = true;

  if(oldActionIndex != -1) {
    const SkeletonContentAction& oldAction = content->GetAction(oldActionIndex);
    oldActionPoseBlendAllowed = oldAction.nextPoseBlendAllowed;
  }

  lastActionPose.Copy(currActionPoseI);

  actionIndex = index;
  actionChanged = true;
  actionCtr = 0.0f;
  actionLen = 0.0f;
  s32 oldLoopCount = actionLoopCount;
  actionLoopCount = 0;
  actionPlayed = false;

  const SkeletonContentAction& action = content->GetAction(actionIndex);

  if(oldActionPoseBlendAllowed && !nextActionPoseBlendCanceled) {
    lastActionPoseBlendTime = action.lastPoseBlendTimeSpecified ? action.lastPoseBlendTime : PRIME_SKELETON_DEFAULT_LAST_POSE_BLEND_TIME;
  }
  else {
    lastActionPoseBlendTime = 0.0f;
  }
  lastActionPoseBlendCtr = lastActionPoseBlendTime;
  nextActionPoseBlendCanceled = false;

  size_t actionLenInFrames = 0;
  for(size_t i = 0; i < action.keyFrameCount; i++) {
    SkeletonContentActionKeyFrame& keyFrame = action.keyFrames[i];
    actionLenInFrames += keyFrame.len;
  }
  actionLen = actionLenInFrames / content->GetFPS();

  const SkeletonContentPose* pose1 = nullptr;
  const SkeletonContentPose* pose2 = nullptr;
  f32 weight;
  const SkeletonContentActionKeyFrame* actionKeyFrame = nullptr;

  GetActionFramePoses(&pose1, &pose2, &weight, &actionKeyFrame);

  if(pose1 && lastActionPoseBlendTime == 0.0f) {
    currActionPoseI.Copy(*pose1);
  }

  currActionPose1.Copy(currActionPoseI);
  currActionPose2.Copy(currActionPoseI);

  ProcessBoneCancelActionBlend(actionKeyFrame);

  if(!HasSkinset())
    return;

  skinset->SetAction(action.name, actionKeyFrame, false);

  if(HasAdditionalSkinsets()) {
    for(auto it: *additionalSkinsets) {
      refptr<Skinset> additionalSkinset = it.value();
      additionalSkinset->SetAction(action.name, actionKeyFrame, false);
    }
  }

  PerformBoneDepthSort(pose1, pose2, weight);
  boneDepthUpdated = true;

  if(processingMode == SkeletonProcessingModeShaderWithPoseVariables) {
    bufferTexLookup.Clear();
    pieceSignaturesOutdated = true;
    UpdateBufferPieces();
    UpdateBufferPose();
  }
}

void Skeleton::ResetActionChanged() {
  actionChanged = false;
}

bool Skeleton::HasActionChanged() {
  bool result = actionChanged;
  actionChanged = false;
  return result;
}

void Skeleton::CancelLastActionBlend() {
  lastActionPoseBlendCtr = 0.0f;
  CalcPose(0.0f);
}

void Skeleton::ProcessBoneCancelActionBlend(const SkeletonContentActionKeyFrame* actionKeyFrame) {
  boneCancelActionBlend.Clear();

  if(!HasContent())
    return;

  if(content->GetActionCount() == 0)
    return;

  const SkeletonContentAction& action = content->GetAction(actionIndex);

  size_t boneCount = content->GetBoneCount();
  for(size_t i = 0; i < boneCount; i++) {
    const SkeletonContentBone* bones = content->GetBones();
    const SkeletonContentBone& bone = bones[i];
    refptr<Skinset> skinset = GetSkinsetForBone(i);
    if(skinset) {
      refptr<SkinsetContent> skinsetContent = skinset->GetSkinsetContent();
      const SkinsetContentAffixPieceLookupStack& affixes = GetBoneSkinsetAffixes(skinset, i);
      size_t affixesCount = affixes.GetCount();
      for(size_t j = 0; j < affixesCount; j++) {
        size_t index = affixes.GetItem(j);
        const std::string& useAction1 = skinsetContent->GetMappedAction(index, action.name, knownActionKeyFrame);
        const std::string& useAction2 = skinsetContent->GetMappedAction(index, action.name, actionKeyFrame);
        if(useAction1 != useAction2) {
          for(size_t k = 0; k < boneCount; k++) {
            if(bones[k].cancelActionBlend && content->IsBoneDescendant(k, i)) {
              boneCancelActionBlend.Add(bones[k].name);
            }
          }
        }
      }
    }
  }
}

void Skeleton::PerformBoneDepthSort() {
  const SkeletonContentPose* pose1 = nullptr;
  const SkeletonContentPose* pose2 = nullptr;

  GetActionFramePoses(&pose1, &pose2, &knownPoseBlendWeight, &knownActionKeyFrame);
    
  PerformBoneDepthSort(pose1, pose2, knownPoseBlendWeight);
}

void Skeleton::SetBoneOverrideTranslation(const char* bone, f32 x, f32 y) {
  SkeletonBoneOverride* boneOverride = GetBoneOverride(bone, true);
  if(!boneOverride)
    return;

  boneOverride->x = x;
  boneOverride->y = y;
  boneOverride->overrideTranslation = true;
}

void Skeleton::SetBoneOverrideAngle(const char* bone, f32 angle, bool absolute) {
  SkeletonBoneOverride* boneOverride = GetBoneOverride(bone, true);
  if(!boneOverride)
    return;

  boneOverride->angle = angle;
  boneOverride->overrideAngle = true;
  boneOverride->overrideAngleAbsolute = absolute;
}

void Skeleton::SetBoneOverrideScale(const char* bone, f32 scaleX, f32 scaleY) {
  SkeletonBoneOverride* boneOverride = GetBoneOverride(bone, true);
  if(!boneOverride)
    return;

  boneOverride->scaleX = scaleX;
  boneOverride->scaleY = scaleY;
  boneOverride->overrideScale = true;
}

void Skeleton::ClearBoneOverrideTranslation(const char* bone) {
  SkeletonBoneOverride* boneOverride = GetBoneOverride(bone);
  if(!boneOverride)
    return;

  boneOverride->overrideTranslation = false;
}

void Skeleton::ClearBoneOverrideAngle(const char* bone) {
  SkeletonBoneOverride* boneOverride = GetBoneOverride(bone);
  if(!boneOverride)
    return;

  boneOverride->overrideAngle = false;
}

void Skeleton::ClearBoneOverrideScale(const char* bone) {
  SkeletonBoneOverride* boneOverride = GetBoneOverride(bone);
  if(!boneOverride)
    return;

  boneOverride->overrideScale = false;
}

void Skeleton::Draw() {
  if(!HasContent())
    return;

  if(!HasSkinset())
    return;

  if(!skinset->HasContent())
    return;

  size_t boneCount = content->GetBoneCount();
  if(!boneCount)
    return;

  if(!shaderDataReady)
    return;

  Graphics& g = PxGraphics;
  if(!g.program)
    return;

  if(processingMode == SkeletonProcessingModeShaderWithPoseVariables) {
    size_t texCount = bufferTexLookup.GetCount();
    if(texCount > 0 && programDataBoneCount > 0) {
      size_t bufferTexCount = bufferTexLookup.GetCount();

      Tex* texList[PRIME_SKELETON_PROGRAM_TEX_UNIT_COUNT];
      memset(texList, 0, sizeof(texList));

      for(size_t i = 0; i < bufferTexCount; i++) {
        auto imc = bufferTexList[i];
        texList[i] = imc->GetTex();
      }

      UpdateProgramBoneData(g.program);

      g.Draw(ab, ib, texList, bufferTexCount);
    }
  }
  else if(processingMode == SkeletonProcessingModeShaderWithPoseVariablesInTree) {
    // Drawing handled by skeletal shader tree.
  }
}

void Skeleton::SetLocalMat(const Mat44& mat) {
  localMat = mat;
}

const Mat44& Skeleton::GetLocalMat() const {
  return localMat;
}

void Skeleton::SetProcessingMode(SkeletonProcessingMode mode) {
  DestroyPieceSignatures();
  thisBoneCount = 0;
  thisPieceCount = 0;
  totalBoneCount = 0;
  totalPieceCount = 0;
  totalTexCount = 0;
  bufferTexLookup.Clear();
  ib = nullptr;
  ab = nullptr;
  processingMode = SkeletonProcessingModeNone;

  if(!HasContent())
    return;

  if(!HasSkinset())
    return;

  if(!skinset->HasContent())
    return;

  processingMode = mode;

  if(mode == SkeletonProcessingModeShaderWithPoseVariables) {
    thisBoneCount = content->GetBoneCount();
    thisPieceCount = skinset->GetPieceCount();
    totalBoneCount = GetTreeBoneCount();
    totalPieceCount = GetSkinsetTreePieceCount();

    if(totalBoneCount > 0 && totalPieceCount > 0 && totalBoneCount <= PRIME_SKELETON_PROGRAM_BONE_COUNT) {
      size_t vertexCount = totalPieceCount * 4;
      size_t indexCount = totalPieceCount * 6;

      ab = ArrayBuffer::Create(sizeof(SkeletonVertex), nullptr, vertexCount, BufferPrimitiveTriangles);
      ab->LoadAttribute("vPos", sizeof(f32) * 2);
      ab->LoadAttribute("vUVBoneTexture", sizeof(f32) * 4);

      IndexFormat indexFormat;
      if(vertexCount < 0x100)
        indexFormat = IndexFormatSize8;
      else if(vertexCount < 0x10000)
        indexFormat = IndexFormatSize16;
      else
        indexFormat = IndexFormatSize32;

      ib = IndexBuffer::Create(indexFormat, nullptr, indexCount);

      CreatePieceSignatures();

      SetProcessingModeInTree(SkeletonProcessingModeShaderWithPoseVariablesInTree);

      UpdateBufferPieces();
      UpdateBufferPose();
    }
    else {
      processingMode = SkeletonProcessingModeInactive;
    }
  }
  else {
    SetProcessingModeInTree(mode);
  }
}

bool Skeleton::IsProcessingModeUsingShader() const {
  return processingMode == SkeletonProcessingModeShaderWithPoseVariables;
}

size_t Skeleton::GetShaderTexCount() const {
  if(processingMode == SkeletonProcessingModeShaderWithPoseVariables)
    return bufferTexLookup.GetCount();
  else
    return 0;
}

size_t Skeleton::GetTreeBoneCount() const {
  if(HasContent()) {
    size_t result = content->GetBoneCount();

    if(HasSkinset())
      result += skinset->GetTreeBoneCount();

    return result;
  }

  return 0;
}

size_t Skeleton::GetSkinsetTreePieceCount() const {
  if(HasSkinset())
    return skinset->GetTreePieceCount();
  else
    return 0;
}

SkeletonPose& Skeleton::GetCurrentPose() {
  return currActionPoseI;
}

void Skeleton::DiscardAction() {
  actionIndex = PrimeNotFound;
  actionChanged = false;
  actionCtr = 0.0f;
  actionLen = 0.0f;
  actionLoopCount = 0;
  actionPlayed = false;

  currActionPose1.SetContent(content);
  currActionPose2.SetContent(content);
  currActionPoseI.SetContent(content);
  lastActionPose.SetContent(content);
  lastActionPoseTemp.SetContent(content);

  knownActionPose1 = nullptr;
  knownActionPose2 = nullptr;
  knownPoseBlendWeight = 0.0f;
}

void Skeleton::InitBonePoseData(SkeletonBonePoseData& bpd) {
  bpd.x = 0.0f;
  bpd.y = 0.0f;
  bpd.x2 = 0.0f;
  bpd.y2 = 0.0f;
  bpd.dx = 0.0f;
  bpd.dy = 0.0f;
  bpd.angle = 0.0f;
  bpd.angleParent = 0.0f;
  bpd.scaleX = 1.0f;
  bpd.scaleY = 1.0f;
}

void Skeleton::GetActionFramePoses(const SkeletonContentPose** pose1, const SkeletonContentPose** pose2, f32* weight, const SkeletonContentActionKeyFrame** actionKeyFrame) {
  if(!HasContent())
    return;

  if(!pose1)
    return;

  SkeletonContentActionKeyFrame* keyFrame1;
  SkeletonContentActionKeyFrame* keyFrame2;
  size_t keyFrame1TimeInFrames = 0;
  size_t keyFrame2TimeInFrames = 0;
  f32 keyFrame1Time;
  f32 keyFrame2Time;

  if(content->GetActionCount() == 0)
    return;

  const SkeletonContentAction& action = content->GetAction(actionIndex);

  PrimeAssert(action.keyFrameCount, "Action has no key frames.");

  SkeletonContentActionKeyFrame* keyFrames = action.keyFrames;
  f32 fps = content->GetFPS();

  f32 useActionCtr;

  if(actionReverse) {
    f32 actionT = actionCtr / (f32) actionLen;
    useActionCtr = (1.0f - actionT) * actionLen;
  }
  else {
    useActionCtr = actionCtr;
  }

  if(useActionCtr >= actionLen && !action.loop) {
    keyFrame1 = &action.keyFrames[action.keyFrameCount - 1];

    *pose1 = &content->GetPose(keyFrame1->poseIndex);

    if(pose2)
      *pose2 = *pose1;

    if(weight)
      *weight = 1.0f;

    if(actionKeyFrame)
      *actionKeyFrame = keyFrame1;

    return;
  }

  keyFrame1 = &action.keyFrames[0];
  keyFrame2 = keyFrame1;
  keyFrame1Time = 0.0f;
  keyFrame2Time = 0.0f;

  for(size_t i = 0; i < action.keyFrameCount; i++) {
    SkeletonContentActionKeyFrame& keyFrame = keyFrames[i];

    if(keyFrame.len == 0)
      continue;

    size_t nextKeyFrameTimeInFrames = keyFrame1TimeInFrames + keyFrame.len;
    f32 nextKeyFrameTime = nextKeyFrameTimeInFrames / fps;

    if(actionReverse) {
      if(useActionCtr < nextKeyFrameTime) {
        keyFrame1 = &keyFrame;
        if(i == 0)
          keyFrame2 = &action.keyFrames[action.keyFrameCount - 1];
        else
          keyFrame2 = &action.keyFrames[i - 1];
        keyFrame2TimeInFrames = nextKeyFrameTimeInFrames;
        break;
      }
    }
    else {
      if(useActionCtr < nextKeyFrameTime) {
        keyFrame1 = &keyFrame;
        if(i == action.keyFrameCount - 1) {
          if(!action.loop) {
            keyFrame2 = &action.keyFrames[i];
          }
          else {
            keyFrame2 = &action.keyFrames[0];
          }
        }
        else { 
          keyFrame2 = &action.keyFrames[i + 1];
        }
        keyFrame2TimeInFrames = nextKeyFrameTimeInFrames;
        break;
      }
    }

    keyFrame1TimeInFrames = nextKeyFrameTimeInFrames;
  }

  keyFrame1Time = keyFrame1TimeInFrames / fps;
  keyFrame2Time = keyFrame2TimeInFrames / fps;

  if(keyFrame1 == keyFrame2 || keyFrame1TimeInFrames == keyFrame2TimeInFrames) {
    *pose1 = &content->GetPose(keyFrame1->poseIndex);

    if(pose2)
      *pose2 = *pose1;

    if(weight)
      *weight = 0.0f;
  }
  else {
    *pose1 = &content->GetPose(keyFrame1->poseIndex);

    if(pose2)
      *pose2 = &content->GetPose(keyFrame2->poseIndex);

    if(weight) {
      if(actionReverse) {
        *weight = (useActionCtr - keyFrame2Time) / (f32) (keyFrame1Time - keyFrame2Time);
      }
      else {
        *weight = (useActionCtr - keyFrame1Time) / (f32) (keyFrame2Time - keyFrame1Time);
      }
    }
  }

  if(actionKeyFrame)
    *actionKeyFrame = keyFrame1;
}

void Skeleton::PerformBoneDepthSort(const SkeletonContentPose* pose1, const SkeletonContentPose* pose2, f32 weight) {
  if(!HasContent())
    return;

  if(!pose1)
    return;

  if(!pose2) {
    pose2 = pose1;
    weight = 0.0f;
  }

  const SkeletonContentBone* bones = content->GetBones();

  size_t count = depthSortedItems.GetCount();
  for(size_t i = 0; i < count; i++) {
    SkeletonDepthSortItem& item = depthSortedItems[i];
    const SkeletonContentBone* bone = &bones[item.boneIndex];
    item.depth = bone->depth + pose1->bones[item.boneIndex].depth;
    lastDepthSortedBoneIndices[i] = item.boneIndex;
  }

  depthSortedItems.Sort();

  if(!boneDepthUpdated) {
    for(size_t i = 0; i < count; i++) {
      if(lastDepthSortedBoneIndices[i] != depthSortedItems[i].boneIndex) {
        boneDepthUpdated = true;
        break;
      }
    }
  }
}

SkeletonBoneOverride* Skeleton::GetBoneOverride(const char* bone, bool create) {
  if(!bone || !bone[0] || !HasContent())
    return nullptr;

  if(!create && !boneOverrides)
    return nullptr;

  size_t boneCount = content->GetBoneCount();

  if(create && !boneOverrides) {
    boneOverrides = new SkeletonBoneOverride[boneCount];
    for(size_t i = 0; i < boneCount; i++) {
      memset(&boneOverrides[i], 0, sizeof(SkeletonBoneOverride));
    }

    currActionPose1.SetBoneOverrides(boneOverrides);
    currActionPose2.SetBoneOverrides(boneOverrides);
    currActionPoseI.SetBoneOverrides(boneOverrides);
    lastActionPose.SetBoneOverrides(boneOverrides);
    lastActionPoseTemp.SetBoneOverrides(boneOverrides);
  }

  const SkeletonContentBone* bones = content->GetBones();

  for(size_t i = 0; i < boneCount; i++) {
    if(bones[i].name == bone) {
      return &boneOverrides[i];
    }
  }

  return nullptr;
}

void Skeleton::DestroyAllBoneSkinsetAffixes() {
  for(auto it: boneSkinsetAffixes) {
    SkinsetContentAffixPieceLookupStack** lookupStack = it.value();
    if(lookupStack) {
      for(size_t i = 0; i < boneSkinsetAffixesBoneCount; i++) {
        delete lookupStack[i];
      }

      delete[] lookupStack;
    }
  }

  boneSkinsetAffixesBoneCount = 0;
}

void Skeleton::DestroyBoneSkinsetAffixes(refptr<Skinset> skinset) {
  if(auto it = boneSkinsetAffixes.Find(skinset)) {
    SkinsetContentAffixPieceLookupStack** lookupStack = it.value();
    if(lookupStack) {
      for(size_t i = 0; i < boneSkinsetAffixesBoneCount; i++) {
        delete lookupStack[i];
      }

      delete[] lookupStack;
    }

    boneSkinsetAffixes.Remove(skinset);
  }
}

void Skeleton::CreateBoneSkinsetAffixes(refptr<Skinset> skinset) {
  if(!skinset)
    return;

  if(!skinset->HasContent())
    return;

  if(!HasContent())
    return;

  DestroyBoneSkinsetAffixes(skinset);

  refptr<SkinsetContent> skinsetContent = skinset->GetSkinsetContent();
  const SkeletonContentBone* bones = content->GetBones();
  size_t boneCount = content->GetBoneCount();

  boneSkinsetAffixesBoneCount = boneCount;
  boneSkinsetAffixes[skinset] = new SkinsetContentAffixPieceLookupStack*[boneSkinsetAffixesBoneCount];

  for(size_t i = 0; i < boneSkinsetAffixesBoneCount; i++) {
    const SkeletonContentBone& bone = bones[i];
    boneSkinsetAffixes[skinset][i] = skinsetContent->CreateAffixPieceLookupStack(bone.name);
  }
}

const SkinsetContentAffixPieceLookupStack& Skeleton::GetBoneSkinsetAffixes(refptr<Skinset> skinset, size_t boneIndex) {
  if(auto it = boneSkinsetAffixes.Find(skinset)) {
    SkinsetContentAffixPieceLookupStack** lookupStack = it.value();
    PrimeAssert(lookupStack, "Affixes not yet created.");
    PrimeAssert(HasContent() && boneIndex < content->GetBoneCount(), "Invalid bone index.");
    return *lookupStack[boneIndex];
  }
  else {
    PrimeAssert(false, "Affixes not yet created for skinset.");
    static const SkinsetContentAffixPieceLookupStack noLookupStack;
    return noLookupStack;
  }
}

void Skeleton::UpdateReferencedBoneSkinsetAffixes() {
  Dictionary<refptr<Skinset>, bool> activeSkinsets;

  if(HasSkinset()) {
    activeSkinsets[skinset] = true;
  }

  if(additionalSkinsets) {
    for(auto it: *additionalSkinsets) {
      refptr<Skinset> additionalSkinset = it.value();
      activeSkinsets[additionalSkinset] = true;
    }
  }

  Stack<refptr<Skinset>> boneSkinsetAffixesToDestroyBySkinset;

  for(auto it: boneSkinsetAffixes) {
    refptr<Skinset> currSkinset = it.key();
    if(!activeSkinsets.HasKey(currSkinset)) {
      boneSkinsetAffixesToDestroyBySkinset.Add(currSkinset);
    }
  }

  for(auto currSkinset: boneSkinsetAffixesToDestroyBySkinset) {
    DestroyBoneSkinsetAffixes(currSkinset);
  }
}

void Skeleton::DestroyPieceSignatures() {
  if(bonePieceSignatureIndices) {
    for(size_t i = 0; i < bonePieceSignatureIndicesCount; i++) {
      PrimeSafeDelete(bonePieceSignatureIndices[i]);
    }
    PrimeSafeDeleteArray(bonePieceSignatureIndices);
    bonePieceSignatureIndicesCount = 0;
  }

  PrimeSafeDeleteArray(pieceSignatures);

  PrimeSafeDeleteArray(skeletonBoneRootTransforms);

  if(skeletonBoneLookup) {
    for(auto it: *skeletonBoneLookup) {
      size_t* lookup = it.value();
      if(lookup) {
        delete[] lookup;
      }
    }
    PrimeSafeDelete(skeletonBoneLookup);
  }
}

void Skeleton::CreatePieceSignatures() {
  DestroyPieceSignatures();

  if(!HasContent() || !HasSkinset())
    return;

  PrimeAssert(processingMode == SkeletonProcessingModeShaderWithPoseVariables, "Bone signatures is for shader processing modes only.");
  if(processingMode != SkeletonProcessingModeShaderWithPoseVariables)
    return;

  if(!skinset->HasContent())
    return;

  if(totalPieceCount == 0)
    return;

  skeletonBoneLookup = new Dictionary<Skeleton*, size_t*>();
  skeletonBoneRootTransforms = new Mat44[totalBoneCount];

  pieceSignatures = new SkeletonPieceSignature[totalPieceCount];
  pieceSignaturesOutdated = true;
  
  bonePieceSignatureIndicesCount = totalBoneCount;
  bonePieceSignatureIndices = new Stack<size_t>*[bonePieceSignatureIndicesCount];
  for(size_t i = 0; i < bonePieceSignatureIndicesCount; i++) {
    bonePieceSignatureIndices[i] = new Stack<size_t>();
  }

  boneDepthUpdated = true;
}

bool Skeleton::UpdatePieceSignature(SkeletonPieceSignature* signature, refptr<ImagemapContent> imc, size_t rectIndex, bool hflip, bool vflip) {
  bool result = signature->imc != imc;
  signature->imc = imc;

  if(!result && signature->rectIndex != rectIndex)
    result = true;
  signature->rectIndex = rectIndex;

  if(!result && signature->hflip != hflip)
    result = true;
  signature->hflip = hflip;

  if(!result && signature->vflip != vflip)
    result = true;
  signature->vflip = vflip;

  return result;
}

void Skeleton::UpdateBufferPieces() {
  PrimeAssert(processingMode == SkeletonProcessingModeShaderWithPoseVariables, "Invalid processing mode.");

  SkeletonAddBufferSkeletonParam param;
  memset(&param, 0, sizeof(param));
  param.ab = ab;
  param.boneIndex = 0;
  param.updatePieceCount = 0;
  param.texLookup = &bufferTexLookup;
  param.texList = bufferTexList;
  param.mainSkeleton = this;
  param.skeletonBoneLookup = skeletonBoneLookup;
  param.pieceSignatures = pieceSignatures;
  param.bonePieceSignatureIndices = bonePieceSignatureIndices;
  param.pieceSignaturesOutdated = pieceSignaturesOutdated;

  AddBufferSkeleton(param);

  if(boneDepthUpdated) {
    SkeletonAddBufferSkeletonDepthBonesParam param2;
    memset(&param2, 0, sizeof(param2));
    param2.ib = ib;
    param2.skeletonBoneLookup = skeletonBoneLookup;
    param2.pieceSignatures = pieceSignatures;
    param2.bonePieceSignatureIndices = bonePieceSignatureIndices;
    param2.pieceIndex = 0;

    AddBufferSkeletonDepthBones(param2);

    boneDepthUpdated = false;
  }

  ab->SetSyncCount(param.updatePieceCount * 4);
  ib->SetSyncCount(param.updatePieceCount * 6);

  pieceSignaturesOutdated = false;
}

void Skeleton::UpdateBufferPose() {
  if(!HasContent())
    return;

  if(!HasSkinset())
    return;

  PrimeAssert(processingMode == SkeletonProcessingModeShaderWithPoseVariables, "Invalid processing mode.");
  PrimeAssert(HasContent(), "Skeleton content not found.");
  PrimeAssert(HasSkinset(), "Skinset not found.");

  SkeletonBonePoseData bpd[PRIME_SKELETON_PROGRAM_BONE_COUNT];

  for(size_t i = 0; i < totalBoneCount; i++) {
    skeletonBoneRootTransforms[i].LoadIdentity();
  }

  SkeletonGetBufferBoneTransformsParam param;
  memset(&param, 0, sizeof(param));
  param.mainSkeleton = this;
  param.boneLookup = skeletonBoneLookup;
  param.boneRootTransforms = skeletonBoneRootTransforms;
  param.bpd = bpd;
  Mat44 rootTransform;
  rootTransform.LoadIdentity();
  GetBufferBoneTransforms(param, rootTransform, 1.0f);

  size_t dataP = 0;

  for(size_t i = 0; i < param.bpdCount; i++) {
    SkeletonBonePoseData& pd = bpd[i];
    programData1[dataP    ] = pd.x;
    programData1[dataP + 1] = pd.y;
    programData1[dataP + 2] = pd.alpha;
    programData2[dataP    ] = pd.scaleX;
    programData2[dataP + 1] = pd.scaleY;
    programData2[dataP + 2] = pd.angle * PrimeDegToRadF;
    dataP += 3;
  }

  PrimeAssert((param.bpdCount <= totalBoneCount) && (dataP / 3 <= totalBoneCount) && (dataP % 3 == 0), "Skeleton bone count mismatch.");

  programDataBoneCount = param.bpdCount;

  shaderDataReady = true;

  if(updateVertexSpan) {
    updateVertexSpan = false;

    vertexMin = Vec3(0.0f, 0.0f, 0.0f);
    vertexMax = Vec3(0.0f, 0.0f, 0.0f);

    size_t boneCount = content->GetBoneCount();
    const SkeletonContentAction& action = content->GetAction(actionIndex);

    for(size_t i = 0; i < boneCount; i++) {
      refptr<Skinset> skinset = GetSkinsetForBone(i);
      if(skinset) {
        refptr<SkinsetContent> skinsetContent = skinset->GetSkinsetContent();

        if(auto itBoneLookup = skeletonBoneLookup->Find(this)) {
          size_t* skeletonBones = itBoneLookup.value();
          if(skeletonBones) {
            size_t useBoneIndex = skeletonBones[i];

            const SkeletonBonePoseData& pd = bpd[useBoneIndex];
            size_t dataP = i * 3;
            f32 x = programData1[dataP];
            f32 y = programData1[dataP + 1];
            f32 scaleX = programData2[dataP];
            f32 scaleY = programData2[dataP + 1];
            f32 angle = programData2[dataP + 2];

            Mat44 mat = skeletonBoneRootTransforms[i];
            mat.Translate(x, y);
            mat.Rotate(angle);
            mat.Scale(scaleX, scaleY);

            const SkinsetContentAffixPieceLookupStack& affixes = GetBoneSkinsetAffixes(skinset, i);
            size_t affixesCount = affixes.GetCount();
            auto pieces = skinset->GetPieces();

            for(size_t j = 0; j < affixesCount; j++) {
              size_t index = affixes.GetItem(j);
              auto piece = (*pieces)[index];
              const SkinsetContentPiece& contentPiece = skinsetContent->GetPiece(index);
              if(piece && (piece->imagemap || piece->skeleton)) {
                if(piece->imagemap) {
                  refptr<ImagemapContent> imc = piece->imagemap->GetImagemapContent();
                  size_t rectIndex = piece->imagemap->GetRectIndex();

                  Tex* tex = imc->GetTex();
                  if(tex) {
                    const TexData* texData = tex->GetTexData("");
                    if(texData) {
                      static const std::string originStr("origin");

                      const ImagemapContentRect* rect = imc->GetRectByIndex(rectIndex);
                      const ImagemapContentTexRect* texRect = imc->GetTexRectByIndex(rectIndex);

                      const ImagemapContentRectPoint* origin = imc->GetRectPointByRectIndex(rectIndex, originStr);
                      f32 originX, originY;
                      if(origin) {
                        originX = origin->x;
                        originY = origin->y;
                      }
                      else {
                        originX = 0.0f;
                        originY = 0.0f;
                      }

                      f32 rx1 = (f32) rect->sx - originX;
                      f32 ry1 = originY - (f32) rect->dh - (f32) rect->sy;
                      f32 rx2 = rx1 + rect->dw;
                      f32 ry2 = ry1 + rect->dh;
                      f32 u1 = tex->GetU("", (f32) texRect->x);
                      f32 v1 = tex->GetV("", (f32) texRect->y);
                      f32 u2 = tex->GetU("", (f32) (texRect->x + texRect->w));
                      f32 v2 = tex->GetV("", (f32) (texRect->y + texRect->h));
                      Vec2 v;
                      Vec2 p;

                      if(vertexMin.IsZero() && vertexMax.IsZero()) {
                        vertexMin = Vec3(std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max(), 0.0f);
                        vertexMax = Vec3(std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest(), 0.0f);
                      }

                      contentPiece.baseTransform.Multiply(rx1, ry1, v.x, v.y);
                      p = mat * v;
                      vertexMin.x = min(vertexMin.x, p.x);
                      vertexMin.y = min(vertexMin.y, p.y);
                      vertexMax.x = max(vertexMax.x, p.x);
                      vertexMax.y = max(vertexMax.y, p.y);

                      contentPiece.baseTransform.Multiply(rx1, ry1, v.x, v.y);
                      p = mat * v;
                      vertexMin.x = min(vertexMin.x, p.x);
                      vertexMin.y = min(vertexMin.y, p.y);
                      vertexMax.x = max(vertexMax.x, p.x);
                      vertexMax.y = max(vertexMax.y, p.y);

                      contentPiece.baseTransform.Multiply(rx1, ry1, v.x, v.y);
                      p = mat * v;
                      vertexMin.x = min(vertexMin.x, p.x);
                      vertexMin.y = min(vertexMin.y, p.y);
                      vertexMax.x = max(vertexMax.x, p.x);
                      vertexMax.y = max(vertexMax.y, p.y);

                      contentPiece.baseTransform.Multiply(rx1, ry1, v.x, v.y);
                      p = mat * v;
                      vertexMin.x = min(vertexMin.x, p.x);
                      vertexMin.y = min(vertexMin.y, p.y);
                      vertexMax.x = max(vertexMax.x, p.x);
                      vertexMax.y = max(vertexMax.y, p.y);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

void Skeleton::GetBufferBoneTransforms(SkeletonGetBufferBoneTransformsParam& param, const Mat44& rootTransform, f32 alpha) {
  size_t boneCount = content->GetBoneCount();
  const SkeletonContentAction& action = content->GetAction(actionIndex);

  Mat44 thisRootTransform = rootTransform;
  thisRootTransform.Translate(-action.x, -action.y);

  size_t* lookup = (*param.boneLookup)[this];
  if(!lookup)
    return;

  for(size_t i = 0; i < boneCount; i++) {
    PrimeAssert(param.bpdCount < PRIME_SKELETON_PROGRAM_BONE_COUNT, "Processed too many skinset bones.");
    if(param.bpdCount >= PRIME_SKELETON_PROGRAM_BONE_COUNT)
      return;

    size_t boneIndex = lookup[i];
    param.boneRootTransforms[boneIndex] = thisRootTransform;

    const SkeletonPoseBone* poseBone = currActionPoseI.GetBone(i);
    SkeletonBonePoseData& pd = param.bpd[param.bpdCount++];
    pd.x = poseBone->x;
    pd.y = poseBone->y;
    pd.dx = poseBone->dx;
    pd.dy = poseBone->dy;
    pd.angle = poseBone->angle;
    pd.angleParent = poseBone->angleParent;
    pd.alpha = alpha * poseBone->alpha;
    if(pd.alpha == 0.0f) {
      pd.scaleX = 0.0f;
      pd.scaleY = 0.0f;
    }
    else {
      pd.scaleX = poseBone->scaleX;
      pd.scaleY = poseBone->scaleY;
    }

    refptr<Skinset> skinset = GetSkinsetForBone(i);
    if(skinset) {
      refptr<SkinsetContent> skinsetContent = skinset->GetSkinsetContent();
      const SkinsetContentAffixPieceLookupStack& affixes = GetBoneSkinsetAffixes(skinset, i);
      size_t affixesCount = affixes.GetCount();
      auto pieces = skinset->GetPieces();

      for(size_t j = 0; j < affixesCount; j++) {
        size_t index = affixes.GetItem(j);
        auto piece = (*pieces)[index];

        if(piece && piece->skeleton) {
          Mat44 brt = rootTransform;
          if(pd.alpha > 0.0f) {
            brt.Multiply(piece->skeleton->GetLocalMat());
          }

          piece->skeleton->GetBufferBoneTransforms(param, brt, pd.alpha);
        }
      }
    }
  }
}

void Skeleton::AddBufferSkeleton(SkeletonAddBufferSkeletonParam& param, size_t parentBoneIndex) {
  size_t boneCount = content->GetBoneCount();
  const SkeletonContentAction& action = content->GetAction(actionIndex);
  SkeletonVertex vertex;

  for(size_t i = 0; i < boneCount; i++) {
    refptr<Skinset> skinset = GetSkinsetForBone(i);
    if(skinset) {
      refptr<SkinsetContent> skinsetContent = skinset->GetSkinsetContent();

      const SkinsetContentAffixPieceLookupStack& affixes = GetBoneSkinsetAffixes(skinset, i);
      size_t affixesCount = affixes.GetCount();
      size_t useBoneIndex = param.boneIndex;
      param.boneIndex++;

      size_t*& skeletonBones = (*param.skeletonBoneLookup)[this];
      if(!skeletonBones) {
        skeletonBones = new size_t[boneCount];
      }

      skeletonBones[i] = useBoneIndex;

      param.bonePieceSignatureIndices[useBoneIndex]->Clear();

      auto pieces = skinset->GetPieces();

      for(size_t j = 0; j < affixesCount; j++) {
        size_t index = affixes.GetItem(j);
        auto piece = (*pieces)[index];
        const SkinsetContentPiece& contentPiece = skinsetContent->GetPiece(index);
        if(piece && (piece->imagemap || piece->skeleton)) {
          size_t pieceSignatureIndex = param.updatePieceCount;
          size_t indexStart = pieceSignatureIndex * 4;
          SkeletonPieceSignature* pieceSignature = &param.pieceSignatures[pieceSignatureIndex];

          if(knownActionKeyFrame)
            skinset->SetPieceAction(index, skinsetContent->GetMappedAction(index, action.name, knownActionKeyFrame), true, actionCtr);

          if(piece->imagemap) {
            param.bonePieceSignatureIndices[useBoneIndex]->Push(pieceSignatureIndex);

            refptr<ImagemapContent> imc = piece->imagemap->GetImagemapContent();
            size_t rectIndex = piece->imagemap->GetRectIndex();

            Tex* tex = imc->GetTex();
            if(tex) {
              const TexData* texData = tex->GetTexData("");
              if(texData) {
                static const std::string originStr("origin");

                const ImagemapContentRect* rect = imc->GetRectByIndex(rectIndex);
                const ImagemapContentTexRect* texRect = imc->GetTexRectByIndex(rectIndex);

                const ImagemapContentRectPoint* origin = imc->GetRectPointByRectIndex(rectIndex, originStr);
                f32 originX, originY;
                if(origin) {
                  originX = origin->x;
                  originY = origin->y;
                }
                else {
                  originX = 0.0f;
                  originY = 0.0f;
                }

                bool update = UpdatePieceSignature(pieceSignature, imc, rectIndex, originX, originY);
                update = update || param.pieceSignaturesOutdated;
                param.updatePieceCount++;

                if(update) {
                  f32 rx1 = (f32) rect->sx - originX;
                  f32 ry1 = originY - (f32) rect->dh - (f32) rect->sy;
                  f32 rx2 = rx1 + rect->dw;
                  f32 ry2 = ry1 + rect->dh;
                  f32 u1 = tex->GetU("", (f32) texRect->x);
                  f32 v1 = tex->GetV("", (f32) texRect->y);
                  f32 u2 = tex->GetU("", (f32) (texRect->x + texRect->w));
                  f32 v2 = tex->GetV("", (f32) (texRect->y + texRect->h));
                  f32 tx, ty;

                  size_t texIndex = LookupTexIndex(imc, param.texLookup, param.texList);

                  vertex.boneIndex = (f32) useBoneIndex;
                  vertex.texIndex = (f32) texIndex;

                  contentPiece.baseTransform.Multiply(rx1, ry1, tx, ty);
                  vertex.x = tx;
                  vertex.y = ty;
                  vertex.u = u1;
                  vertex.v = v2;
                  param.ab->SetItem(indexStart, &vertex);

                  contentPiece.baseTransform.Multiply(rx1, ry2, tx, ty);
                  vertex.x = tx;
                  vertex.y = ty;
                  vertex.u = u1;
                  vertex.v = v1;
                  param.ab->SetItem(indexStart + 1, &vertex);

                  contentPiece.baseTransform.Multiply(rx2, ry2, tx, ty);
                  vertex.x = tx;
                  vertex.y = ty;
                  vertex.u = u2;
                  vertex.v = v1;
                  param.ab->SetItem(indexStart + 2, &vertex);

                  contentPiece.baseTransform.Multiply(rx2, ry1, tx, ty);
                  vertex.x = tx;
                  vertex.y = ty;
                  vertex.u = u2;
                  vertex.v = v2;
                  param.ab->SetItem(indexStart + 3, &vertex);

                  pieceSignature->vertexIndexStart = indexStart;
                  boneDepthUpdated = true;
                  updateVertexSpan = true;
                }
              }
            }
          }
          else if(piece->skeleton) {
            piece->skeleton->AddBufferSkeleton(param, useBoneIndex);
          }
        }
      }
    }
  }
}

void Skeleton::AddBufferSkeletonDepthBones(SkeletonAddBufferSkeletonDepthBonesParam& param) {
  IndexFormat indexFormat = param.ib->GetFormat();

  size_t*& skeletonBones = (*param.skeletonBoneLookup)[this];
  PrimeAssert(skeletonBones, "Skeleton bones not ready for depth sorting.");

  for(auto const& item: depthSortedItems) {
    refptr<Skinset> skinset = GetSkinsetForBone(item.boneIndex);
    if(skinset) {
      refptr<SkinsetContent> skinsetContent = skinset->GetSkinsetContent();
      const SkinsetContentAffixPieceLookupStack& affixes = GetBoneSkinsetAffixes(skinset, item.boneIndex);
      size_t affixesCount = affixes.GetCount();
      auto pieces = skinset->GetPieces();

      for(size_t j = 0; j < affixesCount; j++) {
        size_t index = affixes.GetItem(j);
        auto piece = (*pieces)[index];
        if(piece && piece->skeleton) {
          piece->skeleton->AddBufferSkeletonDepthBones(param);
        }
      }

      size_t boneIndex = skeletonBones[item.boneIndex];
      const Stack<size_t>* pieceSignatureIndices = param.bonePieceSignatureIndices[boneIndex];
      size_t pieceCount = pieceSignatureIndices->GetCount();
      for(size_t j = 0; j < pieceCount; j++) {
        size_t pieceSignatureIndex = (*pieceSignatureIndices)[j];
        SkeletonPieceSignature* signature = &param.pieceSignatures[pieceSignatureIndex];
        size_t vertexIndexStart = signature->vertexIndexStart;

        if(indexFormat == IndexFormatSize8) {
          u8 indexData[6];
          u8 vis = (u8) vertexIndexStart;
          indexData[0] = vis;
          indexData[1] = vis + 1;
          indexData[2] = vis + 2;
          indexData[3] = vis;
          indexData[4] = vis + 2;
          indexData[5] = vis + 3;
          param.ib->SetValues(param.pieceIndex * 6, 6, indexData);
        }
        else if(indexFormat == IndexFormatSize16) {
          u16 indexData[6];
          u16 vis = (u16) vertexIndexStart;
          indexData[0] = vis;
          indexData[1] = vis + 1;
          indexData[2] = vis + 2;
          indexData[3] = vis;
          indexData[4] = vis + 2;
          indexData[5] = vis + 3;
          param.ib->SetValues(param.pieceIndex * 6, 6, indexData);
        }
        else {
          u32 indexData[6];
          u32 vis = (u32) vertexIndexStart;
          indexData[0] = vis;
          indexData[1] = vis + 1;
          indexData[2] = vis + 2;
          indexData[3] = vis;
          indexData[4] = vis + 2;
          indexData[5] = vis + 3;
          param.ib->SetValues(param.pieceIndex * 6, 6, indexData);
        }

        param.pieceIndex++;
      }
    }
  }
}

size_t Skeleton::LookupTexIndex(refptr<ImagemapContent> imc, Dictionary<refptr<ImagemapContent>, size_t>* lookup, refptr<ImagemapContent>* list) {
  if(auto it = lookup->Find(imc)) {
    return it.value();
  }
  else {
    size_t result = lookup->GetCount();
    if(result < PRIME_SKELETON_PROGRAM_TEX_UNIT_COUNT) {
      list[result] = imc;
      (*lookup)[imc] = result;
      return result;
    }
    else {
#if defined(_DEBUG)
      std::string msg = "Too many skeleton skinset textures:\n";
      msg += string_printf("Existing textures (max %d) are:\n", PRIME_SKELETON_PROGRAM_TEX_UNIT_COUNT);
      PrimeAssert(false, msg.c_str());
#endif
      return 0;
    }
  }
}

void Skeleton::SetProcessingModeInTree(SkeletonProcessingMode mode, size_t minDepth, size_t depth) {
  if(depth >= minDepth) {
    processingMode = mode;
  }

  size_t boneCount = content->GetBoneCount();
  for(size_t i = 0; i < boneCount; i++) {
    refptr<Skinset> skinset = GetSkinsetForBone(i);
    if(skinset) {
      const SkinsetContentAffixPieceLookupStack& affixes = GetBoneSkinsetAffixes(skinset, i);
      size_t affixesCount = affixes.GetCount();
      auto pieces = skinset->GetPieces();

      for(size_t j = 0; j < affixesCount; j++) {
        size_t index = affixes.GetItem(j);
        auto piece = (*pieces)[index];
        if(piece && piece->skeleton) {
          piece->skeleton->SetProcessingModeInTree(mode, minDepth, depth + 1);
        }
      }
    }
  }
}

void Skeleton::UpdateProgramBoneData(DeviceProgram* deviceProgram) {
  static const std::string boneTransform1Str("boneTransform1");
  static const std::string boneTransform2Str("boneTransform2");
  static const std::string boneTransformStr("boneTransform");

  if(deviceProgram && programDataBoneCount > 0) {
    deviceProgram->SetArrayVariable3fv(boneTransform1Str, programData1, programDataBoneCount);
    deviceProgram->SetArrayVariable3fv(boneTransform2Str, programData2, programDataBoneCount);
    deviceProgram->SetArrayVariableMat44fv(boneTransformStr, (f32*) skeletonBoneRootTransforms, programDataBoneCount);
  }
}
