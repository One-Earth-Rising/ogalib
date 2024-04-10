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
#include <Prime/Types/Set.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class FontCharInfo {
public:

  char32_t c;
  f32 w;
  Dictionary<char32_t, f32>* kerning;
  f32 sx;
  f32 sy;
  u16 tx;
  u16 ty;
  u16 tw;
  u16 th;

public:

  FontCharInfo(): kerning(NULL) {}
  FontCharInfo(const FontCharInfo& other): kerning(NULL) {(void) operator=(other);}

  FontCharInfo& operator=(s32 value) {
    if(value == 0) {
      c = 0;
      w = 0.0f;
      PrimeSafeDelete(kerning);
      tx = 0;
      ty = 0;
      tw = 0;
      th = 0;
      sx = 0;
      sy = 0;
    }
    return *this;
  }

  FontCharInfo& operator=(const FontCharInfo& other) {
    c = other.c;
    w = other.w;
    if(other.kerning) {
      if(!kerning)
        kerning = new Dictionary<char32_t, f32>();

      if(kerning) {
        kerning->Clear();
        for(auto it: (*other.kerning)) {
          (*kerning)[it.key()] = it.value();
        }
      }
    }
    else {
      PrimeSafeDelete(kerning);
    }
    tx = other.tx;
    ty = other.ty;
    tw = other.tw;
    th = other.th;
    sx = other.sx;
    sy = other.sy;
    return *this;
  }

  bool operator==(const FontCharInfo& other) const {
    return c == other.c;
  }

  bool operator<(const FontCharInfo& other) const {
    return c < other.c;
  }

};

class FontContentValues {
public:

  f32 size;
  f32 outline;
  f32 lineAdvance;
  f32 spaceAdvance;
  bool kerning;

  f32 colorR;
  f32 colorG;
  f32 colorB;
  f32 colorA;
  f32 color2R;
  f32 color2G;
  f32 color2B;
  f32 color2A;
  f32 color3R;
  f32 color3G;
  f32 color3B;
  f32 color3A;
  f32 gradient;
  f32 gradientTop;
  f32 gradientBottom;

  f32 colorOutlineR;
  f32 colorOutlineG;
  f32 colorOutlineB;
  f32 colorOutlineA;
  f32 colorOutline2R;
  f32 colorOutline2G;
  f32 colorOutline2B;
  f32 colorOutline2A;
  f32 colorOutline3R;
  f32 colorOutline3G;
  f32 colorOutline3B;
  f32 colorOutline3A;
  f32 gradientOutline;
  f32 gradientOutlineTop;
  f32 gradientOutlineBottom;

public:

  FontContentValues();
  FontContentValues(const FontContentValues& other);
  ~FontContentValues();

  FontContentValues& operator=(const FontContentValues& other);

public:

  void SetValues(const json& values);
  void GetValues(json& values) const;

};

class FontContentSheet: public RefObject {
friend class FontContent;
private:

  size_t id;
  FontContentValues values;

  f32 lineH;

  Stack<FontCharInfo> charInfo;
  Dictionary<char32_t, size_t> charInfoLookup;
  Set<char32_t> chars;

  refptr<Tex> tex;

public:

  size_t GetId() const {return id;}
  const FontContentValues& GetValues() const {return values;}

  f32 GetLineH() const {return lineH;}

  refptr<Tex> GetTex() const {return tex;}

public:

  FontContentSheet();
  ~FontContentSheet();

public:

  void SetValues(const json& values);
  void GetValues(json& values);

  const FontCharInfo* GetCharInfo(char32_t c);

};

class FontContent: public Content {
private:

  std::string fontData;

  FontContentValues values;
  refptr<FontContentSheet> sheet;
  size_t sheetId;

  TexFormat texFormat;
  Set<char32_t> addedChars;

  u32 loadingCount;
  bool reload;

  ThreadMutex* mutex;

public:

  FontContentSheet* GetSheet() {return sheet;}

public:

  FontContent();
  ~FontContent();

public:

  bool Load(const void* data, size_t dataSize, const json& info) override;

  virtual void SetTexFormat(TexFormat texFormat);

  virtual void AddChar(char32_t c);
  virtual void AddChars(const char* start, const char* end = nullptr);
  virtual void CheckReload();

public:

  static void GetDefaultValues(json& values);

protected:

  static void GetCharCode(u32 c, char* charCode, size_t charCodeSize);

  static void CopyPixels16(u8* src, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destX, size_t destY, size_t destStride, f32 r, f32 g, f32 b, f32 a);
  static void BlendPixels16(u8* src, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destX, size_t destY, size_t destStride, f32 r, f32 g, f32 b, f32 a);

  static void CopyPixels32(u8* src, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destX, size_t destY, size_t destStride, f32 r, f32 g, f32 b, f32 a);
  static void BlendPixels32(u8* src, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destX, size_t destY, size_t destStride, f32 r, f32 g, f32 b, f32 a);

  static void CopyGlyph16(u8* src, size_t x, size_t y, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destStride, f32 r, f32 g, f32 b, f32 a, f32 r2, f32 g2, f32 b2, f32 a2, f32 r3, f32 g3, f32 b3, f32 a3, f32 gradient, f32 gradientTop, f32 gradientBottom, f32 gradientStart, f32 gradientRate);
  static void BlendGlyph16(u8* src, size_t x, size_t y, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destStride, f32 r, f32 g, f32 b, f32 a, f32 r2, f32 g2, f32 b2, f32 a2, f32 r3, f32 g3, f32 b3, f32 a3, f32 gradient, f32 gradientTop, f32 gradientBottom, f32 gradientStart, f32 gradientRate);

  static void CopyGlyph32(u8* src, size_t x, size_t y, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destStride, f32 r, f32 g, f32 b, f32 a, f32 r2, f32 g2, f32 b2, f32 a2, f32 r3, f32 g3, f32 b3, f32 a3, f32 gradient, f32 gradientTop, f32 gradientBottom, f32 gradientStart, f32 gradientRate);
  static void BlendGlyph32(u8* src, size_t x, size_t y, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destStride, f32 r, f32 g, f32 b, f32 a, f32 r2, f32 g2, f32 b2, f32 a2, f32 r3, f32 g3, f32 b3, f32 a3, f32 gradient, f32 gradientTop, f32 gradientBottom, f32 gradientStart, f32 gradientRate);

  static BlockBuffer* ConvertFrom32To16(BlockBuffer* src, size_t w, size_t h, size_t activeH);

};

};
