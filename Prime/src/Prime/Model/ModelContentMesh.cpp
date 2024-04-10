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

#include <Prime/Model/ModelContentMesh.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ModelContentMesh::ModelContentMesh():
textureIndex(PrimeNotFound),
directTex(nullptr),
meshIndex(0),
vertices(nullptr),
indices(nullptr),
ab(nullptr),
ib(nullptr),
vertexCount(0),
indexCount(0),
anim(false),
vertexMin(Vec3(0.0f, 0.0f, 0.0f)),
vertexMax(Vec3(0.0f, 0.0f, 0.0f)) {


}

ModelContentMesh::~ModelContentMesh() {
  PrimeSafeFree(vertices);
  PrimeSafeFree(indices);

  PrimeSafeDelete(ib);
  PrimeSafeDelete(ab);
}

void ModelContentMesh::ReferenceTexture(const std::string& path) {

}

void ModelContentMesh::ReleaseTexture() {

}

void ModelContentMesh::SetDirectTex(Tex* directTex) {
  this->directTex = directTex;
}

f32 ModelContentMesh::GetVertexElement(size_t index, size_t vertexSize, size_t elementOffset) const {
  u8* vertexPtr = (u8*) vertices;
  return *(f32*) &vertexPtr[index * vertexSize + elementOffset];
}

void ModelContentMesh::GetVertexElement2(Vec2& v, size_t index, size_t vertexSize, size_t elementOffset) const {
  u8* vertexPtr = (u8*) vertices;
  f32* vp = (f32*) &vertexPtr[index * vertexSize + elementOffset];
  v.x = *vp++;
  v.y = *vp;
}

void ModelContentMesh::GetVertexElement3(Vec3& v, size_t index, size_t vertexSize, size_t elementOffset) const {
  u8* vertexPtr = (u8*) vertices;
  f32* vp = (f32*) &vertexPtr[index * vertexSize + elementOffset];
  v.x = *vp++;
  v.y = *vp++;
  v.z = *vp;
}

void ModelContentMesh::GetVertexElement4(Vec4& v, size_t index, size_t vertexSize, size_t elementOffset) const {
  u8* vertexPtr = (u8*) vertices;
  f32* vp = (f32*) &vertexPtr[index * vertexSize + elementOffset];
  v.x = *vp++;
  v.y = *vp++;
  v.z = *vp++;
  v.w = *vp;
}

size_t ModelContentMesh::GetIndex(size_t index) const {
  u16* indices16 = (u16*) indices;
  return indices16[index];
}

void ModelContentMesh::GetIndices(size_t index, size_t& index1, size_t& index2, size_t& index3) const {
  u16* indices16 = (u16*) indices;
  index1 = indices16[index++];
  index2 = indices16[index++];
  index3 = indices16[index++];
}
