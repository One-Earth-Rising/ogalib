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

#include <Prime/Skinset/SkinsetContent.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Skeleton/SkeletonContent.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

SkinsetContent::SkinsetContent():
pieces(nullptr),
pieceCount(0) {

}

SkinsetContent::~SkinsetContent() {
  PrimeSafeDeleteArray(pieces);
}

bool SkinsetContent::Load(const json& data, const json& info) {
  if(!Content::Load(data, info))
    return false;

  if(!data.IsObject())
    return false;

  Stack<SkinsetContentPiece*> parsedPieces;

  if(auto itPieces = data.find("pieces")) {
    if(itPieces.IsArray()) {
      for(auto& piece: itPieces) {
        if(piece.IsObject()) {
          SkinsetContentPiece* parsedPiece = new SkinsetContentPiece();
          if(parsedPiece) {
            if(auto it = piece.find("name"))
              parsedPiece->name = it.GetString();

            if(auto it = piece.find("content")) {
              parsedPiece->content = it.GetString();

              // Correct an original export bug.
              if(parsedPiece->content == ".json") {
                parsedPiece->content = "";
              }
            }

            if(auto it = piece.find("action"))
              parsedPiece->action = it.GetString();

            if(auto it = piece.find("skin")) {
              parsedPiece->skin = it.GetString();

              // Correct an original export bug.
              if(parsedPiece->skin == ".json") {
                parsedPiece->skin = "";
              }
            }

            if(auto it = piece.find("affix"))
              parsedPiece->affix = it.GetString();

            if(auto it = piece.find("affixType")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedPiece->affixType = (SkinsetAffixType) value.GetInt();
              }
              else if(value.IsString()) {
                parsedPiece->affixType = GetEnumSkinsetAffixTypeFromString(it.GetString());
              }
            }

            if(auto it = piece.find("affixX")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedPiece->affixX = value.GetFloat();
              }
            }

            if(auto it = piece.find("affixY")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedPiece->affixY = value.GetFloat();
              }
            }

            if(auto it = piece.find("baseAngle")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedPiece->baseAngle = value.GetFloat();
              }
            }

            if(auto it = piece.find("baseScaleX")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedPiece->baseScaleX = value.GetFloat();
              }
            }

            if(auto it = piece.find("baseScaleY")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedPiece->baseScaleY = value.GetFloat();
              }
            }

            parsedPiece->baseTransform.LoadRotation(-parsedPiece->baseAngle).Scale(parsedPiece->baseScaleX, parsedPiece->baseScaleY);

            parsedPieces.Add(parsedPiece);
          }
        }
      }
    }
  }

  pieceCount = parsedPieces.GetCount();
  if(pieceCount) {
    pieces = new SkinsetContentPiece[pieceCount];

    for(size_t i = 0; i < pieceCount; i++) {
      SkinsetContentPiece& piece = pieces[i];
      SkinsetContentPiece* parsedPiece = parsedPieces[i];

      piece = *parsedPiece;

      PrimeSafeDelete(parsedPiece);
    }
  }

  return true;
}

void SkinsetContent::GetWalkReferences(Stack<std::string>& paths) const {
  Content::GetWalkReferences(paths);

  for(size_t i = 0; i < pieceCount; i++) {
    SkinsetContentPiece& piece = pieces[i];
    if(!piece.content.empty()) {
      paths.Add(piece.content);
    }
    if(!piece.skin.empty() && StartsWith(piece.skin, "/")) {
      paths.Add(piece.skin);
    }
  }
}

SkinsetContentAffixPieceLookupStack* SkinsetContent::CreateAffixPieceLookupStack(const std::string& affix) {
  SkinsetContentAffixPieceLookupStack* result = new SkinsetContentAffixPieceLookupStack();

  for(size_t i = 0; i < pieceCount; i++) {
    const SkinsetContentPiece& piece = pieces[i];
    if(piece.affix == affix) {
      result->Push(i);
    }
  }

  return result;
}

const std::string& SkinsetContent::GetMappedAction(size_t pieceIndex, const std::string& actionName, const SkeletonContentActionKeyFrame* actionKeyFrame) {
  PrimeAssert(pieceIndex < pieceCount, "Invalid skinset piece.");
  SkinsetContentPiece& piece = pieces[pieceIndex];

  if(actionKeyFrame) {
    for(size_t i = 0; i < actionKeyFrame->pieceActionMappingCount; i++) {
      SkeletonContentActionKeyFramePieceActionMapping& mapping = actionKeyFrame->pieceActionMappings[i];
      if(mapping.piece == piece.name) {
        return mapping.action;
      }
    }
  }

  return piece.action;
}
