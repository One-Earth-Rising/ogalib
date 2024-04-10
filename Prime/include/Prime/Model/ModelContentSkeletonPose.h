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

#include <Prime/Model/ModelContentSkeletonPoseBone.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class ModelContentSkeletonPose {
friend class ModelContentSkeleton;
private:

  std::string name;

  ModelContentSkeletonPoseBone* poseBones;
  size_t poseBoneCount;

public:

  const std::string& GetName() const {return name;}

  const ModelContentSkeletonPoseBone& GetPoseBone(size_t index) const {PrimeAssert(index < poseBoneCount, "Invalid pose bone index."); return poseBones[index];}
  size_t GetPoseBoneCount() const {return poseBoneCount;}

public:

  ModelContentSkeletonPose();
  ModelContentSkeletonPose(const ModelContentSkeletonPose& other);
  ~ModelContentSkeletonPose();

  ModelContentSkeletonPose& operator=(const ModelContentSkeletonPose& other);

  bool operator==(const ModelContentSkeletonPose& other) const;

protected:

  void DestroyPoseBones();

};

};
