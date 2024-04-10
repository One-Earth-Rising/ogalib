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

#include <Prime/Content/Content.h>
#include <Prime/Types/Mat44.h>
#include <Prime/Enum/SkinsetAffixType.h>

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

struct _SkeletonContentActionKeyFrame;

typedef struct _SkinsetContentPiece {
  std::string name;
  std::string content;
  std::string action;
  std::string skin;
  std::string affix;
  SkinsetAffixType affixType;
  f32 affixX;
  f32 affixY;
  f32 baseAngle;
  f32 baseScaleX;
  f32 baseScaleY;
  Mat44 baseTransform;

  _SkinsetContentPiece():
    affixType(SkinsetAffixType()),
    affixX(0.0f),
    affixY(0.0f),
    baseAngle(0.0f),
    baseScaleX(0.0f),
    baseScaleY(0.0f) {

  }
} SkinsetContentPiece;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef Stack<size_t> SkinsetContentAffixPieceLookupStack;

class SkinsetContent: public Content {
private:

  SkinsetContentPiece* pieces;
  size_t pieceCount;

public:

  const SkinsetContentPiece& GetPiece(size_t index) const {PrimeAssert(index < pieceCount, "Invalid piece index."); return pieces[index];}
  const SkinsetContentPiece* GetPieces() const {return pieces;}
  size_t GetPieceCount() const {return pieceCount;}

public:

  SkinsetContent();
  ~SkinsetContent();

public:

  bool Load(const json& data, const json& info) override;

  void GetWalkReferences(Stack<std::string>& paths) const override;

  virtual SkinsetContentAffixPieceLookupStack* CreateAffixPieceLookupStack(const std::string& affix);
  virtual const std::string& GetMappedAction(size_t pieceIndex, const std::string& actionName, const struct _SkeletonContentActionKeyFrame* actionKeyFrame = nullptr);

};

};
