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

#include <Prime/Skinset/SkinsetContent.h>
#include <Prime/Skeleton/SkeletonContent.h>
#include <Prime/Imagemap/Imagemap.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Imagemap;
class Skeleton;

class SkinsetPiece {
public:

  refptr<Imagemap> imagemap;
  refptr<Skeleton> skeleton;
  size_t boneIndex;
  size_t parentBoneIndex;

public:

  SkinsetPiece(): boneIndex(PrimeNotFound), parentBoneIndex(PrimeNotFound) {}
  ~SkinsetPiece() {}

};

class SkinsetPieces: public RefObject {
private:

  SkinsetPiece* pieces;
  size_t count;

public:

  size_t GetCount() const {return count;}

public:

  SkinsetPieces(size_t count): pieces(nullptr), count(count) {
    if(count > 0) {
      pieces = new SkinsetPiece[count];
      if(!pieces) {
        count = 0;
      }
    }
  }

  ~SkinsetPieces() {
    PrimeSafeDeleteArray(pieces);
  }

public:

  SkinsetPiece* operator[](size_t index) const {
    if(index < count) {
      return &pieces[index];
    }
    else {
      return nullptr;
    }
  }

};

class Skinset: public RefObject {
private:

  refptr<SkinsetContent> content;
  refptr<SkinsetPieces> pieces;

public:

  refptr<SkinsetContent> GetSkinsetContent() const {return content;}
  bool HasContent() const {return (bool) content;}

public:

  Skinset();
  ~Skinset();

public:

  virtual void SetContent(Content* content);
  virtual void SetContent(SkinsetContent* content);

  virtual void Calc(f32 dt);

  refptr<SkinsetPieces> GetPieces() const;
  size_t GetPieceCount() const;

  virtual void SetAction(const std::string& action, const SkeletonContentActionKeyFrame* actionKeyFrame, bool setIfNew);
  virtual void SetPieceAction(size_t index, const std::string& action, bool setIfNew = true, f32 setTime = -1.0f);

  virtual size_t GetPieceBoneIndex(size_t index);
  virtual void SetPieceBoneIndex(size_t index, size_t boneIndex);
  virtual size_t GetPieceParentBoneIndex(size_t index);
  virtual void SetPieceParentBoneIndex(size_t index, size_t boneIndex);

  virtual size_t GetTreeBoneCount() const;
  virtual size_t GetTreePieceCount() const;

};

};
