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

#include <Prime/Asset/Asset.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/Graphics.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

static const std::initializer_list<std::string> AcceptedTextureFormats = {
  "png",
  "bc",
};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Asset::Asset():
parent(nullptr),
textureFilteringEnabled(true),
loadingCount(0),
loadQueuedId(PrimeNotFound) {
  SetAcceptedTextureFormats(AcceptedTextureFormats);
}

Asset::~Asset() {

}

void Asset::SetParent(Asset* parent) {
  this->parent = parent;
}

void Asset::SetAPIRoot(const std::string& apiRoot) {
  this->apiRoot = apiRoot;
}

void Asset::SetTexProgram(refptr<DeviceProgram> program) {
  texProgram = program;
}

void Asset::SetSkeletonProgram(refptr<DeviceProgram> program) {
  skeletonProgram = program;
}

void Asset::SetModelProgram(refptr<DeviceProgram> program) {
  modelProgram = program;
}

void Asset::SetModelAnimProgram(refptr<DeviceProgram> program) {
  modelAnimProgram = program;
}

void Asset::SetAcceptedTextureFormats(const Stack<std::string>& formats) {
  acceptedTextureFormats.Clear();
  for(auto& format: formats) {
    acceptedTextureFormats.Add(format);
  }
}

void Asset::SetTextureFilteringEnabled(bool enabled) {
  textureFilteringEnabled = enabled;

  for(auto asset: dataManifestAssets) {
    asset->SetTextureFilteringEnabled(enabled);
  }

  if(imagemap) {
    imagemap->SetFilteringEnabled(textureFilteringEnabled);
  }
  else if(model) {
    model->SetTextureFilteringEnabled(textureFilteringEnabled);
  }
}

void Asset::Load(size_t id) {
  const std::string& useAPIRoot = GetAPIRoot();

  if(useAPIRoot.empty())
    return;

  if(loadingCount > 0) {
    loadQueuedId = id;
    return;
  }

  imagemap = nullptr;
  skeleton = nullptr;
  model = nullptr;
  rig = nullptr;

  info.object();
  uri.clear();
  format.clear();

  dataManifest.array();
  dataManifestAssets.Clear();

  IncLoading();
  SendURL(string_printf("%s/GetAssetInfo/v1/?id=%d", useAPIRoot.c_str(), id), [=](const json& response) {
    if(auto it = response.find("data")) {
      if(info.parse(it.c_str())) {
        if(auto itParentURL = info.find("parentURL")) {
          std::string parentURI = itParentURL.GetString();
          if(!parentURI.empty()) {
            info["_parentURI"] = parentURI;
          }
        }

        IncLoading();
        SendURL(string_printf("%s/GetAssetDataManifest/v1/?id=%d", useAPIRoot.c_str(), id), [=](const json& response) {
          if(auto it = response.find("data")) {
            if(dataManifest.parse(it.c_str())) {
              // Load the main asset.
              std::string loadURL;
              std::string loadFormat;
              json loadInfo;

              // Search for model file formats.
              for(const auto& item: dataManifest) {
                if(auto itFormat = item.find("format")) {
                  std::string format = itFormat.GetString();

                  if(format == "gltf" || format == "glb") { // prefer GLTF over FBX
                    if(auto itURL = item.find("url")) {
                      loadURL = itURL.GetString();
                      loadFormat = format;
                    }
                  }
                  else if(format == "fbx") {
                    if(loadFormat.empty()) {
                      if(auto itURL = item.find("url")) {
                        loadURL = itURL.GetString();
                        loadFormat = format;
                      }
                    }
                  }
                }
              }

              if(loadURL.empty()) {
                // If the root asset is a PNG, load the PNG instead of a processed asset since it may contain a PPF.
                if(auto itURL = info.find("url")) {
                  std::string url = itURL.GetString();
                  if(EndsWith(url, ".png")) {
                    loadURL = url;
                    loadFormat = "png";
                  }
                }
              }

              // If no model formats found, search for texture formats and treat the asset as an imagemap.
              if(loadURL.empty()) {
                size_t loadW = 0;
                size_t loadH = 0;

                for(const auto& item: dataManifest) {
                  if(auto itFormat = item.find("format")) {
                    std::string format = itFormat.GetString();

                    bool accepted = false;
                    for(auto& acceptedTextureFormat: acceptedTextureFormats) {
                      if(format == acceptedTextureFormat) {
                        accepted = true;
                        break;
                      }
                    }

                    if(accepted) {
                      size_t w = item.find("width").GetUint();
                      size_t h = item.find("height").GetUint();
                      if(w > loadW || h > loadH) {
                        if(auto itURL = item.find("url")) {
                          loadURL = itURL.GetString();
                          loadFormat = format;
                          loadW = w;
                          loadH = h;
                          loadInfo = item;
                        }
                      }
                    }
                  }
                }
              }

              if(loadURL.empty()) {
                if(dataManifest.size() == 0) {
                  if(auto itURL = info.find("url")) {
                    loadURL = itURL.GetString();

                    uri = loadURL;

                    if(auto itFormat = info.find("format")) {
                      format = itFormat.GetString();
                    }

                    if(format.empty()) {
                      format = GetExtension(loadURL);
                    }

                    // No url or data manifest found.  Go to the original asset and load that.
                    IncLoading();
                    GetContent(loadURL, info, [=](Content* content) {
                      if(content->IsInstance<ImagemapContent>()) {
                        auto newImagemap = new Imagemap();
                        imagemap = newImagemap;
                        imagemap->SetContent(content);
                        imagemap->SetRectByIndex(0);
                        imagemap->SetFilteringEnabled(textureFilteringEnabled);
                      }
                      else if(content->IsInstance<SkeletonContent>()) {
                        auto newSkeleton = new Skeleton();
                        skeleton = newSkeleton;
                        skeleton->SetContent(content);

                        std::string parentURI;
                        if(auto it = info.find("_parentURI")) {
                          parentURI = it.GetString();
                        }

                        Stack<std::string> filenames;
                        GetPackFilenames(parentURI, filenames);

                        for(auto& filename: filenames) {
                          if(EndsWith(filename, "Skinset.json")) {
                            IncLoading();
                            GetContent(filename, {{"_parentURI", parentURI}}, [=](Content* content) {
                              if(content->IsInstance<SkinsetContent>()) {
                                refptr newSkinset = new Skinset();
                                newSkinset->SetContent(content);
                                newSkeleton->SetSkinset(newSkinset);
                              }

                              DecLoading();
                            });

                            break;
                          }
                        }
                      }
                      else if(content->IsInstance<ModelContent>()) {
                        model = new Model();
                        model->SetContent(content);
                      }
                      else if(content->IsInstance<RigContent>()) {
                        rig = new Rig();
                        rig->SetContent(content);
                      }

                      DecLoading();
                    });
                  }
                  else {
                    format = "<not found>";
                  }
                }                
              }
              else {
                uri = loadURL;
                format = loadFormat;
                IncLoading();
                GetContent(loadURL, loadInfo, [=](Content* content) {
                  if(content->IsInstance<ModelContent>()) {
                    refptr newModel = new Model();
                    model = newModel;
                    model->SetContent(content);

                    refptr<Tex> modelTex = Tex::Create();
                    modelTex->SetFilteringEnabled(textureFilteringEnabled);
                    model->RemoveAllTextureOverrides();
                    model->ApplyTextureOverride("", modelTex);

                    for(const auto& item: dataManifest) {
                      if(auto itFormat = item.find("format")) {
                        std::string format = itFormat.GetString();

                        bool accepted = false;
                        for(auto& acceptedTextureFormat: acceptedTextureFormats) {
                          if(format == acceptedTextureFormat) {
                            accepted = true;
                            break;
                          }
                        }

                        json itemCopy = item;

                        if(accepted) {
                          u32 w = item.find("width").GetUint();
                          if(auto itName = item.find("name")) {
                            std::string name = itName.GetString();
                            if(auto itURL = item.find("url")) {
                              IncLoading();
                              SendURL(itURL.GetString(), [=](const json& response) {
                                if(auto it = response.find("data")) {
                                  std::string data = it.GetString();
                                  modelTex->AddTexData(name, data, itemCopy);
                                }

                                DecLoading();
                              });
                            }
                          }
                        }
                      }
                    }
                  }
                  else if(content->IsInstance<ImagemapContent>()) {
                    refptr newImagemap = new Imagemap();
                    imagemap = newImagemap;
                    imagemap->SetContent(content);
                    imagemap->SetRectByIndex(0);
                    imagemap->SetFilteringEnabled(textureFilteringEnabled);
                  }
                  
                  DecLoading();
                });
              }

              // Load assets based on the data manifest.
              for(const auto& item: dataManifest) {
                if(auto itId = item.find("id")) {
                  size_t id = itId.GetSizeT();

                  refptr newAsset = new Asset();
                  newAsset->SetParent(this);
                  newAsset->SetTextureFilteringEnabled(textureFilteringEnabled);

                  dataManifestAssets.Add(newAsset);
                  newAsset->Load(id);
                }
              }
            }
          }

          DecLoading();
        });
      }
    }

    DecLoading();
  });
}

size_t Asset::GetActionCount() const {
  if(skeleton) {
    if(skeleton->HasContent()) {
      return skeleton->GetSkeletonContent()->GetActionCount();
    }
  }
  else if(model) {
    if(model->HasContent()) {
      return model->GetModelContent()->GetActionCount();
    }
  }

  return 0;
}

const std::string& Asset::GetActionName() const {
  if(skeleton) {
    return skeleton->GetActionName();
  }
  else if(model) {
    return model->GetActionName();
  }

  static const std::string noActionName;
  return noActionName;
}

size_t Asset::GetActionIndex() const {
  if(skeleton) {
    return skeleton->GetActionIndex();
  }
  else if(model) {
    return model->GetActionIndex();
  }

  return PrimeNotFound;
}

f32 Asset::GetActionLen() const {
  if(skeleton) {
    return skeleton->GetActionLen();
  }
  else if(model) {
    return model->GetActionLen();
  }

  return 0.0f;
}

void Asset::RestartAction() {
  if(skeleton) {
    size_t actionIndex = skeleton->GetActionIndex();
    if(actionIndex != PrimeNotFound) {
      skeleton->SetActionByIndex(actionIndex);
    }
  }
  else if(model) {
    size_t actionIndex = model->GetActionIndex();
    if(actionIndex != PrimeNotFound) {
      model->SetActionByIndex(actionIndex);
    }
  }
}

void Asset::SetNextAction() {
  if(skeleton) {
    if(skeleton->HasContent()) {
      auto skeletonContent = skeleton->GetSkeletonContent();
      size_t actionCount = skeletonContent->GetActionCount();
      if(actionCount > 0) {
        size_t actionIndex = skeleton->GetActionIndex();
        if(actionIndex != PrimeNotFound) {
          if(actionIndex == actionCount - 1) {
            skeleton->SetActionByIndex(0);
          }
          else {
            skeleton->SetActionByIndex(actionIndex + 1);
          }
        }
      }
    }
  }
  else if(model) {
    if(model->HasContent()) {
      auto modelContent = model->GetModelContent();
      size_t actionCount = modelContent->GetActionCount();
      if(actionCount > 0) {
        size_t actionIndex = model->GetActionIndex();
        if(actionIndex != PrimeNotFound) {
          if(actionIndex == actionCount - 1) {
            model->SetActionByIndex(0);
          }
          else {
            model->SetActionByIndex(actionIndex + 1);
          }
        }
      }
    }
  }
}

void Asset::SetPrevAction() {
  if(skeleton) {
    if(skeleton->HasContent()) {
      auto skeletonContent = skeleton->GetSkeletonContent();
      size_t actionCount = skeletonContent->GetActionCount();
      if(actionCount > 0) {
        size_t actionIndex = skeleton->GetActionIndex();
        if(actionIndex != PrimeNotFound) {
          if(actionIndex == 0) {
            skeleton->SetActionByIndex(actionCount - 1);
          }
          else {
            skeleton->SetActionByIndex(actionIndex - 1);
          }
        }
      }
    }
  }
  else if(model) {
    if(model->HasContent()) {
      auto modelContent = model->GetModelContent();
      size_t actionCount = modelContent->GetActionCount();
      if(actionCount > 0) {
        size_t actionIndex = model->GetActionIndex();
        if(actionIndex != PrimeNotFound) {
          if(actionIndex == 0) {
            model->SetActionByIndex(actionCount - 1);
          }
          else {
            model->SetActionByIndex(actionIndex - 1);
          }
        }
      }
    }
  }
}

void Asset::CancelLastActionBlend() {
  if(skeleton) {
    skeleton->CancelLastActionBlend();
  }
  else if(model) {
    model->CancelLastActionBlend();
  }
}

void Asset::Calc(f32 dt) {
  for(auto dmAsset: dataManifestAssets)
    dmAsset->Calc(dt);

  if(skeleton) {
    skeleton->Calc(dt);
  }
  else if(model) {
    model->Calc(dt);
  }
  else if(rig) {
    rig->Calc(dt);
  }
}

void Asset::Draw() {
  Graphics& g = PxGraphics;

  if(imagemap) {
    auto rect = imagemap->GetRect();
    if(rect) {
      g.program.Push() = GetTexProgram();

      imagemap->Draw();

      g.program.Pop();
    }
  }
  else if(skeleton) {
    g.program.Push() = GetSkeletonProgram();
    g.depthMask.Push() = false;

    skeleton->Draw();

    g.depthMask.Pop();
    g.program.Pop();
  }
  else if(model) {
    const ModelContentScene* activeScene = model->GetActiveScene();
    g.program.Push() = (activeScene && activeScene->GetSkeletonCount() > 0) ? GetModelAnimProgram() : GetModelProgram();

    model->Draw();

    g.program.Pop();
  }
  else if(rig) {
    rig->Draw();
  }
}

const std::string& Asset::GetURI() const {
  if(imagemap) {
    if(imagemap->HasContent()) {
      return imagemap->GetImagemapContent()->GetURI();
    }
  }
  else if(skeleton) {
    if(skeleton->HasContent()) {
      return skeleton->GetSkeletonContent()->GetURI();
    }
  }
  else if(model) {
    if(model->HasContent()) {
      return model->GetModelContent()->GetURI();
    }
  }
  else if(rig) {
    if(rig->HasContent()) {
      return rig->GetRigContent()->GetURI();
    }
  }
  else {
    return uri;
  }

  static const std::string noURI;
  return noURI;
}

std::string Asset::GetFormat() const {
  std::string result;

  if(auto itFormat = info.find("format")) {
    result = itFormat.GetString();
  }

  if(result.empty()) {
    result = format;
  }

  return result;
}

f32 Asset::GetUniformSize() const {
  if(imagemap) {
    const Vec3& vertexMin = imagemap->GetVertexMin();
    const Vec3& vertexMax = imagemap->GetVertexMax();

    f32 sizeX = vertexMax.x - vertexMin.x;
    f32 sizeY = vertexMax.y - vertexMin.y;
    f32 size = max(sizeX, sizeY);

    return size;
  }
  else if(skeleton) {
    const Vec3& vertexMin = skeleton->GetVertexMin();
    const Vec3& vertexMax = skeleton->GetVertexMax();

    f32 sizeX = vertexMax.x - vertexMin.x;
    f32 sizeY = vertexMax.y - vertexMin.y;
    f32 sizeZ = vertexMax.z - vertexMin.z;
    f32 size = max(max(sizeX, sizeY), sizeZ);

    return size;
  }
  else if(model) {
    const ModelContentScene* activeScene = model->GetActiveScene();

    Vec3 vertexMin;
    Vec3 vertexMax;

    if(activeScene) {
      vertexMin = activeScene->GetVertexMin();
      vertexMax = activeScene->GetVertexMax();
    }
    else {
      vertexMin = Vec3(0.0f, 0.0f, 0.0f);
      vertexMax = Vec3(0.0f, 0.0f, 0.0f);
    }

    f32 sizeX = vertexMax.x - vertexMin.x;
    f32 sizeY = vertexMax.y - vertexMin.y;
    f32 sizeZ = vertexMax.z - vertexMin.z;
    f32 size = max(max(sizeX, sizeY), sizeZ);

    return size;
  }
  else if(rig) {
    const Vec3& vertexMin = rig->GetVertexMin();
    const Vec3& vertexMax = rig->GetVertexMax();

    f32 sizeX = vertexMax.x - vertexMin.x;
    f32 sizeY = vertexMax.y - vertexMin.y;
    f32 sizeZ = vertexMax.z - vertexMin.z;
    f32 size = max(max(sizeX, sizeY), sizeZ);

    return size;
  }

  return 0.0f;
}

Vec2 Asset::GetViewOffset() const {
  if(imagemap) {
    auto content = imagemap->GetImagemapContent();
    if(content) {
      if(content->GetTex()) {
        auto rect = imagemap->GetRect();
        if(rect) {
          return Vec2((f32) rect->w * 0.5f, (f32) rect->h * 0.5f);
        }
      }
    }
  }
  else if(skeleton) {
    return Vec2(0.0f, GetUniformSize() * 0.5f);
  }
  else if(model) {
    return Vec2(0.0f, GetUniformSize() * 0.5f);
  }
  else if(rig) {
    return Vec2(0.0f, GetUniformSize() * 0.5f);
  }

  return Vec2(0.0f, 0.0f);
}

bool Asset::Is2D() const {
  return imagemap != nullptr || skeleton != nullptr;
}

bool Asset::IsImagemap() const {
  return imagemap != nullptr;
}

bool Asset::IsSkeleton() const {
  return skeleton != nullptr;
}

bool Asset::IsModel() const {
  return model != nullptr;
}

bool Asset::IsRig() const {
  return rig != nullptr;
}

const std::string& Asset::GetAPIRoot() const {
  if(!apiRoot.empty())
    return apiRoot;
  else if(parent)
    return parent->GetAPIRoot();

  return apiRoot;
}

refptr<DeviceProgram> Asset::GetTexProgram() const {
  if(texProgram)
    return texProgram;
  else if(parent)
    return parent->GetTexProgram();

  return nullptr;
}

refptr<DeviceProgram> Asset::GetSkeletonProgram() const {
  if(skeletonProgram)
    return skeletonProgram;
  else if(parent)
    return parent->GetSkeletonProgram();

  return nullptr;
}

refptr<DeviceProgram> Asset::GetModelProgram() const {
  if(modelProgram)
    return modelProgram;
  else if(parent)
    return parent->GetModelProgram();

  return nullptr;
}

refptr<DeviceProgram> Asset::GetModelAnimProgram() const {
  if(modelAnimProgram)
    return modelAnimProgram;
  else if(parent)
    return parent->GetModelAnimProgram();

  return nullptr;
}

Stack<std::string> Asset::SplitString(const std::string& str, const std::string& delim) {
  Stack<std::string> result;
  const char* s = str.c_str();

  do {
    const char* begin = s;

    while(*s != 0) {
      const char* d = delim.c_str();
      bool delimFound = false;

      while(*d != 0) {
        if(*s == *d++) {
          delimFound = true;
          break;
        }
      }

      if(delimFound) {
        break;
      }
      else {
        s++;
      }
    }

    std::string token = std::string(begin, s);
    if(!token.empty()) {
      result.Add(token);
    }
  }
  while(*s++ != 0);

  return result;
}

std::string Asset::GetExtension(const std::string& uri) {
  Stack<std::string> tokens = SplitString(uri, ".");
  size_t tokenCount = tokens.GetCount();
  if(tokenCount > 0) {
    std::string result = tokens[tokenCount - 1];
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
      return std::tolower(c);
    });
    return result;
  }
  else {
    return std::string();
  }
}

void Asset::IncLoading() {
  PxRequireMainThread;

  IncRef();
  loadingCount++;
}

void Asset::DecLoading() {
  PxRequireMainThread;

  if(loadingCount > 0) {
    loadingCount--;

    if(loadingCount == 0) {
      if(loadQueuedId != PrimeNotFound) {
        size_t id = loadQueuedId;
        loadQueuedId = PrimeNotFound;
        Load(id);
      }
    }
  }
  else {
    PrimeAssert(false, "Decremented too many load items.");
  }

  DecRef();
}
