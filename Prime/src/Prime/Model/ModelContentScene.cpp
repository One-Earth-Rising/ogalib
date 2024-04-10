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

#include <Prime/Model/ModelContentScene.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Model/ModelContent.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <png/png.h>
#include <png/pngstruct.h>
#include <png/pnginfo.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#define TINYGLTF_USE_RAPIDJSON
#define TINYGLTF_NO_FS
#include <tinygltf/tiny_gltf.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define MODEL_MESH_VERTEX_MAX_BONE_WEIGHT_COUNT 16

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

typedef struct _ModelMeshVertex {
  f32 x, y, z;
  f32 u, v;
  f32 nx, ny, nz;
} ModelMeshVertex;

typedef struct _ModelMeshAnimVertex {
  f32 x, y, z;
  f32 u, v, boneCount;
  f32 nx, ny, nz;
  f32 boneIndex[MODEL_MESH_VERTEX_MAX_BONE_WEIGHT_COUNT];
  f32 boneWeight[MODEL_MESH_VERTEX_MAX_BONE_WEIGHT_COUNT];
} ModelMeshAnimVertex;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

static const aiNode* FindSceneNodeByName(const aiNode* node, const aiString& name);
static const aiNode* FindSceneNodeByMeshIndex(const aiNode* node, size_t meshIndex);

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ModelContentScene::ModelContentScene():
meshes(nullptr),
meshCount(0),
skeletons(nullptr),
skeletonCount(0),
animations(nullptr),
animationCount(0),
loadTextures(true),
vertexMin(Vec3(0.0f, 0.0f, 0.0f)),
vertexMax(Vec3(0.0f, 0.0f, 0.0f)) {

}

ModelContentScene::~ModelContentScene() {
  DestroyTextures();
  DestroySkeletons();
  DestroyAnimations();
  DestroyMeshes();
}

void ModelContentScene::SetLoadTextures(bool loadTextures) {
  this->loadTextures = loadTextures;
}

size_t ModelContentScene::GetMeshIndexByName(const std::string& name) const {
  for(size_t i = 0; i < meshCount; i++) {
    const ModelContentMesh& mesh = meshes[i];
    if(mesh.GetName() == name) {
      return i;
    }
  }

  return PrimeNotFound;
}

void ModelContentScene::DestroyMeshes() {
  if(meshCount > 0) {
    for(size_t i = 0; i < meshCount; i++) {
      ModelContentMesh& mesh = meshes[i];
      mesh.ReleaseTexture();
    }
  }

  PrimeSafeDeleteArray(meshes);
  meshCount = 0;
}

void ModelContentScene::DestroySkeletons() {
  PrimeSafeDeleteArray(skeletons);
  skeletonCount = 0;
}

void ModelContentScene::DestroyAnimations() {
  PrimeSafeDeleteArray(animations);
  animationCount = 0;
}

void ModelContentScene::DestroyTextures() {
  textures.Clear();
}

void ModelContentScene::ReadModelUsingTinyGLTF(const void* data, size_t dataSize) {
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  bool res = loader.LoadASCIIFromString(&model, &err, &warn, (const char*) data, (unsigned int) dataSize, "");
  if(!res) {
    res = loader.LoadBinaryFromMemory(&model, &err, &warn, (const u8*) data, (unsigned int) dataSize);
  }
  if(!res) {
    dbgprintf("[Error] Error loading model using TinyGLTF: %s\n", err.c_str());
    return;
  }

  if(!warn.empty()) {
    dbgprintf("[Warning] Problem loading model using TinyGLTF: %s\n", warn.c_str());
  }

  size_t modelAnimationCount = model.animations.size();
  if(modelAnimationCount > 0) {
    skeletonCount = 1;
    skeletons = new ModelContentSkeleton[skeletonCount];
    ModelContentSkeleton& skeleton = skeletons[0];
    skeleton.Load(model);

    animationCount = modelAnimationCount;
    if(animationCount) {
      animations = new ModelContentAnimation[animationCount];
      for(size_t i = 0; i < modelAnimationCount; i++) {
        const tinygltf::Animation& dataAnimation = model.animations[i];
        ModelContentAnimation& animation = animations[i];
        animation.name = dataAnimation.name;
      }
    }
  }

  vertexMin = Vec3(std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max());
  vertexMax = Vec3(std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest());

  meshCount = model.meshes.size();
  if(meshCount) {
    meshes = new ModelContentMesh[meshCount];

    for(size_t i = 0; i < meshCount; i++) {
      const tinygltf::Mesh& dataMesh = model.meshes[i];
      f32* positions = nullptr;
      size_t positionsCount = 0;
      size_t positionByteStride = 0;
      f32* texCoords0 = nullptr;
      size_t texCoords0Count = 0;
      size_t texCoords0ByteStride = 0;
      f32* texCoords1 = nullptr;
      size_t texCoords1Count = 0;
      size_t texCoords1ByteStride = 0;
      f32* normals = nullptr;
      size_t normalsCount = 0;
      size_t normalsByteStride = 0;
      u8* joints = nullptr;
      size_t jointCount = 0;
      size_t jointsComponentType = 0;
      size_t jointsByteStride = 0;
      f32* weights = nullptr; 
      size_t weightsCount = 0;
      size_t weightsByteStride = 0;
      u8* joints1 = nullptr;
      size_t joints1Count = 0;
      size_t joints1ComponentType = 0;
      size_t joints1ByteStride = 0;
      f32* weights1 = nullptr; 
      size_t weights1Count = 0;
      size_t weights1ByteStride = 0;
      u8* indices = nullptr;
      size_t indicesCount = 0;
      size_t indicesComponentType = 0;
      size_t indicesByteStride = 0;

      ModelContentMesh& mesh = meshes[i];
      mesh.meshIndex = i;
      mesh.name = dataMesh.name;

      mesh.baseTransform.LoadIdentity();
      mesh.vertexMin = Vec3(std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max());
      mesh.vertexMax = Vec3(std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest());

      for(size_t j = 0; j < dataMesh.primitives.size(); j++) {
        const auto& primitive = dataMesh.primitives[j];

        if(mesh.textureIndex == PrimeNotFound) {
          if(primitive.material != -1) {
            const auto& material = model.materials[primitive.material];
            if(material.pbrMetallicRoughness.baseColorTexture.index != -1) {
              const auto& texture = model.textures[material.pbrMetallicRoughness.baseColorTexture.index];
              if(texture.source != -1) {
                mesh.textureIndex = texture.source;
              }
            }
          }
        }

        if(!positions) {
          auto accessorItem = primitive.attributes.find("POSITION");
          if(accessorItem != primitive.attributes.end()) {
            const auto& accessor = model.accessors[accessorItem->second];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            positionsCount = accessor.count * 3;
            positionByteStride = accessor.ByteStride(bufferView);
            if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
              if(positionsCount > 0) {
                positions = new f32[positionsCount];
                if(positions) {
                  const u16* positionsData = (const u16*) &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                  f32* positionsP = positions;
                  for(size_t k = 0; k < accessor.count; k++) {
                    f32 x = *positionsData++;
                    f32 y = *positionsData++;
                    f32 z = *positionsData++;

                    *positionsP++ = x;
                    *positionsP++ = y;
                    *positionsP++ = z;

                    if(positionByteStride > 6) {
                      for(size_t m = 0; m < (positionByteStride - 6) / sizeof(u16); m++) {
                        positionsData++;
                      }
                    }
                  }
                }
              }
            }
            else {
              PrimeAssert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Unsupported vertex position type.");
              if(positionsCount > 0) {
                positions = new f32[positionsCount];
                if(positions) {
                  memcpy(positions, &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset], bufferView.byteLength);
                }
              }
            }
          }
        }

        if(!texCoords0) {
          auto accessorItem = primitive.attributes.find("TEXCOORD_0");
          if(accessorItem != primitive.attributes.end()) {
            const auto& accessor = model.accessors[accessorItem->second];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            texCoords0Count = accessor.count * 2;
            texCoords0ByteStride = accessor.ByteStride(bufferView);
            if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
              f32 uScale = 1.0f;
              f32 vScale = 1.0f;

              if(primitive.material != -1) {
                const auto& material = model.materials[primitive.material];
                if(material.pbrMetallicRoughness.baseColorTexture.index != -1) {
                  if(material.pbrMetallicRoughness.baseColorTexture.extensions.size() > 0) {
                    auto textureTransformItem = material.pbrMetallicRoughness.baseColorTexture.extensions.find("KHR_texture_transform");
                    if(textureTransformItem != material.pbrMetallicRoughness.baseColorTexture.extensions.end()) {
                      const auto& textureTransform = textureTransformItem->second;
                      const auto& scale = textureTransform.Get("scale");
                      if(scale.IsArray()) {
                        uScale = (f32) scale.Get(0).GetNumberAsDouble();
                        vScale = (f32) scale.Get(1).GetNumberAsDouble();
                      }
                    }
                  }
                }
              }

              if(texCoords0Count > 0) {
                texCoords0 = new f32[texCoords0Count];
                if(texCoords0) {
                  const u16* texCoords0Data = (const u16*) &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];
                  f32* texCoords0P = texCoords0;
                  for(size_t k = 0; k < accessor.count; k++) {
                    f32 u = *texCoords0Data++ / 65535.0f;
                    f32 v = *texCoords0Data++ / 65535.0f;

                    *texCoords0P++ = u * uScale;
                    *texCoords0P++ = v * vScale;

                    if(texCoords0ByteStride > 4) {
                      for(size_t m = 0; m < (texCoords0ByteStride - 4) / sizeof(u16); m++) {
                        texCoords0Data++;
                      }
                    }
                  }
                }
              }
            }
            else {
              PrimeAssert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Unsupported vertex tex-coord component type.");
              if(texCoords0Count > 0) {
                texCoords0 = new f32[texCoords0Count];
                if(texCoords0) {
                  memcpy(texCoords0, &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset], bufferView.byteLength);
                }
              }
            }
          }
        }

        if(!texCoords1) {
          auto accessorItem = primitive.attributes.find("TEXCOORD_1");
          if(accessorItem != primitive.attributes.end()) {
            const auto& accessor = model.accessors[accessorItem->second];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            texCoords1Count = bufferView.byteLength / sizeof(f32);
            texCoords1ByteStride = accessor.ByteStride(bufferView);
            if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
              f32 uScale = 1.0f;
              f32 vScale = 1.0f;

              if(primitive.material != -1) {
                const auto& material = model.materials[primitive.material];
                if(material.pbrMetallicRoughness.baseColorTexture.index != -1) {
                  if(material.pbrMetallicRoughness.baseColorTexture.extensions.size() > 0) {
                    auto textureTransformItem = material.pbrMetallicRoughness.baseColorTexture.extensions.find("KHR_texture_transform");
                    if(textureTransformItem != material.pbrMetallicRoughness.baseColorTexture.extensions.end()) {
                      const auto& textureTransform = textureTransformItem->second;

                      const auto& scale = textureTransform.Get("scale");
                      if(scale.IsArray()) {
                        uScale = (f32) scale.Get(0).GetNumberAsDouble();
                        vScale = (f32) scale.Get(1).GetNumberAsDouble();
                      }
                    }
                  }
                }
              }

              if(texCoords1Count > 0) {
                texCoords1 = new f32[texCoords1Count];
                if(texCoords1) {
                  const u16* texCoords1Data = (const u16*) &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];

                  f32* texCoords1P = texCoords1;
                  for(size_t k = 0; k < accessor.count; k++) {
                    f32 u = *texCoords1Data++ / 65535.0f;
                    f32 v = *texCoords1Data++ / 65535.0f;

                    *texCoords1P++ = u;
                    *texCoords1P++ = v;

                    if(texCoords1ByteStride > 4) {
                      for(size_t m = 0; m < (texCoords1ByteStride - 4) / sizeof(u16); m++) {
                        texCoords1Data++;
                      }
                    }
                  }
                }
              }
            }
            else {
              PrimeAssert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Unsupported vertex tex-coord component type.");
              if(texCoords1Count > 0) {
                texCoords1 = new f32[texCoords1Count];
                if(texCoords1) {
                  memcpy(texCoords1, &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset], bufferView.byteLength);
                }
              }
            }
          }
        }

        if(!normals) {
          auto accessorItem = primitive.attributes.find("NORMAL");
          if(accessorItem != primitive.attributes.end()) {
            const auto& accessor = model.accessors[accessorItem->second];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            normalsCount = accessor.count * 3;
            normalsByteStride = accessor.ByteStride(bufferView);
            if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
              if(normalsCount > 0) {
                normals = new f32[normalsCount];
                if(normals) {
                  const s8* normalsData = (const s8*) &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];

                  f32* normalsP = normals;
                  for(size_t k = 0; k < accessor.count; k++) {
                    f32 x = *normalsData++ / 127.0f;
                    f32 y = *normalsData++ / 127.0f;
                    f32 z = *normalsData++ / 127.0f;

                    *normalsP++ = x;
                    *normalsP++ = y;
                    *normalsP++ = z;

                    if(normalsByteStride > 3) {
                      for(size_t m = 0; m < (normalsByteStride - 3) / sizeof(s8); m++) {
                        normalsData++;
                      }
                    }
                  }
                }
              }
            }
            else if(accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
              if(normalsCount > 0) {
                normals = new f32[normalsCount];
                if(normals) {
                  const s16* normalsData = (const s16*) &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset];

                  f32* normalsP = normals;
                  for(size_t k = 0; k < accessor.count; k++) {
                    f32 x = *normalsData++ / 32767.0f;
                    f32 y = *normalsData++ / 32767.0f;
                    f32 z = *normalsData++ / 32767.0f;

                    *normalsP++ = x;
                    *normalsP++ = y;
                    *normalsP++ = z;

                    if(normalsByteStride > 6) {
                      for(size_t m = 0; m < (normalsByteStride - 6) / sizeof(s16); m++) {
                        normalsData++;
                      }
                    }
                  }
                }
              }
            }
            else {
              PrimeAssert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Unsupported vertex normal component type.");
              if(normalsCount > 0) {
                normals = new f32[normalsCount];
                if(normals) {
                  memcpy(normals, &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset], bufferView.byteLength);
                }
              }
            }
          }
        }

        if(!joints) {
          auto accessorItem = primitive.attributes.find("JOINTS_0");
          if(accessorItem != primitive.attributes.end()) {
            const auto& accessor = model.accessors[accessorItem->second];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            PrimeAssert(bufferView.byteStride == 0, "Unsupported feature.");
            jointsComponentType = accessor.componentType;
            PrimeAssert(jointsComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE || jointsComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT || jointsComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, "Unsupported joint type.");
            jointCount = accessor.count;
            jointsByteStride = accessor.ByteStride(bufferView);
            if(jointCount > 0) {
              joints = new u8[bufferView.byteLength];
              if(joints) {
                memcpy(joints, &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset], bufferView.byteLength);
              }
            }
          }
        }

        if(!joints1) {
          auto accessorItem = primitive.attributes.find("JOINTS_1");
          if(accessorItem != primitive.attributes.end()) {
            const auto& accessor = model.accessors[accessorItem->second];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            PrimeAssert(bufferView.byteStride == 0, "Unsupported feature.");
            joints1ComponentType = accessor.componentType;
            PrimeAssert(joints1ComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE || joints1ComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT || joints1ComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, "Unsupported joint type.");
            joints1Count = accessor.count;
            jointsByteStride = accessor.ByteStride(bufferView);
            if(joints1Count > 0) {
              joints1 = new u8[bufferView.byteLength];
              if(joints1) {
                memcpy(joints1, &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset], bufferView.byteLength);
              }
            }
          }
        }

        if(!weights) {
          auto accessorItem = primitive.attributes.find("WEIGHTS_0");
          if(accessorItem != primitive.attributes.end()) {
            const auto& accessor = model.accessors[accessorItem->second];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            PrimeAssert(bufferView.byteStride == 0, "Unsupported feature.");
            weightsCount = bufferView.byteLength / sizeof(f32);
            weightsByteStride = accessor.ByteStride(bufferView);
            if(weightsCount > 0) {
              weights = new f32[weightsCount];
              if(weights) {
                memcpy(weights, &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset], bufferView.byteLength);
              }
            }
          }
        }

        if(!weights1) {
          auto accessorItem = primitive.attributes.find("WEIGHTS_1");
          if(accessorItem != primitive.attributes.end()) {
            const auto& accessor = model.accessors[accessorItem->second];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            PrimeAssert(bufferView.byteStride == 0, "Unsupported feature.");
            weights1Count = bufferView.byteLength / sizeof(f32);
            weights1ByteStride = accessor.ByteStride(bufferView);
            if(weights1Count > 0) {
              weights1 = new f32[weights1Count];
              if(weights1) {
                memcpy(weights1, &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset], bufferView.byteLength);
              }
            }
          }
        }

        if(!indices) {
          if(primitive.indices != -1) {
            const auto& accessor = model.accessors[primitive.indices];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            PrimeAssert(bufferView.byteStride == 0, "Unsupported feature.");
            indicesComponentType = accessor.componentType;
            PrimeAssert(indicesComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE || indicesComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT || indicesComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, "Unsupported index type.");
            indicesCount = accessor.count;
            indicesByteStride = accessor.ByteStride(bufferView);
            if(indicesCount > 0) {
              indices = new u8[bufferView.byteLength];
              if(indicesCount) {
                memcpy(indices, &model.buffers[bufferView.buffer].data[accessor.byteOffset + bufferView.byteOffset], bufferView.byteLength);
              }
            }
          }
        }
      }

      PrimeAssert(positions && indices, "Expected to find a position and index buffer.");

      size_t meshBoneIndex = PrimeNotFound;
      if(skeletonCount > 0 && (joints == nullptr || weights == nullptr)) {
        ModelContentSkeleton& skeleton = skeletons[0];
        meshBoneIndex = skeleton.FindBoneIndexByTinyGLTFMeshIndex(model, i);
      }

      mesh.anim = skeletonCount > 0;

      if(positions && indices) {
        if(mesh.anim) {
          size_t vertexCount = positionsCount / 3;
          ModelMeshAnimVertex* vertices = (ModelMeshAnimVertex*) calloc(vertexCount, sizeof(ModelMeshAnimVertex));
          ModelMeshAnimVertex* vertex = vertices;

          const f32* vertexPosition = positions;
          const f32* vertexTexCoord0 = texCoords0;
          const f32* vertexTexCoord1 = texCoords1;
          const f32* vertexNormal = normals;
          const f32* vertexWeight = weights;
          const f32* vertexWeight1 = weights1;

          for(size_t j = 0; j < vertexCount; j++, vertex++) {
            static const aiVector3D sceneUVNone(0.0f, 0.0f, 0.0f);
            static const aiVector3D sceneNormalNone(0.0f, 1.0f, 0.0f);

            vertex->x = *vertexPosition++;
            vertex->y = *vertexPosition++;
            vertex->z = *vertexPosition++;
            vertex->u = vertexTexCoord0 ? *vertexTexCoord0++ : 0.0f;
            vertex->v = vertexTexCoord0 ? *vertexTexCoord0++ : 0.0f;
            vertex->boneCount = 0.0f;

            if(vertexNormal) {
              f32 nx = *vertexNormal++;
              f32 ny = *vertexNormal++;
              f32 nz = *vertexNormal++;
              Vec3 normal(nx, ny, nz);
              normal.Normalize();
              vertex->nx = normal.x;
              vertex->ny = normal.y;
              vertex->nz = normal.z;
            }
            else {
              vertex->nx = 0.0f;
              vertex->ny = 0.0f;
              vertex->nz = 0.0f;
            }

            if(vertexWeight) {
              for(size_t k = 0; k < 4; k++) {
                vertex->boneWeight[k] = *vertexWeight++;
              }
              vertex->boneCount = 4;
            }
            else if(skeletonCount > 0) {
              if(meshBoneIndex != PrimeNotFound) {
                vertex->boneIndex[0] = (f32) meshBoneIndex;
                vertex->boneWeight[0] = 1.0f;
                vertex->boneCount = 1.0f;
              }
            }

            if(vertexWeight1) {
              for(size_t k = 4; k < 8; k++) {
                vertex->boneWeight[k] = *vertexWeight1++;
              }
              vertex->boneCount = 8;
            }

            mesh.vertexMin.x = min(mesh.vertexMin.x, vertex->x);
            mesh.vertexMin.y = min(mesh.vertexMin.y, vertex->y);
            mesh.vertexMin.z = min(mesh.vertexMin.z, vertex->z);
            mesh.vertexMax.x = max(mesh.vertexMax.x, vertex->x);
            mesh.vertexMax.y = max(mesh.vertexMax.y, vertex->y);
            mesh.vertexMax.z = max(mesh.vertexMax.z, vertex->z);
          }

          if(skeletonCount > 0) {
            ModelContentSkeleton& skeleton = skeletons[0];

            if(joints) {
              if(jointsComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                ModelMeshAnimVertex* vertex = vertices;
                const u8* vertexJoint = (const u8*) joints;
                for(size_t j = 0; j < vertexCount; j++, vertex++) {
                  for(size_t k = 0; k < 4; k++) {
                    size_t jointIndex = *vertexJoint++;
                    vertex->boneIndex[k] = (f32) skeleton.FindBoneIndexByTinyGLTFJointIndex(model, jointIndex);
                  }
                }
              }
              else if(jointsComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                ModelMeshAnimVertex* vertex = vertices;
                const u16* vertexJoint = (const u16*) joints;
                for(size_t j = 0; j < vertexCount; j++, vertex++) {
                  for(size_t k = 0; k < 4; k++) {
                    size_t jointIndex = *vertexJoint++;
                    vertex->boneIndex[k] = (f32) skeleton.FindBoneIndexByTinyGLTFJointIndex(model, jointIndex);
                  }
                }
              }
              else if(jointsComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                ModelMeshAnimVertex* vertex = vertices;
                const size_t* vertexJoint = (const size_t*) joints;
                for(size_t j = 0; j < vertexCount; j++, vertex++) {
                  for(size_t k = 0; k < 4; k++) {
                    size_t jointIndex = *vertexJoint++;
                    vertex->boneIndex[k] = (f32) skeleton.FindBoneIndexByTinyGLTFJointIndex(model, jointIndex);
                  }
                }
              }
            }

            if(joints1) {
              if(joints1ComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                ModelMeshAnimVertex* vertex = vertices;
                const u8* vertexJoint = (const u8*) joints1;
                for(size_t j = 0; j < vertexCount; j++, vertex++) {
                  for(size_t k = 4; k < 8; k++) {
                    size_t jointIndex = *vertexJoint++;
                    vertex->boneIndex[k] = (f32) skeleton.FindBoneIndexByTinyGLTFJointIndex(model, jointIndex);
                  }
                }
              }
              else if(joints1ComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                ModelMeshAnimVertex* vertex = vertices;
                const u16* vertexJoint = (const u16*) joints1;
                for(size_t j = 0; j < vertexCount; j++, vertex++) {
                  for(size_t k = 4; k < 8; k++) {
                    size_t jointIndex = *vertexJoint++;
                    vertex->boneIndex[k] = (f32) skeleton.FindBoneIndexByTinyGLTFJointIndex(model, jointIndex);
                  }
                }
              }
              else if(joints1ComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                ModelMeshAnimVertex* vertex = vertices;
                const size_t* vertexJoint = (const size_t*) joints1;
                for(size_t j = 0; j < vertexCount; j++, vertex++) {
                  for(size_t k = 4; k < 8; k++) {
                    size_t jointIndex = *vertexJoint++;
                    vertex->boneIndex[k] = (f32) skeleton.FindBoneIndexByTinyGLTFJointIndex(model, jointIndex);
                  }
                }
              }
            }
          }

          void* meshIndices = nullptr;
          IndexFormat indexFormat = IndexFormatNone;

          if(indices) {
            if(indicesComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
              meshIndices = calloc(indicesCount, sizeof(u8));
              u8* meshIndices8 = (u8*) meshIndices;
              const u8* index = (const u8*) indices;
              for(size_t j = 0; j < indicesCount; j++) {
                *meshIndices8++ = *index++;
              }
              indexFormat = IndexFormatSize8;
            }
            else if(indicesComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
              meshIndices = calloc(indicesCount, sizeof(u16));
              u16* meshIndices16 = (u16*) meshIndices;
              const u16* index = (const u16*) indices;
              for(size_t j = 0; j < indicesCount; j++) {
                *meshIndices16++ = *index++;
              }
              indexFormat = IndexFormatSize16;
            }
            else if(indicesComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
              meshIndices = calloc(indicesCount, sizeof(u32));
              u32* meshIndices32 = (u32*) meshIndices;
              const u32* index = (const u32*) indices;
              for(size_t j = 0; j < indicesCount; j++) {
                *meshIndices32++ = *index++;
              }
              indexFormat = IndexFormatSize32;
            }
          }

          mesh.ab = ArrayBuffer::Create(sizeof(ModelMeshAnimVertex), vertices, vertexCount, BufferPrimitiveTriangles);
          mesh.ab->LoadAttribute("vPos", sizeof(f32) * 3);
          mesh.ab->LoadAttribute("vUVBoneCount", sizeof(f32) * 3);
          mesh.ab->LoadAttribute("vNormal", sizeof(f32) * 3);
          mesh.ab->LoadAttribute("vBoneIndex1", sizeof(f32) * 4);
          mesh.ab->LoadAttribute("vBoneIndex2", sizeof(f32) * 4);
          mesh.ab->LoadAttribute("vBoneIndex3", sizeof(f32) * 4);
          mesh.ab->LoadAttribute("vBoneIndex4", sizeof(f32) * 4);
          mesh.ab->LoadAttribute("vBoneWeight1", sizeof(f32) * 4);
          mesh.ab->LoadAttribute("vBoneWeight2", sizeof(f32) * 4);
          mesh.ab->LoadAttribute("vBoneWeight3", sizeof(f32) * 4);
          mesh.ab->LoadAttribute("vBoneWeight4", sizeof(f32) * 4);

          if(meshIndices) {
            mesh.ib = IndexBuffer::Create(indexFormat, meshIndices, indicesCount);

            mesh.indices = meshIndices;
            mesh.indexCount = indicesCount;
          }
          else {
            PrimeAssert(false, "Could not create index buffer.");
            mesh.indices = nullptr;
            mesh.indexCount = 0;
          }

          mesh.vertices = vertices;
          mesh.vertexCount = vertexCount;
        }
        else {
          size_t vertexCount = positionsCount / 3;
          ModelMeshVertex* vertices = (ModelMeshVertex*) calloc(vertexCount, sizeof(ModelMeshVertex));
          ModelMeshVertex* vertex = vertices;

          const f32* vertexPosition = positions;
          const f32* vertexTexCoord0 = texCoords0;
          const f32* vertexTexCoord1 = texCoords1;
          const f32* vertexNormal = normals;

          for(size_t j = 0; j < vertexCount; j++, vertex++) {
            static const aiVector3D sceneUVNone(0.0f, 0.0f, 0.0f);
            static const aiVector3D sceneNormalNone(0.0f, 1.0f, 0.0f);

            vertex->x = *vertexPosition++;
            vertex->y = *vertexPosition++;
            vertex->z = *vertexPosition++;
            vertex->u = vertexTexCoord0 ? *vertexTexCoord0++ : 0.0f;
            vertex->v = vertexTexCoord0 ? *vertexTexCoord0++ : 0.0f;

            if(vertexNormal) {
              f32 nx = *vertexNormal++;
              f32 ny = *vertexNormal++;
              f32 nz = *vertexNormal++;
              Vec3 normal(nx, ny, nz);
              normal.Normalize();
              vertex->nx = normal.x;
              vertex->ny = normal.y;
              vertex->nz = normal.z;
            }
            else {
              vertex->nx = 0.0f;
              vertex->ny = 0.0f;
              vertex->nz = 0.0f;
            }

            mesh.vertexMin.x = min(mesh.vertexMin.x, vertex->x);
            mesh.vertexMin.y = min(mesh.vertexMin.y, vertex->y);
            mesh.vertexMin.z = min(mesh.vertexMin.z, vertex->z);
            mesh.vertexMax.x = max(mesh.vertexMax.x, vertex->x);
            mesh.vertexMax.y = max(mesh.vertexMax.y, vertex->y);
            mesh.vertexMax.z = max(mesh.vertexMax.z, vertex->z);
          }

          void* meshIndices = nullptr;
          IndexFormat indexFormat = IndexFormatNone;

          if(indices) {
            if(indicesComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
              meshIndices = calloc(indicesCount, sizeof(u8));
              u8* meshIndices8 = (u8*) meshIndices;
              const u8* index = (const u8*) indices;
              for(size_t j = 0; j < indicesCount; j++) {
                *meshIndices8++ = *index++;
              }
              indexFormat = IndexFormatSize8;
            }
            else if(indicesComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
              meshIndices = calloc(indicesCount, sizeof(u16));
              u16* meshIndices16 = (u16*) meshIndices;
              const u16* index = (const u16*) indices;
              for(size_t j = 0; j < indicesCount; j++) {
                *meshIndices16++ = *index++;
              }
              indexFormat = IndexFormatSize16;
            }
            else if(indicesComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
              meshIndices = calloc(indicesCount, sizeof(u32));
              u32* meshIndices32 = (u32*) meshIndices;
              const u32* index = (const u32*) indices;
              for(size_t j = 0; j < indicesCount; j++) {
                *meshIndices32++ = *index++;
              }
              indexFormat = IndexFormatSize32;
            }
          }

          mesh.ab = ArrayBuffer::Create(sizeof(ModelMeshVertex), vertices, vertexCount, BufferPrimitiveTriangles);
          mesh.ab->LoadAttribute("vPos", sizeof(f32) * 3);
          mesh.ab->LoadAttribute("vUV", sizeof(f32) * 2);
          mesh.ab->LoadAttribute("vNormal", sizeof(f32) * 3);

          if(meshIndices) {
            mesh.ib = IndexBuffer::Create(indexFormat, meshIndices, indicesCount);

            mesh.indices = meshIndices;
            mesh.indexCount = indicesCount;
          }
          else {
            PrimeAssert(false, "Could not create index buffer.");
            mesh.indices = nullptr;
            mesh.indexCount = 0;
          }

          mesh.vertices = vertices;
          mesh.vertexCount = vertexCount;
        }
      }

      PrimeSafeDeleteArray(indices);
      PrimeSafeDeleteArray(weights);
      PrimeSafeDeleteArray(weights1);
      PrimeSafeDeleteArray(joints);
      PrimeSafeDeleteArray(joints1);
      PrimeSafeDeleteArray(normals);
      PrimeSafeDeleteArray(texCoords1);
      PrimeSafeDeleteArray(texCoords0);
      PrimeSafeDeleteArray(positions);

      vertexMin.x = min(vertexMin.x, mesh.vertexMin.x);
      vertexMin.y = min(vertexMin.y, mesh.vertexMin.y);
      vertexMin.z = min(vertexMin.z, mesh.vertexMin.z);
      vertexMax.x = max(vertexMax.x, mesh.vertexMax.x);
      vertexMax.y = max(vertexMax.y, mesh.vertexMax.y);
      vertexMax.z = max(vertexMax.z, mesh.vertexMax.z);
    }
  }

  if(loadTextures) {
    size_t textureCount = model.textures.size();
    for(size_t i = 0; i < textureCount; i++) {
      const auto& texture = model.textures[i];
      const auto& image = model.images[texture.source];

      if(image.image.size() > 0 && (image.component == 3 || image.component == 4) && (image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE || image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)) {
        std::string subFormat;

        if(image.component == 3) {
          if(image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            subFormat = "R16G16B16_sRGB";
          }
          else if(image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
            subFormat = "R8G8B8_sRGB";
          }
        }
        else if(image.component == 4) {
          if(image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            subFormat = "R16G16B16A16_sRGB";
          }
          else if(image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
            subFormat = "R8G8B8A8_sRGB";
          }
        }

        if(!subFormat.empty()) {
          std::string imageData((char*) &image.image[0], (size_t) image.image.size());

          new Job(nullptr, [=](Job& job) {
            Tex* tex = Tex::Create();

            tex->AddTexData("", imageData, {
              {"format", "raw"},
              {"subFormat", subFormat},
              {"subFormatAsNative", true},
              {"w", image.width},
              {"h", image.height},
              });

            textures.Add(tex);
          });
        }
      }
    }
  }
}

void ModelContentScene::ReadModelUsingAssimp(const void* data, size_t dataSize) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFileFromMemory(data, dataSize, 0);
  if(!scene) {
    scene = importer.ReadFileFromMemory(data, dataSize, 0, "gltf");
  }
  if(!scene) {
    scene = importer.ReadFileFromMemory(data, dataSize, 0, "glb");
  }
  if(!scene) {
    dbgprintf("[Error] Error loading model using Assimp: %s\n", importer.GetErrorString());
    return;
  }

  if(scene->mNumAnimations) {
    skeletonCount = 1;
    skeletons = new ModelContentSkeleton[skeletonCount];
    ModelContentSkeleton& skeleton = skeletons[0];
    skeleton.Load(*scene);

    animationCount = scene->mNumAnimations;
    if(animationCount) {
      animations = new ModelContentAnimation[animationCount];
      for(size_t i = 0; i < scene->mNumAnimations; i++) {
        aiAnimation* sceneAnimation = scene->mAnimations[i];
        ModelContentAnimation& animation = animations[i];
        animation.name = sceneAnimation->mName.data;
      }
    }
  }

  vertexMin = Vec3(std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max());
  vertexMax = Vec3(std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest());

  meshCount = scene->mNumMeshes;
  if(meshCount) {
    meshes = new ModelContentMesh[meshCount];

    for(size_t i = 0; i < meshCount; i++) {
      ModelContentMesh& mesh = meshes[i];
      mesh.meshIndex = i;
      const struct aiMesh* sceneMesh = scene->mMeshes[i];

      mesh.name = sceneMesh->mName.data;

      mesh.baseTransform.LoadIdentity();
      mesh.vertexMin = Vec3(std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max());
      mesh.vertexMax = Vec3(std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest());
      Mat44 measureVertexTransform;
      measureVertexTransform.LoadIdentity();

      size_t vertexCount = sceneMesh->mNumVertices;

      if(sceneMesh->HasBones()) {
        ModelMeshAnimVertex* vertices = (ModelMeshAnimVertex*) calloc(vertexCount, sizeof(ModelMeshAnimVertex));
        ModelMeshAnimVertex* vertex = vertices;

        for(size_t j = 0; j < vertexCount; j++, vertex++) {
          static const aiVector3D sceneUVNone(0.0f, 0.0f, 0.0f);
          static const aiVector3D sceneNormalNone(0.0f, 1.0f, 0.0f);

          const aiVector3D& sceneVertex = sceneMesh->mVertices[j];
          const aiVector3D& sceneUV = sceneMesh->mTextureCoords[0] ? sceneMesh->mTextureCoords[0][j] : sceneUVNone;
          const aiVector3D& sceneNormal = sceneMesh->mNormals ? sceneMesh->mNormals[j] : sceneNormalNone;

          vertex->x = sceneVertex.x;
          vertex->y = sceneVertex.y;
          vertex->z = sceneVertex.z;
          vertex->u = sceneUV.x;
          vertex->v = 1.0f - sceneUV.y;
          vertex->boneCount = 0.0f;

          Vec3 normal(sceneNormal.x, sceneNormal.y, sceneNormal.z);
          normal.Normalize();
          vertex->nx = normal.x;
          vertex->ny = normal.y;
          vertex->nz = normal.z;

          mesh.vertexMin.x = min(mesh.vertexMin.x, vertex->x);
          mesh.vertexMin.y = min(mesh.vertexMin.y, vertex->y);
          mesh.vertexMin.z = min(mesh.vertexMin.z, vertex->z);
          mesh.vertexMax.x = max(mesh.vertexMax.x, vertex->x);
          mesh.vertexMax.y = max(mesh.vertexMax.y, vertex->y);
          mesh.vertexMax.z = max(mesh.vertexMax.z, vertex->z);
        }

        size_t indexCount = 0;

        for(size_t j = 0; j < sceneMesh->mNumFaces; j++) {
          const struct aiFace* sceneFace = &sceneMesh->mFaces[j];
          if(sceneFace->mNumIndices == 3) {
            indexCount += 3;
          }
          else if(sceneFace->mNumIndices == 4) {
            indexCount += 6;
          }
          else if(sceneFace->mNumIndices >= 5) {
            indexCount += (sceneFace->mNumIndices - 2) * 3;
          }
        }

        void* indices = nullptr;
        IndexFormat indexFormat = IndexFormatNone;

        if(indexCount < 0x100) {
          u8* indices8 = (u8*) calloc(indexCount, sizeof(u8));
          u8* index = indices8;
          indices = indices8;
          indexFormat = IndexFormatSize8;

          for(size_t j = 0; j < sceneMesh->mNumFaces; j++) {
            const struct aiFace* sceneFace = &sceneMesh->mFaces[j];
            if(sceneFace->mNumIndices == 3) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];
            }
            else if(sceneFace->mNumIndices == 4) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[2];
              *index++ = sceneFace->mIndices[3];
            }
            else if(sceneFace->mNumIndices >= 5) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              size_t triangleCount = sceneFace->mNumIndices - 2;
              for(size_t i = 1; i < triangleCount; i++) {
                *index++ = sceneFace->mIndices[0];
                *index++ = sceneFace->mIndices[i + 1];
                *index++ = sceneFace->mIndices[i + 2];
              }
            }
          }
        }
        else if(indexCount < 0x10000) {
          u16* indices16 = (u16*) calloc(indexCount, sizeof(u16));
          u16* index = indices16;
          indices = indices16;
          indexFormat = IndexFormatSize16;

          for(size_t j = 0; j < sceneMesh->mNumFaces; j++) {
            const struct aiFace* sceneFace = &sceneMesh->mFaces[j];
            if(sceneFace->mNumIndices == 3) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];
            }
            else if(sceneFace->mNumIndices == 4) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[2];
              *index++ = sceneFace->mIndices[3];
            }
            else if(sceneFace->mNumIndices >= 5) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              size_t triangleCount = sceneFace->mNumIndices - 2;
              for(size_t i = 1; i < triangleCount; i++) {
                *index++ = sceneFace->mIndices[0];
                *index++ = sceneFace->mIndices[i + 1];
                *index++ = sceneFace->mIndices[i + 2];
              }
            }
          }
        }
        else {
          u32* indices32 = (u32*) calloc(indexCount, sizeof(u32));
          u32* index = indices32;
          indices = indices32;
          indexFormat = IndexFormatSize32;

          for(size_t j = 0; j < sceneMesh->mNumFaces; j++) {
            const struct aiFace* sceneFace = &sceneMesh->mFaces[j];
            if(sceneFace->mNumIndices == 3) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];
            }
            else if(sceneFace->mNumIndices == 4) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[2];
              *index++ = sceneFace->mIndices[3];
            }
            else if(sceneFace->mNumIndices >= 5) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              size_t triangleCount = sceneFace->mNumIndices - 2;
              for(size_t i = 1; i < triangleCount; i++) {
                *index++ = sceneFace->mIndices[0];
                *index++ = sceneFace->mIndices[i + 1];
                *index++ = sceneFace->mIndices[i + 2];
              }
            }
          }
        }

        ModelContentSkeleton& skeleton = skeletons[0];
        for(size_t j = 0; j < sceneMesh->mNumBones; j++) {
          const struct aiBone* sceneBone = sceneMesh->mBones[j];
          std::string boneName = sceneBone->mName.C_Str();
          size_t actionPoseBoneIndex = PrimeNotFound;

          for(size_t k = 0; k < sceneBone->mNumWeights; k++) {
            const struct aiVertexWeight& sceneVertexWeight = sceneBone->mWeights[k];
            if(sceneVertexWeight.mVertexId < vertexCount) {
              ModelMeshAnimVertex& vertex = vertices[sceneVertexWeight.mVertexId];
              if(vertex.boneCount < MODEL_MESH_VERTEX_MAX_BONE_WEIGHT_COUNT) {
                size_t boneWeightNumber = (size_t) roundf(vertex.boneCount);

                if(actionPoseBoneIndex == PrimeNotFound) {
                  skeleton.ApplyBoneAffectingVertices(boneName);
                  actionPoseBoneIndex = skeleton.GetActionPoseBoneIndexByName(boneName);
                  PrimeAssert(actionPoseBoneIndex != PrimeNotFound, "Failed to apply a bone to affect vertices: name = %s", boneName.c_str());
                }

                vertex.boneIndex[boneWeightNumber] = (f32) actionPoseBoneIndex;
                vertex.boneWeight[boneWeightNumber] = sceneVertexWeight.mWeight;
                vertex.boneCount += 1.0f;
              }
              else {
                PrimeAssert(false, "Vertex %d is affected by too many bones, max is %d, found %zu", sceneVertexWeight.mVertexId, MODEL_MESH_VERTEX_MAX_BONE_WEIGHT_COUNT, (size_t) vertex.boneCount);
              }
            }
            else {
              PrimeAssert(false, "Invalid vertex id: %d", sceneVertexWeight.mVertexId);
            }
          }
        }

        const aiNode* node = FindSceneNodeByName(scene->mRootNode, sceneMesh->mName);
        if(!node) {
          node = FindSceneNodeByMeshIndex(scene->mRootNode, i);
        }
        while(node) {
          Mat44 nodeTransformation;
          CopyMatrix(nodeTransformation, node->mTransformation);
          measureVertexTransform = nodeTransformation * measureVertexTransform;
          node = node->mParent;
        }

        mesh.ab = ArrayBuffer::Create(sizeof(ModelMeshAnimVertex), vertices, vertexCount, BufferPrimitiveTriangles);
        mesh.ab->LoadAttribute("vPos", sizeof(f32) * 3);
        mesh.ab->LoadAttribute("vUVBoneCount", sizeof(f32) * 3);
        mesh.ab->LoadAttribute("vNormal", sizeof(f32) * 3);
        mesh.ab->LoadAttribute("vBoneIndex1", sizeof(f32) * 4);
        mesh.ab->LoadAttribute("vBoneIndex2", sizeof(f32) * 4);
        mesh.ab->LoadAttribute("vBoneIndex3", sizeof(f32) * 4);
        mesh.ab->LoadAttribute("vBoneIndex4", sizeof(f32) * 4);
        mesh.ab->LoadAttribute("vBoneWeight1", sizeof(f32) * 4);
        mesh.ab->LoadAttribute("vBoneWeight2", sizeof(f32) * 4);
        mesh.ab->LoadAttribute("vBoneWeight3", sizeof(f32) * 4);
        mesh.ab->LoadAttribute("vBoneWeight4", sizeof(f32) * 4);

        mesh.ib = IndexBuffer::Create(indexFormat, indices, indexCount);

        mesh.vertices = vertices;
        mesh.vertexCount = vertexCount;

        mesh.indices = indices;
        mesh.indexCount = indexCount;

        mesh.anim = true;
      }
      else {
        size_t vertexCount = sceneMesh->mNumVertices;
        ModelMeshVertex* vertices = (ModelMeshVertex*) calloc(vertexCount, sizeof(ModelMeshVertex));
        ModelMeshVertex* vertex = vertices;

        for(size_t j = 0; j < vertexCount; j++, vertex++) {
          static const aiVector3D sceneUVNone(0.0f, 0.0f, 0.0f);
          static const aiVector3D sceneNormalNone(0.0f, 1.0f, 0.0f);

          const aiVector3D& sceneVertex = sceneMesh->mVertices[j];
          const aiVector3D& sceneUV = sceneMesh->mTextureCoords[0] ? sceneMesh->mTextureCoords[0][j] : sceneUVNone;
          const aiVector3D& sceneNormal = sceneMesh->mNormals ? sceneMesh->mNormals[j] : sceneNormalNone;

          vertex->x = sceneVertex.x;
          vertex->y = sceneVertex.y;
          vertex->z = sceneVertex.z;
          vertex->u = sceneUV.x;
          vertex->v = 1.0f - sceneUV.y;

          Vec3 normal(sceneNormal.x, sceneNormal.y, sceneNormal.z);
          normal.Normalize();
          vertex->nx = normal.x;
          vertex->ny = normal.y;
          vertex->nz = normal.z;

          mesh.vertexMin.x = min(mesh.vertexMin.x, vertex->x);
          mesh.vertexMin.y = min(mesh.vertexMin.y, vertex->y);
          mesh.vertexMin.z = min(mesh.vertexMin.z, vertex->z);
          mesh.vertexMax.x = max(mesh.vertexMax.x, vertex->x);
          mesh.vertexMax.y = max(mesh.vertexMax.y, vertex->y);
          mesh.vertexMax.z = max(mesh.vertexMax.z, vertex->z);
        }

        size_t indexCount = 0;

        for(size_t j = 0; j < sceneMesh->mNumFaces; j++) {
          const struct aiFace* sceneFace = &sceneMesh->mFaces[j];
          if(sceneFace->mNumIndices == 3) {
            indexCount += 3;
          }
          else if(sceneFace->mNumIndices == 4) {
            indexCount += 6;
          }
          else if(sceneFace->mNumIndices >= 5) {
            indexCount += (sceneFace->mNumIndices - 2) * 3;
          }
        }

        void* indices = nullptr;
        IndexFormat indexFormat = IndexFormatNone;

        if(indexCount < 0x100) {
          u8* indices8 = (u8*) calloc(indexCount, sizeof(u8));
          u8* index = indices8;
          indices = indices8;
          indexFormat = IndexFormatSize8;

          for(size_t j = 0; j < sceneMesh->mNumFaces; j++) {
            const struct aiFace* sceneFace = &sceneMesh->mFaces[j];
            if(sceneFace->mNumIndices == 3) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];
            }
            else if(sceneFace->mNumIndices == 4) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[2];
              *index++ = sceneFace->mIndices[3];
            }
            else if(sceneFace->mNumIndices >= 5) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              size_t triangleCount = sceneFace->mNumIndices - 2;
              for(size_t i = 1; i < triangleCount; i++) {
                *index++ = sceneFace->mIndices[0];
                *index++ = sceneFace->mIndices[i + 1];
                *index++ = sceneFace->mIndices[i + 2];
              }
            }
          }
        }
        else if(indexCount < 0x10000) {
          u16* indices16 = (u16*) calloc(indexCount, sizeof(u16));
          u16* index = indices16;
          indices = indices16;
          indexFormat = IndexFormatSize16;

          for(size_t j = 0; j < sceneMesh->mNumFaces; j++) {
            const struct aiFace* sceneFace = &sceneMesh->mFaces[j];
            if(sceneFace->mNumIndices == 3) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];
            }
            else if(sceneFace->mNumIndices == 4) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[2];
              *index++ = sceneFace->mIndices[3];
            }
            else if(sceneFace->mNumIndices >= 5) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              size_t triangleCount = sceneFace->mNumIndices - 2;
              for(size_t i = 1; i < triangleCount; i++) {
                *index++ = sceneFace->mIndices[0];
                *index++ = sceneFace->mIndices[i + 1];
                *index++ = sceneFace->mIndices[i + 2];
              }
            }
          }
        }
        else {
          u32* indices32 = (u32*) calloc(indexCount, sizeof(u32));
          u32* index = indices32;
          indices = indices32;
          indexFormat = IndexFormatSize32;

          for(size_t j = 0; j < sceneMesh->mNumFaces; j++) {
            const struct aiFace* sceneFace = &sceneMesh->mFaces[j];
            if(sceneFace->mNumIndices == 3) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];
            }
            else if(sceneFace->mNumIndices == 4) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[2];
              *index++ = sceneFace->mIndices[3];
            }
            else if(sceneFace->mNumIndices >= 5) {
              *index++ = sceneFace->mIndices[0];
              *index++ = sceneFace->mIndices[1];
              *index++ = sceneFace->mIndices[2];

              size_t triangleCount = sceneFace->mNumIndices - 2;
              for(size_t i = 1; i < triangleCount; i++) {
                *index++ = sceneFace->mIndices[0];
                *index++ = sceneFace->mIndices[i + 1];
                *index++ = sceneFace->mIndices[i + 2];
              }
            }
          }
        }

        const aiNode* node = FindSceneNodeByName(scene->mRootNode, sceneMesh->mName);
        if(!node) {
          node = FindSceneNodeByMeshIndex(scene->mRootNode, i);
        }
        while(node) {
          Mat44 nodeTransformation;
          CopyMatrix(nodeTransformation, node->mTransformation);
          mesh.baseTransform = nodeTransformation * mesh.baseTransform;
          node = node->mParent;
        }

        mesh.ab = ArrayBuffer::Create(sizeof(ModelMeshVertex), vertices, vertexCount, BufferPrimitiveTriangles);
        mesh.ab->LoadAttribute("vPos", sizeof(f32) * 3);
        mesh.ab->LoadAttribute("vUV", sizeof(f32) * 2);
        mesh.ab->LoadAttribute("vNormal", sizeof(f32) * 3);

        mesh.ib = IndexBuffer::Create(indexFormat, indices, indexCount);

        mesh.vertices = vertices;
        mesh.vertexCount = vertexCount;

        mesh.indices = indices;
        mesh.indexCount = indexCount;
      }

      measureVertexTransform = measureVertexTransform.Multiply(mesh.baseTransform);

      mesh.vertexMin = measureVertexTransform * mesh.vertexMin;
      mesh.vertexMax = measureVertexTransform * mesh.vertexMax;

      vertexMin.x = min(vertexMin.x, mesh.vertexMin.x);
      vertexMin.y = min(vertexMin.y, mesh.vertexMin.y);
      vertexMin.z = min(vertexMin.z, mesh.vertexMin.z);
      vertexMax.x = max(vertexMax.x, mesh.vertexMax.x);
      vertexMax.y = max(vertexMax.y, mesh.vertexMax.y);
      vertexMax.z = max(vertexMax.z, mesh.vertexMax.z);

      if(vertexMin.x > vertexMax.x) {
        std::swap(vertexMin.x, vertexMax.x);
      }
      if(vertexMin.y > vertexMax.y) {
        std::swap(vertexMin.y, vertexMax.y);
      }
      if(vertexMin.z > vertexMax.z) {
        std::swap(vertexMin.z, vertexMax.z);
      }
    }
  }

  if(loadTextures && scene->mNumTextures) {
    for(size_t i = 0; i < scene->mNumTextures; i++) {
      aiTexture* texture = scene->mTextures[i];
      std::string formatHint(texture->achFormatHint);

      if(formatHint == "png" && png_sig_cmp((png_bytep) texture->pcData, 0, 8) == 0) {
#pragma message("todo")
        //Tex* tex = Tex::Create(texture->pcData, texture->mWidth);
        //textures.push_back(tex);
      }
    }
  }
}

const aiNode* FindSceneNodeByName(const aiNode* node, const aiString& name) {
  if(node->mName == name) {
    return node;
  }
  else {
    for(size_t i = 0; i < node->mNumChildren; i++) {
      const aiNode* child = node->mChildren[i];
      const aiNode* result = FindSceneNodeByName(child, name);
      if(result) {
        return result;
      }
    }
  }

  return nullptr;
}
const aiNode* FindSceneNodeByMeshIndex(const aiNode* node, size_t meshIndex) {
  for(size_t i = 0; i < node->mNumMeshes; i++) {
    if(node->mMeshes[i] == meshIndex) {
      return node;
    }
  }

  for(size_t i = 0; i < node->mNumChildren; i++) {
    const aiNode* child = node->mChildren[i];
    const aiNode* result = FindSceneNodeByMeshIndex(child, meshIndex);
    if(result) {
      return result;
    }
  }

  return nullptr;
}
