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

#include <Prime/Graphics/ArrayBuffer.h>
#include <Prime/Graphics/IndexBuffer.h>
#include <Prime/Graphics/Tex.h>
#include <Prime/Types/Mat44.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class ModelContentMesh {
friend class Model;
friend class ModelContent;
friend class ModelContentScene;
private:

  size_t textureIndex;
  Tex* directTex;

  std::string name;
  size_t meshIndex;

  void* vertices;
  void* indices;

  Vec3 vertexMin;
  Vec3 vertexMax;

  ArrayBuffer* ab;
  IndexBuffer* ib;

  size_t vertexCount;
  size_t indexCount;

  Mat44 baseTransform;

  bool anim;

public:
  
  size_t GetTextureIndex() const {return textureIndex;}
  Tex* GetDirectTex() const {return directTex;}

  const std::string& GetName() const {return name;}
  size_t GetMeshIndex() const {return meshIndex;}

  const Vec3& GetVertexMin() const {return vertexMin;}
  const Vec3& GetVertexMax() const {return vertexMax;}

  const Mat44& GetBaseTransform() const {return baseTransform;}

  bool GetAnim() const {return anim;}

public:

  ModelContentMesh();
  ~ModelContentMesh();

public:

  void ReferenceTexture(const std::string& path);
  void ReleaseTexture();

  void SetDirectTex(Tex* directTex);

  f32 GetVertexElement(size_t index, size_t vertexSize, size_t elementOffset = 0) const;
  void GetVertexElement2(Vec2& v, size_t index, size_t vertexSize, size_t elementOffset = 0) const;
  void GetVertexElement3(Vec3& v, size_t index, size_t vertexSize, size_t elementOffset = 0) const;
  void GetVertexElement4(Vec4& v, size_t index, size_t vertexSize, size_t elementOffset = 0) const;
  size_t GetIndex(size_t index) const;
  void GetIndices(size_t index, size_t& index1, size_t& index2, size_t& index3) const;

};

};
