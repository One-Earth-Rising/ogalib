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

#include <Prime/Font/FontContent.h>
#include <Prime/Imagemap/ImagemapContent.h>
#include <Prime/Types/Pair.h>

////////////////////////////////////////////////////////////////////////////////
// Enums
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  FontDataStateNone = 0,
  FontDataStateLoadQueued,
  FontDataStateAvailable,
  FontDataStateReady,
  FontDataStateWaitingOnPrerenderedTex,
} FontDataState;

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _FontAlignBox {
  f32 x;
  f32 y;
  f32 w;
  f32 h;
  u32 align;
} FontAlignBox;

typedef struct _FontWordSizeResult {
  const char* charNext;
  const char* charNextNonWhiteSpace;
  const char* charNextNonWhiteSpaceOriginal;
  const char* charFinalNonWhiteSpace;
  const char* charFinalNonWhiteSpaceOriginal;
  f32 size;
  f32 sizeWithPreSpace;
  bool crlf;
} FontWordSizeResult;

typedef struct _FontWrapBuffer {
  union {
    const char* index;
    u32 lineCount;
  };
  f32 w;
} FontWrapBuffer;

typedef struct _FontTextCacheItem {
  refptr<ArrayBuffer> ab;
  refptr<IndexBuffer> ib;
  f64 lastUsedTime;
} FontTextCacheItem;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Font: public RefObject {
private:

  refptr<FontContent> content;

  Dictionary<Pair<std::string, size_t>, FontTextCacheItem> textCacheItems;

public:

  refptr<FontContent> GetFontContent() const {return content;}
  bool HasContent() const {return (bool) content;}

public:

  Font();
  ~Font();

public:

  virtual void SetContent(Content* content);
  virtual void SetContent(FontContent* content);

  virtual f32 GetLineH() const;
  virtual f32 GetStringW(const char* start, const char* end = nullptr) const;
  virtual f32 GetStringW(const std::string& text) const;

  virtual void Draw(const char* start, Align align = AlignBottomLeft);
  virtual void Draw(const char* start, const char* end, Align align = AlignBottomLeft);
  virtual void Draw(const std::string& text, Align align = AlignBottomLeft);

};

};
