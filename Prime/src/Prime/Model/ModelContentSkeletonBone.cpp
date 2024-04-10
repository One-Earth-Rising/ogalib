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

#include <Prime/Model/ModelContentSkeletonBone.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ModelContentSkeletonBone::ModelContentSkeletonBone():
actionPoseBoneIndex(PrimeNotFound),
childBoneIndices(nullptr),
childBoneIndexCount(0),
meshTransformations(nullptr),
meshTransformationsValid(nullptr),
meshTransformationCount(0) {
  transformation.LoadIdentity();
}

ModelContentSkeletonBone::~ModelContentSkeletonBone() {
  DestroyMeshTransformations();
  DestroyChildBoneIndices();
}

bool ModelContentSkeletonBone::IsMeshTransformationValid(size_t index) const {
  if(meshTransformations == nullptr || index >= meshTransformationCount) {
    return false;
  }
  else {
    return meshTransformationsValid[index];
  }
}

void ModelContentSkeletonBone::DestroyChildBoneIndices() {
  PrimeSafeDeleteArray(childBoneIndices);
  childBoneIndexCount = 0;
}

void ModelContentSkeletonBone::DestroyMeshTransformations() {
  PrimeSafeDeleteArray(meshTransformations);
  PrimeSafeFree(meshTransformationsValid);
  meshTransformationCount = 0;
}
