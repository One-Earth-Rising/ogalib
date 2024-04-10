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

#include <Prime/Model/ModelContentSkeletonPoseBone.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ModelContentSkeletonPoseBone::ModelContentSkeletonPoseBone():
translation(Vec3(0.0f, 0.0f, 0.0f)),
rotation(Quat(0.0f, 0.0f, 0.0f, 1.0f)),
scaling(Vec3(1.0f, 1.0f, 1.0f)),
boneIndex(PrimeNotFound),
translationKnown(false),
scalingKnown(false),
rotationKnown(false) {

}

ModelContentSkeletonPoseBone::ModelContentSkeletonPoseBone(const ModelContentSkeletonPoseBone& other) {
  (void) operator=(other);
}

ModelContentSkeletonPoseBone& ModelContentSkeletonPoseBone::operator=(const ModelContentSkeletonPoseBone& other) {
  translation = other.translation;
  scaling = other.scaling;
  rotation = other.rotation;
  boneIndex = other.boneIndex;
  translationKnown = other.translationKnown;
  scalingKnown = other.scalingKnown;
  rotationKnown = other.rotationKnown;

  return *this;
}

bool ModelContentSkeletonPoseBone::operator==(const ModelContentSkeletonPoseBone& other) const {
  return translation == other.translation
    && scaling == other.scaling
    && rotation == other.rotation
    && boneIndex == other.boneIndex
    && translationKnown == other.rotationKnown
    && scalingKnown == other.translationKnown
    && rotationKnown == other.rotationKnown;
}
