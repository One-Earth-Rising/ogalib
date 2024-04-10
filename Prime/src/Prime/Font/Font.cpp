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

#include <Prime/Font/Font.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/Graphics.h>
#include <utf8/utf8.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define FONT_INTERNAL_WRAP_BUFFER_LEN     1024
#define FONT_WORD_BREAK_SINGLE_CHARS_START  10000

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

typedef struct _FontCharVertex {
  f32 x, y;
  f32 u, v;
} FontCharVertex;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Font::Font() {

}

Font::~Font() {

}

void Font::SetContent(Content* content) {
  SetContent(dynamic_cast<FontContent*>(content));
}

void Font::SetContent(FontContent* content) {
  this->content = content;

  if(!content)
    return;
}

f32 Font::GetLineH() const {
  if(!content)
    return 0.0f;

  refptr<FontContentSheet> sheet = content->GetSheet();
  if(!sheet)
    return 0.0f;

  return sheet->GetLineH();
}

f32 Font::GetStringW(const char* start, const char* end) const {
  if(!content)
    return 0.0f;

  if(start == nullptr || start == end)
    return 0.0f;

  if(end == nullptr)
    end = start + strlen(start);

  refptr<FontContentSheet> sheet = content->GetSheet();
  if(!sheet)
    return 0.0f;

  const FontContentValues& sheetValues = sheet->GetValues();

  f32 result = 0.0f;
  size_t charCount = 0;
  const FontCharInfo* prevCharInfo = nullptr;

  const char* iter = start;
  while(iter != end) {
    const char* charStart = iter;
    char charCode[5];

    utf8::next(iter, end);

    u32 cIndex = 0;
    u32 c = 0;
    while(cIndex < sizeof(charCode) - 1 && charStart != iter) {
      char cc = *charStart++;
      c |= ((u8) cc) << (cIndex << 3);
      charCode[cIndex++] = cc;
    }
    charCode[cIndex] = 0;

    const FontCharInfo* info = sheet->GetCharInfo(c);
    if(!info) {
      if(c <= 32) {
        // skip
      }
      else {
        if(!info) {
          info = sheet->GetCharInfo('*');
        }
        if(!info) {
          info = sheet->GetCharInfo('?');
        }
      }
    }

    if(info) {
      result += info->w;
      if(info->c == 32) {
        result += sheetValues.spaceAdvance;
      }
      if(info->kerning && prevCharInfo) {
        if(auto it = prevCharInfo->kerning->Find(info->c)) {
          result += it.value();
        }
      }
    }

    prevCharInfo = info;
  }

  return result;
}

f32 Font::GetStringW(const std::string& text) const {
  const char* start = text.c_str();
  const char* end = start + text.size();
  return GetStringW(start, end);
}

void Font::Draw(const char* start, Align align) {
  Draw(start, nullptr, align);
}

void Font::Draw(const char* start, const char* end, Align align) {
  if(!content)
    return;

  if(start == nullptr || start == end)
    return;

  if(end == nullptr)
    end = start + strlen(start);

  refptr<FontContentSheet> sheet = content->GetSheet();
  if(!sheet)
    return;

  refptr<Tex> tex = sheet->GetTex();
  if(!tex)
    return;

  const TexData* texData = tex->GetTexData("");
  if(!texData || texData->format == TexFormatNone)
    return;

  const FontContentValues& sheetValues = sheet->GetValues();
  Graphics& g = PxGraphics;
  f32 px = 0.0f;
  f32 py = 0.0f;

  std::string text;
  size_t charCount = 0;

  const char* iter = start;
  while(iter != end) {
    const char* charStart = iter;
    char charCode[5];

    utf8::next(iter, end);

    u32 cIndex = 0;
    char32_t c = 0;
    while(cIndex < sizeof(charCode) - 1 && charStart != iter) {
      char cc = *charStart++;
      c |= ((u8) cc) << (cIndex << 3);
      charCode[cIndex++] = cc;
    }
    charCode[cIndex] = 0;

    size_t size = sizeof(c);

    text.append(charCode);
    charCount++;
  }

  content->CheckReload();

  FontCharVertex* vertices = nullptr;
  void* indices = nullptr;
  size_t currIndex = 0;
  size_t vertexCount = charCount * 4;

  Pair<std::string, size_t> itemKey = {text, sheet->GetId()};
  if(textCacheItems.HasKey(itemKey)) {
    FontTextCacheItem& item = textCacheItems[itemKey];

    if(align != AlignBottomLeft) {
      f32 ax, ay;

      if((align & AlignRight) != 0)
        ax = -GetStringW(text);
      else if((align & AlignHCenter) != 0)
        ax = -GetStringW(text) * 0.5f;
      else
        ax = 0.0f;

      if((align & AlignTop) != 0)
        ay = -GetLineH();
      else if((align & AlignVCenter) != 0)
        ay = -GetLineH() * 0.5f;
      else
        ay = 0.0f;

      g.model.Push().Translate(ax, ay);
    }

    g.Draw(item.ab, item.ib, tex);

    if(align != AlignBottomLeft) {
      g.model.Pop();
    }

    item.lastUsedTime = GetSystemTime();
    return;
  }

  content->AddChars(text.c_str());

  vertices = (FontCharVertex*) calloc(vertexCount, sizeof(FontCharVertex));
  if(vertices) {
    if(vertexCount < 0x100) {
      indices = calloc(charCount * 6, sizeof(u8));
    }
    else if(vertexCount < 0x10000) {
      indices = calloc(charCount * 6, sizeof(u16));
    }
    else {
      indices = calloc(charCount * 6, sizeof(u32));
    }

    if(!indices) {
      PrimeSafeFree(vertices);
      return;
    }
  }
  else {
    return;
  }

  const FontCharInfo* prevCharInfo = nullptr;

  iter = start;
  while(iter != end) {
    const char* charStart = iter;
    char charCode[5];

    utf8::next(iter, end);

    u32 cIndex = 0;
    u32 c = 0;
    while(cIndex < sizeof(charCode) - 1 && charStart != iter) {
      char cc = *charStart++;
      c |= ((u8) cc) << (cIndex << 3);
      charCode[cIndex++] = cc;
    }
    charCode[cIndex] = 0;

    const FontCharInfo* info = sheet->GetCharInfo(c);
    if(!info) {
      if(c <= 32) {
        // skip
      }
      else {
        if(!info) {
          info = sheet->GetCharInfo('*');
        }
        if(!info) {
          info = sheet->GetCharInfo('?');
        }
      }
    }

    if(prevCharInfo) {
      px += prevCharInfo->w;
      if(prevCharInfo->c == 32) {
        px += sheetValues.spaceAdvance;
      }
      if(prevCharInfo->kerning && info) {
        if(auto it = prevCharInfo->kerning->Find(info->c)) {
          px += it.value();
        }
      }
    }

    if(info && info->tw > 0 && info->th > 0) {
      if(currIndex < charCount) {
        f32 w = info->tw;
        f32 h = info->th;
        f32 sx = info->tx;
        f32 sy = info->ty;
        f32 u1 = sx * texData->mu / texData->tw;
        f32 v1 = sy * texData->mv / texData->th;
        f32 u2 = (sx + info->tw) * texData->mu / texData->tw;
        f32 v2 = (sy + info->th) * texData->mv / texData->th;
        f32 vx = px + (s32) info->sx;
        f32 vy = py + (s32) info->sy;

        FontCharVertex* v = &vertices[currIndex * 4];

        v->x = vx;
        v->y = vy;
        v->u = u1;
        v->v = v2;
        v++;

        v->x = vx + w;
        v->y = vy;
        v->u = u2;
        v->v = v2;
        v++;

        v->x = vx + w;
        v->y = vy + h;
        v->u = u2;
        v->v = v1;
        v++;

        v->x = vx;
        v->y = vy + h;
        v->u = u1;
        v->v = v1;
        v++;

        if(vertexCount < 0x100) {
          u8* index = &((u8*) indices)[currIndex * 6];
          u8 iv = (u8) (currIndex * 4);
          *index++ = iv;
          *index++ = iv + 1;
          *index++ = iv + 2;
          *index++ = iv;
          *index++ = iv + 2;
          *index++ = iv + 3;
        }
        else if(vertexCount < 0x10000) {
          u16* index = &((u16*) indices)[currIndex * 6];
          u16 iv = (u16) (currIndex * 4);
          *index++ = iv;
          *index++ = iv + 1;
          *index++ = iv + 2;
          *index++ = iv;
          *index++ = iv + 2;
          *index++ = iv + 3;
        }
        else {
          u32* index = &((u32*) indices)[currIndex * 6];
          u32 iv = (u32) (currIndex * 4);
          *index++ = iv;
          *index++ = iv + 1;
          *index++ = iv + 2;
          *index++ = iv;
          *index++ = iv + 2;
          *index++ = iv + 3;
        }

        currIndex++;
      }
    }

    prevCharInfo = info;
  }

  FontTextCacheItem item;
  memset(&item, 0, sizeof(item));

  IndexFormat indexFormat;
  if(vertexCount < 0x100) {
    indexFormat = IndexFormatSize8;
  }
  else if(vertexCount < 0x10000) {
    indexFormat = IndexFormatSize16;
  }
  else {
    indexFormat = IndexFormatSize32;
  }

  item.ab = ArrayBuffer::Create(sizeof(FontCharVertex), vertices, vertexCount, BufferPrimitiveTriangles);
  if(item.ab) {
    item.ab->LoadAttribute("vPos", sizeof(f32) * 2);
    item.ab->LoadAttribute("vUV", sizeof(f32) * 2);

    item.ib = IndexBuffer::Create(indexFormat, indices, charCount * 6);
    if(item.ib) {
      item.lastUsedTime = GetSystemTime();
    }
  }

  PrimeSafeFree(vertices);
  PrimeSafeFree(indices);

  if(item.ab && item.ib) {
    textCacheItems[itemKey] = item;
    g.Draw(item.ab, item.ib, tex);
  }
}

void Font::Draw(const std::string& text, Align align) {
  const char* start = text.c_str();
  const char* end = start + text.size();
  Draw(start, end, align);
}
