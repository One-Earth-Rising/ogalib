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

#include <Prime/Skinset/Skinset.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Imagemap/Imagemap.h>
#include <Prime/Skeleton/Skeleton.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Skinset::Skinset() {

}

Skinset::~Skinset() {

}

void Skinset::SetContent(Content* content) {
  SetContent(dynamic_cast<SkinsetContent*>(content));
}

void Skinset::SetContent(SkinsetContent* content) {
  pieces = nullptr;

  this->content = content;

  if(!content)
    return;

  size_t pieceCount = content->GetPieceCount();

  if(pieceCount) {
    const SkinsetContentPiece* contentPieces = content->GetPieces();
    refptr skinsetContent = content;

    refptr newPieces = new SkinsetPieces(pieceCount);
    pieces = newPieces;

    for(size_t i = 0; i < pieceCount; i++) {
      const SkinsetContentPiece& contentPiece = contentPieces[i];

      skinsetContent->GetContent(contentPiece.content, [=](Content* content) {
        auto piece = (*newPieces)[i];

        if(content->IsInstance<ImagemapContent>()) {
          piece->imagemap = new Imagemap();
          piece->imagemap->SetContent(content);
        }
        else if(content->IsInstance<SkeletonContent>()) {
          refptr newSkeleton = new Skeleton();
          piece->skeleton = newSkeleton;
          piece->skeleton->SetContent(content);

          skinsetContent->GetContent(contentPiece.skin, [=](Content* content) {
            if(content->IsInstance<SkinsetContent>()) {
              refptr newSkinset = new Skinset();
              newSkinset->SetContent(content);
              newSkeleton->SetSkinset(newSkinset);
            }
          });
        }
      });
    }
  }
}

void Skinset::Calc(f32 dt) {
  for(size_t i = 0; i < pieces->GetCount(); i++) {
    auto piece = (*pieces)[i];

    if(piece->skeleton) {
      piece->skeleton->Calc(dt);
    }
  }
}

refptr<SkinsetPieces> Skinset::GetPieces() const {
  return pieces;
}

size_t Skinset::GetPieceCount() const {
  if(pieces)
    return pieces->GetCount();
  else
    return 0;
}

void Skinset::SetAction(const std::string& action, const SkeletonContentActionKeyFrame* actionKeyFrame, bool setIfNew) {
  if(!HasContent())
    return;

  if(!pieces)
    return;

  for(size_t i = 0; i < pieces->GetCount(); i++) {
    SetPieceAction(i, content->GetMappedAction(i, action, actionKeyFrame), setIfNew);
  }
}

void Skinset::SetPieceAction(size_t index, const std::string& action, bool setIfNew, f32 setTime) {
  if(!pieces || index >= pieces->GetCount())
    return;

  auto piece = (*pieces)[index];
  if(piece) {
    if(piece->imagemap) {
      piece->imagemap->SetRect(action);
    }
    else if(piece->skeleton) {
      if(setIfNew) {
        if(piece->skeleton->SetActionIfNew(action)) {
          if(setTime >= 0.0f) {
            piece->skeleton->SetActionTime(setTime);
          }
        }
      }
      else {
        piece->skeleton->SetAction(action);
      }
    }
  }
}

size_t Skinset::GetPieceBoneIndex(size_t index) {
  PrimeAssert(index < GetPieceCount(), "Invalid skinset piece.");
  auto piece = (*pieces)[index];
  return piece->boneIndex;
}

void Skinset::SetPieceBoneIndex(size_t index, size_t boneIndex) {
  PrimeAssert(index < GetPieceCount(), "Invalid skinset piece.");
  auto piece = (*pieces)[index];
  if(piece)
    piece->boneIndex = boneIndex;
}

size_t Skinset::GetPieceParentBoneIndex(size_t index) {
  PrimeAssert(index < GetPieceCount(), "Invalid skinset piece.");
  auto piece = (*pieces)[index];
  if(piece)
    return piece->parentBoneIndex;
  else
    return PrimeNotFound;
}

void Skinset::SetPieceParentBoneIndex(size_t index, size_t boneIndex) {
  PrimeAssert(index < GetPieceCount(), "Invalid skinset piece.");
  auto piece = (*pieces)[index];
  if(piece)
    piece->parentBoneIndex = boneIndex;
}

size_t Skinset::GetTreeBoneCount() const {
  size_t result = 0;

  for(size_t i = 0; i < GetPieceCount(); i++) {
    auto piece = (*pieces)[i];
    if(piece->skeleton) {
      result += piece->skeleton->GetTreeBoneCount();
    }
  }

  return result;
}

size_t Skinset::GetTreePieceCount() const {
  size_t result = GetPieceCount();

  for(size_t i = 0; i < GetPieceCount(); i++) {
    auto piece = (*pieces)[i];
    if(piece->skeleton) {
      result += piece->skeleton->GetSkinsetTreePieceCount();      
    }
  }

  return result;
}
