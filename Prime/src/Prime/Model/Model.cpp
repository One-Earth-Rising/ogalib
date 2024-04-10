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

#include <Prime/Model/Model.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/Graphics.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define MODEL_DEFAULT_LAST_POSE_BLEND_TIME 0.1f

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class ModelWeightSortItem {
public:

  Model* model;
  f32 weight;

  ModelWeightSortItem(Model* model = nullptr, f32 weight = 0.0f): model(model), weight(weight) {}
  ModelWeightSortItem(const ModelWeightSortItem& other) {
    model = other.model;
    weight = other.weight;
  }

  ModelWeightSortItem& operator=(const ModelWeightSortItem& other) {
    model = other.model;
    weight = other.weight;

    return *this;
  }
  
  bool operator>(const ModelWeightSortItem& other) const {
    return weight < other.weight;
  }
  
  bool operator<(const ModelWeightSortItem& other) const {
    return weight > other.weight;
  }

  bool operator==(const ModelWeightSortItem& other) const {
    return weight == other.weight;
  }

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Model::Model():
lastActionPoseBlendCtr(0.0f),
lastActionPoseBlendTime(0.0f),
nextActionPoseBlendCanceled(false),
knownActionKeyFrame1(nullptr),
knownActionKeyFrame2(nullptr),
knownPoseBlendWeight(0.0f),
boneOverrides(nullptr),
actionSceneNameKnown(false),
actionIndex(PrimeNotFound),
actionChanged(false),
actionTimeScale(1.0f),
actionCtr(0.0f),
actionLoopedCtr(0.0f),
actionLen(0.0f),
actionLoopCount(0),
actionPlayed(false),
actionReverse(false),
activeBoneTransforms(nullptr),
boneTransforms(nullptr),
activeMeshCount(0),
activeBoneCount(0),
totalBoneCount(0),
textureFilteringEnabled(true),
uniformBaseScale(0.0f),
uniformBaseScaleCached(false),
vertexMin(Vec3(0.0f, 0.0f, 0.0f)),
vertexMax(Vec3(0.0f, 0.0f, 0.0f)) {

}

Model::~Model() {
  DestroyBoneTransforms();
}

void Model::SetContent(Content* content) {
  SetContent(dynamic_cast<ModelContent*>(content));
}

void Model::SetContent(ModelContent* content) {
  DestroyBoneTransforms();
  RemoveAllTextureOverrides();

  currActionPose1.SetContent(nullptr, 0);
  currActionPose2.SetContent(nullptr, 0);
  currActionPoseI.SetContent(nullptr, 0);
  lastActionPose.SetContent(nullptr, 0);
  lastActionPoseTemp.SetContent(nullptr, 0);

  lastActionPoseBlendCtr = 0.0f;
  lastActionPoseBlendTime = 0.0f;
  nextActionPoseBlendCanceled = false;
  knownActionKeyFrame1 = nullptr;
  knownActionKeyFrame2 = nullptr;
  knownPoseBlendWeight = 0.0f;
  boneOverrides = nullptr;
  boneCancelActionBlend.Clear();
  boneCancelNextActionBlend.Clear();

  actionSceneName.clear();
  actionSceneNameKnown = false;
  actionIndex = PrimeNotFound;
  actionChanged = false;
  actionTimeScale = 1.0f;
  actionCtr = 0.0f;
  actionLoopedCtr = 0.0f;
  actionLen = 0.0f;
  actionLoopCount = 0;
  actionPlayed = false;
  actionReverse = false;
  mappedActionName.Clear();

  activeBoneTransforms = nullptr;
  boneTransforms = nullptr;
  activeMeshCount = 0;
  activeBoneCount = 0;
  totalBoneCount = 0;

  textureOverrides.Clear();
  textureFilteringEnabled = true;

  meshTransforms.Clear();

  uniformBaseScale = 0.0f;
  uniformBaseScaleCached = false;

  vertexMin = Vec3(0.0f, 0.0f, 0.0f);
  vertexMax = Vec3(0.0f, 0.0f, 0.0f);

  this->content = content;

  if(!content)
    return;

  size_t actionCount = content->GetActionCount();
  if(actionCount > 0) {
    SetActionByIndex(0);

    lastActionPose.Copy(currActionPoseI);
    lastActionPoseBlendCtr = 0.0f;
    lastActionPoseBlendTime = 0.0f;

    CalcPose(0.0f);

    const ModelContentScene* activeScene = GetActiveScene();
    if(activeScene) {
      vertexMin = activeScene->GetVertexMin();
      vertexMax = activeScene->GetVertexMax();
    }
  }
}

void Model::Calc(f32 dt) {
  if(!HasContent())
    return;

  f32 dtAmount = dt * actionTimeScale;

  if(actionIndex != PrimeNotFound) {
    const ModelContentAction& action = content->GetAction(actionIndex);
    if(action.speedScale != 1.0f) {
      dtAmount *= action.speedScale;
    }
  }

  actionCtr += dtAmount;
  actionLoopedCtr += dtAmount;

  if(actionLen > 0.0f) {
    while(actionCtr >= actionLen) {
      const ModelContentAction& action = content->GetAction(actionIndex);
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
}

void Model::Draw() {
  if(!HasContent())
    return;

  const ModelContentScene* scenePtr = GetActiveScene();
  if(scenePtr) {
    const ModelContentScene& scene = *scenePtr;
    Graphics& g = PxGraphics;

    g.model.Push().Multiply(scene.GetBaseTransform());

    for(size_t i = 0; i < scene.GetMeshCount(); i++) {
      const ModelContentMesh& mesh = scene.GetMesh(i);
      DrawMesh(mesh, mesh.GetMeshIndex());
    }

    g.model.Pop();
  }
}

f32 Model::GetUniformSize() const {
  const Vec3& vertexMin = GetVertexMin();
  const Vec3& vertexMax = GetVertexMax();

  f32 sizeX = vertexMax.x - vertexMin.x;
  f32 sizeY = vertexMax.y - vertexMin.y;
  f32 sizeZ = vertexMax.z - vertexMin.z;
  f32 size = max(max(sizeX, sizeY), sizeZ);

  return size;
}

void Model::SetAction(const std::string& name) {
  if(!HasContent())
    return;

  size_t actionCount = content->GetActionCount();

  if(!name.empty()) {
    for(size_t i = 0; i < actionCount; i++) {
      const ModelContentAction& action = content->GetAction(i);
      if(action.name == name) {
        SetActionByIndex(i);
        return;
      }
      else if(mappedActionName.GetCount() > 0) {
        if(auto it = mappedActionName.Find(action.name)) {
          if(it.value() == name) {
            SetActionByIndex(i);
            return;
          }
        }
      }
    }
  }
  else {
    SetActionByIndex(0);
  }
}

bool Model::SetActionIfNew(const std::string& name) {
  if(!HasContent())
    return false;

  size_t actionCount = content->GetActionCount();

  if(!name.empty()) {
    for(size_t i = 0; i < actionCount; i++) {
      const ModelContentAction& action = content->GetAction(i);
      if(action.name == name && actionIndex != i) {
        SetActionByIndex(i);
        return true;
      }
      else if(mappedActionName.GetCount() > 0) {
        if(auto it = mappedActionName.Find(action.name)) {
          if(it.value() == name && actionIndex != i) {
            SetActionByIndex(i);
            return true;
          }
        }
      }
    }
  }

  return false;
}

void Model::SetActionTime(f32 time) {
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

void Model::SetActionT(f32 t) {
  SetActionTime(GetActionLen() * t);
}

void Model::SetActionTimeScale(f32 scale) {
  actionTimeScale = scale;
  if(actionTimeScale < 0.0f)
    actionTimeScale = 0.0f;
}

void Model::SetActionReverse(bool reverse) {
  actionReverse = reverse;
}

bool Model::DoesActionExist(const std::string& name) {
  if(!HasContent())
    return false;

  size_t actionCount = content->GetActionCount();

  if(!name.empty()) {
    for(size_t i = 0; i < actionCount; i++) {
      const ModelContentAction& action = content->GetAction(i);
      if(action.name == name) {
        return true;
      }
    }
  }

  return false;
}

bool Model::IsInAction(const std::string& name) {
  if(!HasContent())
    return false;

  if(actionIndex != PrimeNotFound) {
    if(content->GetActionCount()) {
      const ModelContentAction& action = content->GetAction(actionIndex);
      return name == action.name;
    }
  }

  return false;
}

size_t Model::GetActionIndex() const {
  return actionIndex;
}

const std::string& Model::GetActionName() const {
  if(HasContent()) {
    if(actionIndex != PrimeNotFound) {
      if(content->GetActionCount()) {
        const ModelContentAction& action = content->GetAction(actionIndex);
        return action.name;
      }
    }
  }

  static const std::string noActionName;
  return noActionName;
}

f32 Model::GetActionLen() const {
  return actionLen;
}

f32 Model::GetActionTime() const {
  return actionCtr;
}

f32 Model::GetActionLoopedTime() const {
  return actionLoopedCtr;
}

f32 Model::GetActionT() const {
  return actionLen ? actionCtr / actionLen : 0.0f;
}

void Model::SetActionByIndex(size_t index) {
  if(!HasContent())
    return;

  if(content->GetActionCount() == 0) {
    DiscardAction();
    return;
  }

  PrimeAssert(index < content->GetActionCount(), "Invalid action index.");
  size_t oldActionIndex = actionIndex;
  bool oldActionPoseBlendAllowed = true;

  const ModelContentSkeleton* oldSkeletonPtr = GetSkeletonByActionIndex(*content, oldActionIndex);

  const ModelContentAction& action = content->GetAction(index);
  const ModelContentScene* scenePtr = GetSceneByActionIndex(*content, index);
  if(!scenePtr) {
    DiscardAction();
    return;
  }
  const ModelContentScene& scene = *scenePtr;

  const ModelContentSkeleton* skeletonPtr = GetSkeletonByActionIndex(*content, index);
  if(!skeletonPtr) {
    DiscardAction();
    return;
  }
  const ModelContentSkeleton& skeleton = *skeletonPtr;

  if(oldActionIndex != PrimeNotFound) {
    const ModelContentAction& oldAction = content->GetAction(oldActionIndex);
    oldActionPoseBlendAllowed = oldAction.nextPoseBlendAllowed;
  }

  bool discardedAction = false;

  if(!actionSceneNameKnown || actionSceneName != action.scene) {
    if(oldSkeletonPtr == nullptr) {
      discardedAction = true;
    }
    else if(oldSkeletonPtr->GetSignature() != skeletonPtr->GetSignature()) {
      discardedAction = true;
    }

    if(discardedAction) {
      DiscardAction();

      size_t meshCount = scene.GetMeshCount();
      size_t boneCount = skeleton.GetBoneCount();
      size_t actionPoseBoneCount = skeleton.GetActionPoseBoneCount();

      if(meshCount) {
        activeMeshCount = meshCount;

        if(actionPoseBoneCount) {
          activeBoneCount = actionPoseBoneCount;
          activeBoneTransforms = new Mat44*[activeMeshCount];
          if(activeBoneTransforms) {
            for(size_t i = 0; i < activeMeshCount; i++) {
              activeBoneTransforms[i] = new Mat44[activeBoneCount];
            }
          }
        }

        if(boneCount) {
          totalBoneCount = boneCount;
          boneTransforms = new Mat44*[activeMeshCount];
          if(boneTransforms) {
            for(size_t i = 0; i < activeMeshCount; i++) {
              boneTransforms[i] = new Mat44[totalBoneCount];
            }
          }
        }
      }

      currActionPose1.SetContent(content, index);
      currActionPose2.SetContent(content, index);
      currActionPoseI.SetContent(content, index);
      lastActionPose.SetContent(content, index);
      lastActionPoseTemp.SetContent(content, index);
    }
    else {
      lastActionPose.Copy(currActionPoseI);
    }

    actionSceneName = scene.GetName();
    actionSceneNameKnown = true;
  }
  else {
    lastActionPose.Copy(currActionPoseI);
  }

  actionIndex = index;
  actionChanged = true;
  actionCtr = 0.0f;
  actionLoopedCtr = 0.0f;
  actionLen = 0.0f;
  s32 oldLoopCount = actionLoopCount;
  actionLoopCount = 0;
  actionPlayed = false;

  const ModelContentSkeletonAction* skeletonAction = skeleton.GetActionByName(action.sceneActionName);
  if(skeletonAction) {
    actionLen = skeletonAction->GetLen();
  }
  else {
    actionLen = 0.0f;
  }

  if(oldActionPoseBlendAllowed && !nextActionPoseBlendCanceled && !discardedAction) {
    lastActionPoseBlendTime = action.lastPoseBlendTimeSpecified ? action.lastPoseBlendTime : MODEL_DEFAULT_LAST_POSE_BLEND_TIME;
  }
  else {
    lastActionPoseBlendTime = 0.0f;
  }
  lastActionPoseBlendCtr = lastActionPoseBlendTime;
  nextActionPoseBlendCanceled = false;

  if(skeletonAction) {
    const ModelContentSkeletonActionKeyFrame* keyFrame1;
    const ModelContentSkeletonActionKeyFrame* keyFrame2;
    f32 weight;

    GetActionKeyFrames(skeleton, *skeletonAction, &keyFrame1, &keyFrame2, &weight);

    if(keyFrame1 && lastActionPoseBlendTime == 0.0f && knownActionKeyFrame1) {
      const ModelContentSkeletonPose& pose1 = skeleton.GetPose(knownActionKeyFrame1->GetPoseIndex());
      currActionPoseI.Copy(pose1);
    }

    currActionPose1.Copy(currActionPoseI);
    currActionPose2.Copy(currActionPoseI);
  }

  const ModelContentAction& newAction = content->GetAction(actionIndex);
  std::string prevActionName;
  std::string nextActionName = newAction.name;

  if(oldActionIndex >= 0 && oldActionIndex != PrimeNotFound) {
    const ModelContentAction& oldAction = content->GetAction(oldActionIndex);
    prevActionName = oldAction.name;
  }

  if(discardedAction) {
    CalcPose(0.0f);
  }
}

void Model::ResetActionChanged() {
  actionChanged = false;
}

bool Model::HasActionChanged() {
  bool result = actionChanged;
  actionChanged = false;
  return result;
}

void Model::CancelLastActionBlend() {
  lastActionPoseBlendCtr = 0.0f;
  CalcPose(0.0f);
}

void Model::MapActionName(const std::string& name, const std::string& mappedToName) {
  mappedActionName[name] = mappedToName;
}

const ModelContentScene* Model::GetActiveScene() const {
  if(!HasContent())
    return nullptr;

  if(actionIndex == PrimeNotFound) {
    if(content->GetSceneCount() > 0) {
      return &content->GetScene(0);
    }
    else {
      return nullptr;
    }
  }
  else {
    if(actionIndex >= content->GetActionCount())
      return nullptr;

    const ModelContentAction& action = content->GetAction(actionIndex);

    size_t sceneIndex = content->GetSceneIndexByName(action.scene);
    if(sceneIndex == PrimeNotFound)
      return nullptr;

    return &content->GetScene(sceneIndex);
  }
}

const ModelContentSkeleton* Model::GetActiveSkeleton(const ModelContentScene** associatedScene) const {
  if(!HasContent())
    return nullptr;

  if(actionIndex == PrimeNotFound)
    return nullptr;

  if(actionIndex >= content->GetActionCount())
    return nullptr;

  const ModelContentAction& action = content->GetAction(actionIndex);

  size_t sceneIndex = content->GetSceneIndexByName(action.scene);
  if(sceneIndex == PrimeNotFound)
    return nullptr;

  const ModelContentScene& scene = content->GetScene(sceneIndex);
  if(associatedScene) {
    *associatedScene = &scene;
  }

  const ModelContentSkeleton* skeleton = nullptr;
  if(scene.GetSkeletonCount() > 0) {
    skeleton = &scene.GetSkeleton(0);
  }

  return skeleton;
}

const ModelContentScene* Model::GetSceneByActionIndex(const ModelContent& content, size_t actionIndex) const {
  if(actionIndex == PrimeNotFound)
    return nullptr;

  if(actionIndex >= content.GetActionCount())
    return nullptr;

  const ModelContentAction& action = content.GetAction(actionIndex);

  size_t sceneIndex = content.GetSceneIndexByName(action.scene);
  if(sceneIndex == PrimeNotFound)
    return nullptr;

  return &content.GetScene(sceneIndex);
}

const ModelContentSkeleton* Model::GetSkeletonByActionIndex(const ModelContent& content, size_t actionIndex) const {
  if(actionIndex == PrimeNotFound)
    return nullptr;

  if(actionIndex >= content.GetActionCount())
    return nullptr;

  const ModelContentAction& action = content.GetAction(actionIndex);

  size_t sceneIndex = content.GetSceneIndexByName(action.scene);
  if(sceneIndex == PrimeNotFound)
    return nullptr;

  const ModelContentScene& scene = content.GetScene(sceneIndex);

  const ModelContentSkeleton* skeleton = nullptr;
  if(scene.GetSkeletonCount() > 0) {
    skeleton = &scene.GetSkeleton(0);
  }

  return skeleton;
}

void Model::CalcPose(f32 dt) {
  if(lastActionPoseBlendCtr) {
    lastActionPoseBlendCtr -= dt;
    if(lastActionPoseBlendCtr < 0.0f) {
      lastActionPoseBlendCtr = 0.0f;
      lastActionPoseBlendTime = 0.0f;
    }
  }

  if(HasContent()) {
    const ModelContentSkeleton* skeletonPtr = GetActiveSkeleton();
    if(skeletonPtr) {
      const ModelContentSkeleton& skeleton = *skeletonPtr;
      const ModelContentAction& action = content->GetAction(actionIndex);
      const ModelContentSkeletonAction* skeletonAction = skeleton.GetActionByName(action.sceneActionName);
      if(skeletonAction) {
        size_t keyFrameCount = skeletonAction->GetKeyFrameCount();
        if(keyFrameCount >= 2) {
          const ModelContentSkeletonActionKeyFrame* keyFrame1;
          const ModelContentSkeletonActionKeyFrame* keyFrame2;
          f32 weight;
          GetActionKeyFrames(skeleton, *skeletonAction, &keyFrame1, &keyFrame2, &weight);
          const ModelContentSkeletonPose& pose1 = skeleton.GetPose(keyFrame1->GetPoseIndex());
          const ModelContentSkeletonPose& pose2 = skeleton.GetPose(keyFrame2->GetPoseIndex());

          if(!knownActionKeyFrame1 || knownActionKeyFrame1 != keyFrame1) {
            knownActionKeyFrame1 = keyFrame1;
            currActionPose1.Copy(pose1);
          }

          if(!knownActionKeyFrame2 || knownActionKeyFrame2 != keyFrame2) {
            knownActionKeyFrame2 = keyFrame2;
            currActionPose2.Copy(pose2);
          }

          knownPoseBlendWeight = weight;
          currActionPoseI.Interpolate(currActionPose1, currActionPose2, knownPoseBlendWeight);

          if(lastActionPoseBlendCtr > 0.0f && lastActionPoseBlendTime > 0.0f) {
            f32 t = lastActionPoseBlendCtr / lastActionPoseBlendTime;
            lastActionPoseTemp.Copy(currActionPoseI);
            currActionPoseI.Interpolate(lastActionPoseTemp, lastActionPose, t, &boneCancelActionBlend);
          }

          size_t rootBoneIndex = skeleton.GetRootBoneIndex();
          if(rootBoneIndex != PrimeNotFound) {
            for(size_t j = 0; j < activeMeshCount; j++) {
              Mat44 transformation;
              transformation.LoadIdentity();
              UpdateBoneTransformsForModelPose(*content, skeleton, j, rootBoneIndex, transformation, currActionPoseI);
            }
          }
        }
      }
      else {
        for(size_t j = 0; j < activeMeshCount; j++) {
          for(size_t i = 0; i < activeBoneCount; i++) {
            activeBoneTransforms[j][i].LoadIdentity();
          }
          for(size_t i = 0; i < totalBoneCount; i++) {
            boneTransforms[j][i].LoadIdentity();
          }
        }
      }
    }
  }
}

void Model::ApplyTextureOverride(const std::string& meshName, refptr<Tex> tex) {
  textureOverrides.Remove(meshName);

  if(tex) {
    textureOverrides[meshName] = tex;
    tex->SetFilteringEnabled(textureFilteringEnabled);
  }
}

void Model::RemoveTextureOverride(const std::string& meshName) {
  textureOverrides.Remove(meshName);
}

void Model::RemoveAllTextureOverrides() {
  textureOverrides.Clear();
}

void Model::SetTextureFilteringEnabled(bool enabled) {
  for(auto it: textureOverrides) {
    Tex* tex = it.value();
    tex->SetFilteringEnabled(enabled);
  }
}

void Model::SetMeshTransform(const std::string& name, const Mat44& mat) {
  meshTransforms[name] = mat;
}

void Model::ClearMeshTransform(const std::string& name) {
  meshTransforms.Remove(name);
}

void Model::DrawMesh(const ModelContentMesh& mesh, size_t meshIndex) {
  static const std::string boneTransformStr("boneTransform");
  Graphics& g = PxGraphics;

  refptr<Tex> directTex;

  if(auto it = textureOverrides.Find(mesh.GetName())) {
    directTex = it.value();
  }
  else {
    directTex = mesh.GetDirectTex();
  }

  if(!directTex) {
    for(auto it: textureOverrides) {
      directTex = it.value();
      if(directTex) {
        break;
      }
    }
  }

  if(!directTex) {
    const ModelContentScene* activeScene = GetActiveScene();
    if(activeScene) {
      size_t textureCount = activeScene->GetTextureCount();
      if(textureCount > 0) {
        for(size_t i = 0; i < textureCount; i++) {
          auto texture = activeScene->GetTexture(i);

          if(texture) {
            directTex = texture;
            break;
          }
        }
      }
    }
  }

  if(!directTex)
    return;

  bool anim = mesh.GetAnim();

  DeviceProgram* program = g.program;
  if(!program)
    return;

  if(anim && meshIndex < activeMeshCount) {
    program->SetArrayVariableMat44fv(boneTransformStr, (f32*) activeBoneTransforms[meshIndex][0].e, activeBoneCount);
  }

  bool pushedColorScale = false;

  g.model.Push().Multiply(mesh.GetBaseTransform());

  if(auto it = meshTransforms.Find(mesh.name))
    g.model.Multiply(it.value());

  g.Draw(mesh.ab, mesh.ib, directTex);

  g.model.Pop();
}

const Mat44* Model::GetActiveBoneTransform(size_t meshIndex, size_t activePoseBoneIndex) const {
  if(!HasContent())
    return nullptr;

  if(meshIndex == PrimeNotFound || activePoseBoneIndex == PrimeNotFound)
    return nullptr;

  if(meshIndex < activeMeshCount) {
    if(activePoseBoneIndex < activeBoneCount) {
      return &activeBoneTransforms[meshIndex][activePoseBoneIndex];
    }
  }

  return nullptr;
}

const Mat44* Model::GetBoneTransform(size_t meshIndex, size_t boneIndex) const {
  if(!HasContent())
    return nullptr;

  if(meshIndex == PrimeNotFound || boneIndex == PrimeNotFound)
    return nullptr;

  if(meshIndex < activeMeshCount) {
    if(boneIndex < totalBoneCount) {
      return &boneTransforms[meshIndex][boneIndex];
    }
  }

  return nullptr;
}

f32 Model::GetUniformBaseScale(bool cached) {
  if(cached && uniformBaseScaleCached)
    return uniformBaseScale;

  f32 result = 1.0f;

  if(HasContent()) {
    if(content->GetSceneCount() > 0) {
      const ModelContentScene& scene = content->GetScene(0);
      Mat44 sceneBaseTransform = scene.GetBaseTransform();
      f32 sceneBaseScale = sqrtf(sceneBaseTransform.e11 * sceneBaseTransform.e11 + sceneBaseTransform.e21 * sceneBaseTransform.e21 + sceneBaseTransform.e31 * sceneBaseTransform.e31);
      result *= sceneBaseScale;

      if(scene.GetMeshCount() > 0) {
        const ModelContentMesh& mesh = scene.GetMesh(0);
        Mat44 meshBaseTransform = mesh.GetBaseTransform();
        f32 meshBaseScale = sqrtf(meshBaseTransform.e11 * meshBaseTransform.e11 + meshBaseTransform.e21 * meshBaseTransform.e21 + meshBaseTransform.e31 * meshBaseTransform.e31);
        result *= meshBaseScale;
      }
    }
  }

  uniformBaseScaleCached = true;
  uniformBaseScale = result;

  return result;
}

void Model::DiscardAction() {
  DestroyBoneTransforms();

  actionSceneName.clear();
  actionSceneNameKnown = false;
  actionIndex = PrimeNotFound;
  actionChanged = false;
  actionCtr = 0.0f;
  actionLen = 0.0f;
  actionLoopCount = 0;
  actionPlayed = false;

  currActionPose1.SetContent(content, PrimeNotFound);
  currActionPose2.SetContent(content, PrimeNotFound);
  currActionPoseI.SetContent(content, PrimeNotFound);
  lastActionPose.SetContent(content, PrimeNotFound);
  lastActionPoseTemp.SetContent(content, PrimeNotFound);

  knownActionKeyFrame1 = nullptr;
  knownActionKeyFrame2 = nullptr;
  knownPoseBlendWeight = 0.0f;
}

void Model::GetActionKeyFrames(const ModelContentSkeleton& skeleton, const ModelContentSkeletonAction& skeletonAction, const ModelContentSkeletonActionKeyFrame** keyFrame1, const ModelContentSkeletonActionKeyFrame** keyFrame2, f32* weight) {
  size_t keyFrameCount = skeletonAction.GetKeyFrameCount();
  f32 useActionCtr;

  if(actionReverse) {
    f32 actionT = actionCtr / (f32) actionLen;
    useActionCtr = (1.0f - actionT) * actionLen;
  }
  else {
    useActionCtr = actionCtr;
  }

  if(useActionCtr > actionLen) {
    useActionCtr = actionLen;
  }

  const ModelContentSkeletonActionKeyFrame* kf1 = &skeletonAction.GetKeyFrame(0);
  const ModelContentSkeletonActionKeyFrame* kf2 = kf1;
  const ModelContentSkeletonActionKeyFrame& firstKeyFrame = skeletonAction.GetKeyFrame(0);
  f32 firstKeyFrame1Time = firstKeyFrame.GetTime();
  f32 keyFrame1Time = kf1->GetTime() - firstKeyFrame1Time;
  f32 keyFrame2Time = keyFrame1Time;

  size_t kf1Index = 0;
  size_t kf2Index = 0;

  for(size_t i = 0; i < keyFrameCount; i++) {
    const ModelContentSkeletonActionKeyFrame& keyFrame = skeletonAction.GetKeyFrame(i);
    const ModelContentSkeletonActionKeyFrame& nextKeyFrame = skeletonAction.GetKeyFrame((i == keyFrameCount - 1) ? i : (i + 1));
    f32 keyFrameTime = keyFrame.GetTime() - firstKeyFrame1Time;
    f32 nextKeyFrameTime = nextKeyFrame.GetTime() - firstKeyFrame1Time;
    if(keyFrameTime >= 0.0f) {
      if(actionReverse) {
        if(useActionCtr < nextKeyFrameTime) {
          kf1 = &keyFrame;
          if(i == 0) {
            kf2 = &skeletonAction.GetKeyFrame(keyFrameCount - 1);
          }
          else {
            kf2 = &skeletonAction.GetKeyFrame(i - 1);
          }
          break;
        }
      }
      else {
        if(useActionCtr < nextKeyFrameTime) {
          kf1 = &keyFrame;
          kf1Index = i;
          if(i == keyFrameCount - 1) {
            size_t nextIndex = 0;
            do {
              kf2 = &skeletonAction.GetKeyFrame(nextIndex);
              kf2Index = nextIndex;
              nextIndex++;
            }
            while(kf2->GetTime() < 0.0f);
          }
          else {
            kf2 = &skeletonAction.GetKeyFrame(i + 1);
            kf2Index = i + 1;
          }
          keyFrame2Time = nextKeyFrameTime;
          break;
        }
      }
    }

    kf1 = &keyFrame;
    kf1Index = i;
    keyFrame1Time = nextKeyFrameTime;
  }

  const ModelContentAction& action = content->GetAction(actionIndex);
  if(!action.loop) {
    if(actionReverse) {
      if(kf1Index == 0) {
        kf2 = kf1;
        kf2Index = kf1Index;
        keyFrame2Time = keyFrame1Time;
      }
    }
    else {
      if(kf1Index == keyFrameCount - 1) {
        kf2 = kf1;
        kf2Index = kf1Index;
        keyFrame2Time = keyFrame1Time;
      }
    }
  }

  if(keyFrame1)
    *keyFrame1 = kf1;

  if(keyFrame2)
    *keyFrame2 = kf2;

  if(weight) {
    if(kf1 == kf2 || kf1->GetTime() == kf2->GetTime()) {
      *weight = 0.0f;
    }
    else {
      f32 useWeight = (useActionCtr - keyFrame1Time) / (f32) (keyFrame2Time - keyFrame1Time);
      useWeight = clamp(useWeight, 0.0f, 1.0f);
      *weight = useWeight;
    }
  }
}

void Model::UpdateBoneTransformsForPoses(const ModelContent& content, const ModelContentSkeleton& skeleton, size_t meshIndex, size_t boneIndex, Mat44 transformation, const ModelContentSkeletonPose* pose1, const ModelContentSkeletonPose* pose2, f32 t) {
  Mat44 boneTransformation;
  boneTransformation.LoadIdentity();
  const ModelContentSkeletonBone& bone = skeleton.GetBone(boneIndex);
  size_t actionPoseBoneIndex = bone.GetActionPoseBoneIndex();

  Mat44 poseTransform;
  poseTransform.LoadIdentity();
  bool defaultPoseTransform = true;

  const ModelContentSkeletonPoseBone& poseBone1 = pose1->GetPoseBone(boneIndex);
  const ModelContentSkeletonPoseBone& poseBone2 = pose2->GetPoseBone(boneIndex);
  if(poseBone1.GetBoneIndex() != PrimeNotFound && poseBone2.GetBoneIndex() != PrimeNotFound) {
    if(poseBone1 == poseBone2) {
      poseTransform.Translate(poseBone1.GetTranslation());
      poseTransform.Multiply(poseBone1.GetRotation().GetRotationMat44());
      poseTransform.Scale(poseBone1.GetScaling());
      defaultPoseTransform = false;
    }
    else {
      Vec3 translation = poseBone1.GetTranslation().GetLerp(poseBone2.GetTranslation(), t);
      Quat rotation = poseBone1.GetRotation().Interpolate(poseBone2.GetRotation(), t);
      Vec3 scaling = poseBone1.GetScaling().GetLerp(poseBone2.GetScaling(), t);
      poseTransform.Translate(translation);
      poseTransform.Multiply(rotation.GetRotationMat44());
      poseTransform.Scale(scaling);
      defaultPoseTransform = false;
    }
  }

  if(defaultPoseTransform) {
    poseTransform = bone.GetTransformation();
  }

  boneTransformation = transformation * poseTransform;

  if(actionPoseBoneIndex != PrimeNotFound) {
    if(bone.IsMeshTransformationValid(meshIndex)) {
      activeBoneTransforms[meshIndex][actionPoseBoneIndex] = boneTransformation * bone.GetMeshTransformation(meshIndex);
    }
    else {
      activeBoneTransforms[meshIndex][actionPoseBoneIndex] = boneTransformation;
    }
  }
  boneTransforms[meshIndex][boneIndex] = boneTransformation;

  size_t childBoneIndexCount = bone.GetChildBoneIndexCount();
  for(size_t i = 0; i < childBoneIndexCount; i++) {
    size_t childBoneIndex = bone.GetChildBoneIndex(i);
    UpdateBoneTransformsForPoses(content, skeleton, meshIndex, childBoneIndex, boneTransformation, pose1, pose2, t);
  }
}

void Model::UpdateBoneTransformsForModelPose(const ModelContent& content, const ModelContentSkeleton& skeleton, size_t meshIndex, size_t boneIndex, Mat44 transformation, const ModelPose& pose) {
  Mat44 boneTransformation;
  boneTransformation.LoadIdentity();
  const ModelContentSkeletonBone& bone = skeleton.GetBone(boneIndex);
  size_t actionPoseBoneIndex = bone.GetActionPoseBoneIndex();

  Mat44 poseTransform;
  poseTransform.LoadIdentity();
  bool defaultPoseTransform = true;

  const ModelPoseBone* poseBone = pose.GetBone(boneIndex);
  if(poseBone && poseBone->poseValid) {
    poseTransform.Translate(poseBone->translation);
    poseTransform.Multiply(poseBone->rotation.GetRotationMat44());
    poseTransform.Scale(poseBone->scaling);
    defaultPoseTransform = false;
  }

  if(defaultPoseTransform) {
    poseTransform = bone.GetTransformation();
  }

  boneTransformation = transformation * poseTransform;

  if(actionPoseBoneIndex != PrimeNotFound) {
    if(bone.IsMeshTransformationValid(meshIndex)) {
      activeBoneTransforms[meshIndex][actionPoseBoneIndex] = boneTransformation * bone.GetMeshTransformation(meshIndex);
    }
    else {
      activeBoneTransforms[meshIndex][actionPoseBoneIndex] = boneTransformation;
    }
  }
  boneTransforms[meshIndex][boneIndex] = boneTransformation;

  size_t childBoneIndexCount = bone.GetChildBoneIndexCount();
  for(size_t i = 0; i < childBoneIndexCount; i++) {
    size_t childBoneIndex = bone.GetChildBoneIndex(i);
    UpdateBoneTransformsForModelPose(content, skeleton, meshIndex, childBoneIndex, boneTransformation, pose);
  }
}

void Model::DestroyBoneTransforms() {
  if(activeBoneTransforms) {
    for(size_t i = 0; i < activeMeshCount; i++) {
      PrimeSafeDeleteArray(activeBoneTransforms[i]);
    }
    PrimeSafeDeleteArray(activeBoneTransforms);
  }

  if(boneTransforms) {
    for(size_t i = 0; i < activeMeshCount; i++) {
      PrimeSafeDeleteArray(boneTransforms[i]);
    }
    PrimeSafeDeleteArray(boneTransforms);
  }

  activeMeshCount = 0;
  activeBoneCount = 0;
}
