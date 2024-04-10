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

#include <Prime/Font/FontContent.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/Graphics.h>
#include <Prime/Engine.h>
#include <fttools/fttools.h>
#include <utf8/utf8.h>

using namespace Prime;
using namespace ftt;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define FONT_CONTENT_MAX_TEXTURE_W  8192
#define FONT_CONTENT_MAX_TEXTURE_H  8192

#if defined(PrimeTargetOpenGL)
#define FONT_PIXEL_16_REVERSE_FORMAT
#endif

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

FontContentValues::FontContentValues():
size(0.0f),
outline(0.0f),
lineAdvance(0.0f),
spaceAdvance(0.0f),
kerning(false),
colorR(0.0f),
colorG(0.0f),
colorB(0.0f),
colorA(0.0f),
color2R(0.0f),
color2G(0.0f),
color2B(0.0f),
color2A(0.0f),
color3R(0.0f),
color3G(0.0f),
color3B(0.0f),
color3A(0.0f),
gradient(0.0f),
gradientTop(0.0f),
gradientBottom(0.0f),
colorOutlineR(0.0f),
colorOutlineG(0.0f),
colorOutlineB(0.0f),
colorOutlineA(0.0f),
colorOutline2R(0.0f),
colorOutline2G(0.0f),
colorOutline2B(0.0f),
colorOutline2A(0.0f),
colorOutline3R(0.0f),
colorOutline3G(0.0f),
colorOutline3B(0.0f),
colorOutline3A(0.0f),
gradientOutline(0.0f),
gradientOutlineTop(0.0f),
gradientOutlineBottom(0.0f) {

}

FontContentValues::~FontContentValues() {

}

FontContentValues::FontContentValues(const FontContentValues& other) {
  (void) operator=(other);
}

FontContentValues& FontContentValues::operator=(const FontContentValues& other) {
  json otherValues;
  other.GetValues(otherValues);
  SetValues(otherValues);
  
  return *this;
}

void FontContentValues::SetValues(const json& values) {
  if(auto it = values.find("h"))  // backwards compatible with size value "h" (font height)
    size = it.GetFloat();

  if(auto it = values.find("size"))
    size = it.GetFloat();

  if(auto it = values.find("outline"))
    outline = it.GetFloat();

  if(auto it = values.find("lineAdvance"))
    lineAdvance = it.GetFloat();

  if(auto it = values.find("spaceAdvance"))
    spaceAdvance = it.GetFloat();

  if(auto it = values.find("kerning"))
    kerning = it.GetBool();

  if(auto it = values.find("colorR"))
    colorR = it.GetFloat();

  if(auto it = values.find("colorG"))
    colorG = it.GetFloat();

  if(auto it = values.find("colorB"))
    colorB = it.GetFloat();

  if(auto it = values.find("colorA"))
    colorA = it.GetFloat();

  if(auto it = values.find("color2R"))
    color2R = it.GetFloat();

  if(auto it = values.find("color2G"))
    color2G = it.GetFloat();

  if(auto it = values.find("color2B"))
    color2B = it.GetFloat();

  if(auto it = values.find("color2A"))
    color2A = it.GetFloat();

  if(auto it = values.find("color3R"))
    color3R = it.GetFloat();

  if(auto it = values.find("color3G"))
    color3G = it.GetFloat();

  if(auto it = values.find("color3B"))
    color3B = it.GetFloat();

  if(auto it = values.find("color3A"))
    color3A = it.GetFloat();

  if(auto it = values.find("gradient"))
    gradient = it.GetFloat();

  if(auto it = values.find("gradientTop"))
    gradientTop = it.GetFloat();

  if(auto it = values.find("gradientBottom"))
    gradientBottom = it.GetFloat();

  if(auto it = values.find("colorOutlineR"))
    colorOutlineR = it.GetFloat();

  if(auto it = values.find("colorOutlineG"))
    colorOutlineG = it.GetFloat();

  if(auto it = values.find("colorOutlineB"))
    colorOutlineB = it.GetFloat();

  if(auto it = values.find("colorOutlineA"))
    colorOutlineA = it.GetFloat();

  if(auto it = values.find("colorOutline2R"))
    colorOutline2R = it.GetFloat();

  if(auto it = values.find("colorOutline2G"))
    colorOutline2G = it.GetFloat();

  if(auto it = values.find("colorOutline2B"))
    colorOutline2B = it.GetFloat();

  if(auto it = values.find("colorOutline2A"))
    colorOutline2A = it.GetFloat();

  if(auto it = values.find("colorOutline3R"))
    colorOutline3R = it.GetFloat();

  if(auto it = values.find("colorOutline3G"))
    colorOutline3G = it.GetFloat();

  if(auto it = values.find("colorOutline3B"))
    colorOutline3B = it.GetFloat();

  if(auto it = values.find("colorOutline3A"))
    colorOutline3A = it.GetFloat();

  if(auto it = values.find("gradientOutline"))
    gradientOutline = it.GetFloat();

  if(auto it = values.find("gradientOutlineTop"))
    gradientOutlineTop = it.GetFloat();

  if(auto it = values.find("gradientOutlineBottom"))
    gradientOutlineBottom = it.GetFloat();
}

void FontContentValues::GetValues(json& values) const {
  values["size"] = size;
  values["outline"] = outline;
  values["lineAdvance"] = lineAdvance;
  values["spaceAdvance"] = spaceAdvance;
  values["kerning"] = kerning;

  values["colorR"] = colorR;
  values["colorG"] = colorG;
  values["colorB"] = colorB;
  values["colorA"] = colorA;
  values["color2R"] = color2R;
  values["color2G"] = color2G;
  values["color2B"] = color2B;
  values["color2A"] = color2A;
  values["color3R"] = color3R;
  values["color3G"] = color3G;
  values["color3B"] = color3B;
  values["color3A"] = color3A;
  values["gradient"] = gradient;
  values["gradientTop"] = gradientTop;
  values["gradientBottom"] = gradientBottom;

  values["colorOutlineR"] = colorOutlineR;
  values["colorOutlineG"] = colorOutlineG;
  values["colorOutlineB"] = colorOutlineB;
  values["colorOutlineA"] = colorOutlineA;
  values["colorOutline2R"] = colorOutline2R;
  values["colorOutline2G"] = colorOutline2G;
  values["colorOutline2B"] = colorOutline2B;
  values["colorOutline2A"] = colorOutline2A;
  values["colorOutline3R"] = colorOutline3R;
  values["colorOutline3G"] = colorOutline3G;
  values["colorOutline3B"] = colorOutline3B;
  values["colorOutline3A"] = colorOutline3A;
  values["gradientOutline"] = gradientOutline;
  values["gradientOutlineTop"] = gradientOutlineTop;
  values["gradientOutlineBottom"] = gradientOutlineBottom;
}

FontContentSheet::FontContentSheet():
id(0),
lineH(0.0f) {

}

FontContentSheet::~FontContentSheet() {

}

void FontContentSheet::SetValues(const json& values) {
  this->values.SetValues(values);
}

void FontContentSheet::GetValues(json& values) {
  this->values.SetValues(values);
}

const FontCharInfo* FontContentSheet::GetCharInfo(char32_t c) {
  if(auto it = charInfoLookup.Find(c))
    return &charInfo[it.value()];

  return nullptr;
}

FontContent::FontContent():
sheetId(0),
texFormat(TexFormatNone),
loadingCount(0),
reload(false) {
  mutex = new ThreadMutex("FontContent mutex", true);

  json defaultValues;
  GetDefaultValues(defaultValues);
  values.SetValues(defaultValues);
}

FontContent::~FontContent() {
  PrimeSafeDelete(mutex);
}

bool FontContent::Load(const void* data, size_t dataSize, const json& info) {
  bool copyFontData = true;

  if(data == nullptr || dataSize == 0) {
    bool available;

    mutex->Lock();

    size_t fontDataSize = fontData.size();
    if(fontDataSize > 0) {
      data = fontData.c_str();
      dataSize = fontDataSize;
      copyFontData = false;
      available = true;
    }
    else {
      available = false;
    }

    mutex->Unlock();

    if(!available) {
      return false;
    }
  }

  if(copyFontData) {
    mutex->Lock();

    fontData.clear();
    fontData.append((const char*) data, dataSize);

    mutex->Unlock();
  }

  FontContentSheet* newSheet = new FontContentSheet();
  if(!newSheet)
    return false;

  mutex->Lock();

  newSheet->values = values;

  if(auto it = info.find("size")) {
    if(it.IsNumber()) {
      newSheet->values.size = it.GetFloat();
    }
  }

  mutex->Unlock();

  const FontContentValues& nsv = newSheet->values;

  TexFormat useTexFormat = texFormat;

  // Ensure a valid texture format.
  if(useTexFormat != TexFormatR4G4B4A4) {
    useTexFormat = TexFormatR8G8B8A8;
  }

  Graphics& g = PxGraphics;

  size_t maxCalcTexW = FONT_CONTENT_MAX_TEXTURE_W;
  size_t graphicsMaxTexW = g.GetMaxTexW();
  if(graphicsMaxTexW != 0 && maxCalcTexW > graphicsMaxTexW)
    maxCalcTexW = graphicsMaxTexW;

  size_t maxCalcTexH = FONT_CONTENT_MAX_TEXTURE_H;
  size_t graphicsMaxTexH = g.GetMaxTexH();
  if(graphicsMaxTexH != 0 && maxCalcTexH > graphicsMaxTexH)
    maxCalcTexH = graphicsMaxTexH;

  maxCalcTexW = GetNextPowerOf2(maxCalcTexW);
  maxCalcTexH = GetNextPowerOf2(maxCalcTexH);

  size_t maxTexW = maxCalcTexW;
  size_t maxTexH = maxCalcTexH;

  f32 useSize = nsv.size;
  f32 useOutline = nsv.outline;

  u32 finalPixelSize = useTexFormat == TexFormatR4G4B4A4 ? 2 : 4;

  bool use32To16;
  u32 pixelSize;
  if(useOutline > 0.0f && useTexFormat == TexFormatR4G4B4A4) {
    // If tex format is 16-bit and not 32-bit, still render as 32-bit, but perform the downsample
    // after the outline texture + main texture is blended.  Therefore, the workspace will be 32-bit.
    pixelSize = 4;
    use32To16 = true;
  }
  else {
    pixelSize = finalPixelSize;
    use32To16 = false;
  }

  const char* charStart = FTToolsDefaultChars;
  const char* charEnd = charStart + strlen(charStart);
  const char* charIter = charStart;
  while(charIter != charEnd) {
    const char* start = charIter;
    char charCode[5];

    utf8::next(charIter, charEnd);

    u32 cIndex = 0;
    u32 c = 0;
    while(cIndex < sizeof(charCode) - 1 && start != charIter) {
      char cc = *start++;
      c |= ((u8) cc) << (cIndex << 3);
      charCode[cIndex++] = cc;
    }
    charCode[cIndex] = 0;

    newSheet->chars.Add(c);
  }

  mutex->Lock();

  newSheet->id = sheetId++;

  for(auto addedChar: addedChars) {
    if(!newSheet->chars.Find(addedChar)) {
      newSheet->chars.Add(addedChar);
    }
  }

  mutex->Unlock();

  char* loadChars = (char*) malloc(newSheet->chars.GetCount() * sizeof(char32_t) + 1);
  if(!loadChars) {
    PrimeSafeDelete(newSheet);
    return false;
  }

  char* loadCharsP = loadChars;
  for(auto c: newSheet->chars) {
    char charCode[5];
    GetCharCode(c, charCode, sizeof(charCode));

    for(u32 i = 0; i < 5; i++) {
      char cv = charCode[i];
      if(cv) {
        *loadCharsP++ = cv;
      }
    }
  }
  *loadCharsP++ = 0;

  texture_atlas_t* atlas = texture_atlas_new_ex(maxTexW, maxTexH, 1, useOutline > 0.0f ? 1 : 0);
  texture_font_t* font = texture_font_new_from_memory(atlas, useSize, data, dataSize);
  font->kerning = nsv.kerning ? 1 : 0;
  if(useOutline > 0.0f) {
    font->outlineMode = 1;
    font->outline_type = 1;
    font->outline_thickness = useOutline;
  }

  texture_font_load_glyphs(font, loadChars);

  PrimeSafeFree(loadChars);

  f32 adjustY = font->descender;
  newSheet->lineH = font->ascender - font->descender + font->linegap;
  if(font->outlineMode) {
    newSheet->lineH += font->outline_thickness * 2.0f;
  }
  f32 baseLineH = newSheet->lineH;

  bool simpleColors = nsv.gradient == -1.0f && nsv.gradientOutline == -1.0f;
  size_t usedTexH = 0;
  Stack<texture_glyph_t*> glyphs;

  // Perform an inital sweep to see the max texture height needed.
  for(auto c: newSheet->chars) {
    char charCode[5];
    GetCharCode(c, charCode, sizeof(charCode));

    texture_glyph_t* glyph = texture_font_get_glyph(font, charCode);

    if(glyph) {
      size_t charInfoIndex = newSheet->charInfo.GetCount();
      FontCharInfo fontCharInfo;

      fontCharInfo.c = c;
      fontCharInfo.tw = (u16) ((glyph->s1 - glyph->s0) * maxTexW);
      fontCharInfo.th = (u16) ((glyph->t1 - glyph->t0) * maxTexH);
      fontCharInfo.tx = (u16) (glyph->s0 * maxTexW);
      fontCharInfo.ty = (u16) (glyph->t0 * maxTexH);
      fontCharInfo.w = glyph->advance_x;
      fontCharInfo.sx = (f32) (glyph->offset_x);
      fontCharInfo.sy = (f32) (glyph->offset_y - (f32) fontCharInfo.th - adjustY);

      u32 infoBottom = fontCharInfo.ty + fontCharInfo.th;
      if(usedTexH < infoBottom) {
        usedTexH = infoBottom;
      }

      fontCharInfo.kerning = NULL;

      glyphs.Add(glyph);
      newSheet->charInfo.Add(fontCharInfo);
      newSheet->charInfoLookup[c] = charInfoIndex;

      if(font->kerning) {
        FontCharInfo& useInfo = newSheet->charInfo[charInfoIndex];

        for(auto kc: newSheet->chars) {
          char kernCharCode[5];
          GetCharCode(kc, kernCharCode, sizeof(kernCharCode));

          f32 kerning = texture_glyph_get_kerning(glyph, kernCharCode);
          if(kerning != 0.0f) {
            if(!useInfo.kerning) {
              useInfo.kerning = new Dictionary<char32_t, f32>();
            }

            if(useInfo.kerning) {
              (*useInfo.kerning)[kc] = kerning;
            }
          }
        }
      }
    }
  }

  maxTexH = GetNextPowerOf2(usedTexH);

  size_t pixelsStride = pixelSize * maxTexW;
  size_t pixelsSize = pixelsStride * maxTexH;
  BlockBuffer* pixels = new BlockBuffer(pixelsStride, pixelsSize);
  if(!pixels) {
    return false;
  }

  if(simpleColors) {
    if(useTexFormat == TexFormatR4G4B4A4) {
      if(use32To16) {
        // Perform the blending in 32-bit, and then convert to 16-bit on the final composite texture.
        PrimeAssert(pixelSize == 4, "Expected pixel size to be 4.");
        PrimeAssert(atlas->dataOutline, "Feature is meant for fonts with outlines.");
        CopyPixels32(atlas->dataOutline, maxTexW, usedTexH, maxTexW, pixels, 0, 0, maxTexW * sizeof(u32), nsv.colorOutlineR, nsv.colorOutlineG, nsv.colorOutlineB, nsv.colorOutlineA);
        BlendPixels32(atlas->data, maxTexW, usedTexH, maxTexW, pixels, 0, 0, maxTexW * sizeof(u32), nsv.colorR, nsv.colorG, nsv.colorB, nsv.colorA);
        BlockBuffer* newPixels = ConvertFrom32To16(pixels, maxTexW, maxTexH, usedTexH);
        if(newPixels) {
          PrimeSafeDelete(pixels);
          pixels = newPixels;
        }
      }
      else {
        u16* pixels16 = (u16*) pixels;
        if(atlas->dataOutline) {
          CopyPixels16(atlas->dataOutline, maxTexW, usedTexH, maxTexW, pixels, 0, 0, maxTexW * sizeof(u16), nsv.colorOutlineR, nsv.colorOutlineG, nsv.colorOutlineB, nsv.colorOutlineA);
          BlendPixels16(atlas->data, maxTexW, usedTexH, maxTexW, pixels, 0, 0, maxTexW * sizeof(u16), nsv.colorR, nsv.colorG, nsv.colorB, nsv.colorA);
        }
        else {
          CopyPixels16(atlas->data, maxTexW, usedTexH, maxTexW, pixels, 0, 0, maxTexW * sizeof(u16), nsv.colorR, nsv.colorG, nsv.colorB, nsv.colorA);
        }
      }
    }
    else {
      if(atlas->dataOutline) {
        CopyPixels32(atlas->dataOutline, maxTexW, usedTexH, maxTexW, pixels, 0, 0, maxTexW * sizeof(u32), nsv.colorOutlineR, nsv.colorOutlineG, nsv.colorOutlineB, nsv.colorOutlineA);
        BlendPixels32(atlas->data, maxTexW, usedTexH, maxTexW, pixels, 0, 0, maxTexW * sizeof(u32), nsv.colorR, nsv.colorG, nsv.colorB, nsv.colorA);
      }
      else {
        CopyPixels32(atlas->data, maxTexW, usedTexH, maxTexW, pixels, 0, 0, maxTexW * sizeof(u32), nsv.colorR, nsv.colorG, nsv.colorB, nsv.colorA);
      }
    }
  }
  else {
    size_t count = newSheet->charInfo.GetCount();
    for(size_t i = 0; i < count; i++) {
      texture_glyph_t* glyph = glyphs[i];
      FontCharInfo& fontCharInfo = newSheet->charInfo[i];

      s32 glyphX = fontCharInfo.tx;
      s32 glyphY = fontCharInfo.ty;
      s32 glyphW = fontCharInfo.tw;
      s32 glyphH = fontCharInfo.th;
      f32 gradientStart = 1.0f - (glyph->offset_y - adjustY) / baseLineH;
      f32 gradientRate = 1.0f / baseLineH;

      PrimeAssert(glyphX + glyphW <= (s32) maxTexW, "Glyph is out of texture range.");
      PrimeAssert(glyphY + glyphH <= (s32) usedTexH, "Glyph is out of texture range.");

      if(useTexFormat == TexFormatR4G4B4A4) {
        if(use32To16) {
          // Perform the blending in 32-bit, and then convert to 16-bit on the final composite texture.
          PrimeAssert(pixelSize == 4, "Expected pixel size to be 4.");
          PrimeAssert(atlas->dataOutline, "Feature is meant for fonts with outlines.");
          CopyGlyph32(atlas->dataOutline, glyphX, glyphY, glyphW, glyphH, maxTexW, pixels, maxTexW * sizeof(u32), nsv.colorOutlineR, nsv.colorOutlineG, nsv.colorOutlineB, nsv.colorOutlineA, nsv.colorOutline2R, nsv.colorOutline2G, nsv.colorOutline2B, nsv.colorOutline2A, nsv.colorOutline3R, nsv.colorOutline3G, nsv.colorOutline3B, nsv.colorOutline3A, nsv.gradientOutline, nsv.gradientOutlineTop, nsv.gradientOutlineBottom, gradientStart, gradientRate);
          BlendGlyph32(atlas->data, glyphX, glyphY, glyphW, glyphH, maxTexW, pixels, maxTexW * sizeof(u32), nsv.colorR, nsv.colorG, nsv.colorB, nsv.colorA, nsv.color2R, nsv.color2G, nsv.color2B, nsv.color2A, nsv.color3R, nsv.color3G, nsv.color3B, nsv.color3A, nsv.gradient, nsv.gradientTop, nsv.gradientBottom, gradientStart, gradientRate);
        }
        else {
          u16* pixels16 = (u16*) pixels;
          if(atlas->dataOutline) {
            CopyGlyph16(atlas->dataOutline, glyphX, glyphY, glyphW, glyphH, maxTexW, pixels, maxTexW * sizeof(u16), nsv.colorOutlineR, nsv.colorOutlineG, nsv.colorOutlineB, nsv.colorOutlineA, nsv.colorOutline2R, nsv.colorOutline2G, nsv.colorOutline2B, nsv.colorOutline2A, nsv.colorOutline3R, nsv.colorOutline3G, nsv.colorOutline3B, nsv.colorOutline3A, nsv.gradientOutline, nsv.gradientOutlineTop, nsv.gradientOutlineBottom, gradientStart, gradientRate);
            BlendGlyph16(atlas->data, glyphX, glyphY, glyphW, glyphH, maxTexW, pixels, maxTexW * sizeof(u16), nsv.colorR, nsv.colorG, nsv.colorB, nsv.colorA, nsv.color2R, nsv.color2G, nsv.color2B, nsv.color2A, nsv.color3R, nsv.color3G, nsv.color3B, nsv.color3A, nsv.gradient, nsv.gradientTop, nsv.gradientBottom, gradientStart, gradientRate);
          }
          else {
            CopyGlyph16(atlas->data, glyphX, glyphY, glyphW, glyphH, maxTexW, pixels, maxTexW * sizeof(u16), nsv.colorR, nsv.colorG, nsv.colorB, nsv.colorA, nsv.color2R, nsv.color2G, nsv.color2B, nsv.color2A, nsv.color3R, nsv.color3G, nsv.color3B, nsv.color3A, nsv.gradient, nsv.gradientTop, nsv.gradientBottom, gradientStart, gradientRate);
          }
        }
      }
      else {
        if(atlas->dataOutline) {
          CopyGlyph32(atlas->dataOutline, glyphX, glyphY, glyphW, glyphH, maxTexW, pixels, maxTexW * sizeof(u32), nsv.colorOutlineR, nsv.colorOutlineG, nsv.colorOutlineB, nsv.colorOutlineA, nsv.colorOutline2R, nsv.colorOutline2G, nsv.colorOutline2B, nsv.colorOutline2A, nsv.colorOutline3R, nsv.colorOutline3G, nsv.colorOutline3B, nsv.colorOutline3A, nsv.gradientOutline, nsv.gradientOutlineTop, nsv.gradientOutlineBottom, gradientStart, gradientRate);
          BlendGlyph32(atlas->data, glyphX, glyphY, glyphW, glyphH, maxTexW, pixels, maxTexW * sizeof(u32), nsv.colorR, nsv.colorG, nsv.colorB, nsv.colorA, nsv.color2R, nsv.color2G, nsv.color2B, nsv.color2A, nsv.color3R, nsv.color3G, nsv.color3B, nsv.color3A, nsv.gradient, nsv.gradientTop, nsv.gradientBottom, gradientStart, gradientRate);
        }
        else {
          CopyGlyph32(atlas->data, glyphX, glyphY, glyphW, glyphH, maxTexW, pixels, maxTexW * sizeof(u32), nsv.colorR, nsv.colorG, nsv.colorB, nsv.colorA, nsv.color2R, nsv.color2G, nsv.color2B, nsv.color2A, nsv.color3R, nsv.color3G, nsv.color3B, nsv.color3A, nsv.gradient, nsv.gradientTop, nsv.gradientBottom, gradientStart, gradientRate);
        }
      }
    }

    if(useTexFormat == TexFormatR4G4B4A4) {
      if(use32To16) {
        BlockBuffer* newPixels = ConvertFrom32To16(pixels, maxTexW, maxTexH, usedTexH);
        if(newPixels) {
          PrimeSafeDelete(pixels);
          pixels = newPixels;
        }
      }
    }
  }

  //dbgprintf("[Info] Font could use %zu KB, uses %zu KB after memory opt.\n", maxCalcTexW * maxCalcTexH * finalPixelSize / 1024, maxTexW * maxTexH * finalPixelSize / 1024);

  texture_font_delete(font);
  texture_atlas_delete(atlas);

  new Job(nullptr, [=](Job& job) {
    newSheet->tex = Tex::Create();
    if(newSheet->tex) {
      newSheet->tex->AddTexData("", "", {
        {"format", "raw"},
        {"subFormat", useTexFormat},
        {"w", maxTexW},
        {"h", maxTexH},
        {"pixels", pixels},
        });
    }
    else {
      delete pixels;
    }

    sheet = newSheet;
  });

  return true;
}

void FontContent::SetTexFormat(TexFormat texFormat) {
  this->texFormat = texFormat;
}

void FontContent::AddChar(char32_t c) {
  bool added = false;

  mutex->Lock();

  refptr currentSheet = sheet;

  if(c >= 32) {
    if(!addedChars.Find(c) && (!currentSheet || currentSheet->GetCharInfo(c) == nullptr)) {
      addedChars.Add(c);
      added = true;
    }
  }

  if(added) {
    reload = true;
    CheckReload();
  }

  mutex->Unlock();
}

void FontContent::AddChars(const char* start, const char* end) {
  if(start == nullptr || start == end)
    return;

  if(end == nullptr)
    end = start + strlen(start);

  bool added = false;

  mutex->Lock();

  refptr currentSheet = sheet;

  const char* charStart = start;
  const char* charEnd = end;
  const char* charIter = charStart;
  while(charIter != charEnd) {
    const char* start = charIter;
    char charCode[5];

    utf8::next(charIter, charEnd);

    u32 cIndex = 0;
    u32 c = 0;
    while(cIndex < sizeof(charCode) - 1 && start != charIter) {
      char cc = *start++;
      c |= ((u8) cc) << (cIndex << 3);
      charCode[cIndex++] = cc;
    }
    charCode[cIndex] = 0;

    if(c >= 32) {
      if(!addedChars.Find(c) && (!currentSheet || currentSheet->GetCharInfo(c) == nullptr)) {
        addedChars.Add(c);
        added = true;
      }
    }
  }

  if(added) {
    reload = true;
    CheckReload();
  }

  mutex->Unlock();
}

void FontContent::CheckReload() {
  mutex->Lock();

  if(reload && loadingCount == 0) {
    reload = false;
    loadingCount++;

    IncRef();
    new Job([=](Job& job) {
      Load(nullptr, 0, json());
    }, [=](Job& job) {
      if(loadingCount > 0) {
        loadingCount--;
      }
      DecRef();
    });
  }

  mutex->Unlock();
}

void FontContent::GetDefaultValues(json& values) {
  values["h"] = 20.0f;
  values["outline"] = 0.0f;
  values["lineAdvance"] = 0.0f;
  values["spaceAdvance"] = 0.0f;
  values["kerning"] = false;

  values["colorR"] = 1.0f;
  values["colorG"] = 1.0f;
  values["colorB"] = 1.0f;
  values["colorA"] = 1.0f;
  values["color2R"] = 1.0f;
  values["color2G"] = 1.0f;
  values["color2B"] = 1.0f;
  values["color2A"] = 1.0f;
  values["color3R"] = 1.0f;
  values["color3G"] = 1.0f;
  values["color3B"] = 1.0f;
  values["color3A"] = 1.0f;
  values["gradient"] = -1.0f;
  values["gradientTop"] = 0.0f;
  values["gradientBottom"] = 1.0f;

  values["colorOutlineR"] = 1.0f;
  values["colorOutlineG"] = 1.0f;
  values["colorOutlineB"] = 1.0f;
  values["colorOutlineA"] = 1.0f;
  values["colorOutline2R"] = 1.0f;
  values["colorOutline2G"] = 1.0f;
  values["colorOutline2B"] = 1.0f;
  values["colorOutline2A"] = 1.0f;
  values["colorOutline3R"] = 1.0f;
  values["colorOutline3G"] = 1.0f;
  values["colorOutline3B"] = 1.0f;
  values["colorOutline3A"] = 1.0f;
  values["gradientOutline"] = -1.0f;
  values["gradientOutlineTop"] = 0.0f;
  values["gradientOutlineBottom"] = 1.0f;
}

void FontContent::GetCharCode(u32 c, char* charCode, size_t charCodeSize) {
  if(charCodeSize == 0)
    return;

  if(charCodeSize == 1)
    charCode[0] = 0;

  for(size_t i = 0; i < charCodeSize - 1; i++) {
    charCode[i] = (c >> (i << 3)) & 0xFF;
  }

  charCode[charCodeSize - 1] = 0;
}

void FontContent::CopyPixels16(u8* src, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destX, size_t destY, size_t destStride, f32 r, f32 g, f32 b, f32 a) {
  u8* ps = src;
  size_t destOffset = destX * sizeof(u16) + destY * destStride;
  s32 rb = clamp((s32) (r * 15), 0, 15);
  s32 gb = clamp((s32) (g * 15), 0, 15);
  s32 bb = clamp((s32) (b * 15), 0, 15);
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
  u16 color = (rb << 12) | (gb << 8) | (bb << 4);
#else
  u16 color = rb | (gb << 4) | (bb << 8);
#endif

  for(size_t j = 0; j < h; j++) {
    u8* ps8 = (u8*) ps;

    for(size_t i = 0; i < w; i++) {
      u16* pd = (u16*) dest->GetAddr(destOffset);
      PrimeAssert(pd, "Could not get destination pixel address.");
      destOffset += sizeof(u16);

      s32 value = *ps8++;
      value = value >> 4;
      if(a == 1.0f) {
        *pd++ = color | value;
      }
      else {
        f32 aValue = value * a;
        s32 aInt = (s32) roundf(aValue);
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
        aInt = clamp(aInt, 0, 15);
#else
        aInt = clamp(aInt, 0, 15) << 12;
#endif
        *pd++ = color | aInt;
      }
    }

    ps += stride;
  }
}

void FontContent::BlendPixels16(u8* src, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destX, size_t destY, size_t destStride, f32 r, f32 g, f32 b, f32 a) {
  u8* ps = src;
  size_t destOffset = destX * sizeof(u16) + destY * destStride;
  s32 rb = clamp((s32) (r * 15), 0, 15);
  s32 gb = clamp((s32) (g * 15), 0, 15);
  s32 bb = clamp((s32) (b * 15), 0, 15);
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
  u16 color = (rb << 12) | (gb << 8) | (bb << 4);
#else
  u16 color = rb | (gb << 4) | (bb << 8);
#endif

  for(size_t j = 0; j < h; j++) {
    u8* ps8 = (u8*) ps;

    for(size_t i = 0; i < w; i++) {
      u16* pd = (u16*) dest->GetAddr(destOffset);
      PrimeAssert(pd, "Could not get destination pixel address.");
      destOffset += sizeof(u16);

      u32 baseColor = *pd;
      s32 baseColorA = baseColor & 0xF;
      s32 value = *ps8++;
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
      value = value >> 4;
#else
      value = (value >> 4) << 12;
#endif
      if(baseColorA == 0) {
        if(value > 0) {
          *pd = color | value;
        }
      }
      else {
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
        s32 baseColorR = (baseColor >> 12) & 0xF;
        s32 baseColorG = (baseColor >> 8) & 0xF;
        s32 baseColorB = (baseColor >> 4) & 0xF;
#else
        s32 baseColorR = baseColor & 0xF;
        s32 baseColorG = (baseColor >> 4) & 0xF;
        s32 baseColorB = (baseColor >> 8) & 0xF;
#endif
        f32 aValue = value * a;
        s32 aValueInt = clamp((s32) roundf(aValue * 15.0f), 0, 15);

        s32 colorR = (s32) (baseColorR * (1.0f - aValue) + r * aValue);
        s32 colorG = (s32) (baseColorG * (1.0f - aValue) + g * aValue);
        s32 colorB = (s32) (baseColorB * (1.0f - aValue) + b * aValue);
        s32 colorA = (s32) max(baseColorA, aValueInt);

        colorR = clamp(colorR, 0, 15);
        colorG = clamp(colorG, 0, 15);
        colorB = clamp(colorB, 0, 15);
        colorA = clamp(colorA, 0, 15);

#ifdef FONT_PIXEL_16_REVERSE_FORMAT
        *pd = (colorR << 12) | (colorG << 8) | (colorB << 4) | colorA;
#else
        *pd = colorR | (colorG << 4) | (colorB << 8) | (colorA << 12);
#endif
      }
    }

    ps += stride;
  }
}

void FontContent::CopyPixels32(u8* src, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destX, size_t destY, size_t destStride, f32 r, f32 g, f32 b, f32 a) {
  u8* ps = src;
  size_t destOffset = destX * sizeof(u32) + destY * destStride;
  s32 rb = clamp((s32) (r * 255), 0, 255);
  s32 gb = clamp((s32) (g * 255), 0, 255);
  s32 bb = clamp((s32) (b * 255), 0, 255);
  u32 color = rb | (gb << 8) | (bb << 16);

  for(size_t j = 0; j < h; j++) {
    u8* ps8 = (u8*) ps;

    for(size_t i = 0; i < w; i++) {
      u32* pd = (u32*) dest->GetAddr(destOffset);
      PrimeAssert(pd, "Could not get destination pixel address.");
      destOffset += sizeof(u32);

      s32 value = *ps8++;
      if(a == 1.0f) {
        *pd = color | (value << 24);
      }
      else {
        f32 aValue = value * a;
        s32 aInt = (s32) roundf(aValue);
        aInt = clamp(aInt, 0, 255);
        *pd = color | (aInt << 24);
      }
    }

    ps += stride;
  }
}

void FontContent::BlendPixels32(u8* src, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destX, size_t destY, size_t destStride, f32 r, f32 g, f32 b, f32 a) {
  u8* ps = src;
  size_t destOffset = destX * sizeof(u16) + destY * destStride;
  s32 rb = clamp((s32) (r * 255), 0, 255);
  s32 gb = clamp((s32) (g * 255), 0, 255);
  s32 bb = clamp((s32) (b * 255), 0, 255);
  u32 color = rb | (gb << 8) | (bb << 16);

  for(size_t j = 0; j < h; j++) {
    u8* ps8 = (u8*) ps;

    for(size_t i = 0; i < w; i++) {
      u32* pd = (u32*) dest->GetAddr(destOffset);
      PrimeAssert(pd, "Could not get destination pixel address.");
      destOffset += sizeof(u32);

      u32 baseColor = *pd;
      s32 baseColorA = (baseColor >> 24) & 0xFF;
      s32 value = *ps8++;
      if(baseColorA == 0) {
        if(value > 0) {
          *pd = color | (value << 24);
        }
      }
      else {
        s32 baseColorR = baseColor & 0xFF;
        s32 baseColorG = (baseColor >> 8) & 0xFF;
        s32 baseColorB = (baseColor >> 16) & 0xFF;
        f32 aValue = value * a;
        s32 aValueInt = clamp((s32) roundf(aValue * 255.0f), 0, 255);

        s32 colorR = (s32) (baseColorR * (1.0f - aValue) + r * aValue);
        s32 colorG = (s32) (baseColorG * (1.0f - aValue) + g * aValue);
        s32 colorB = (s32) (baseColorB * (1.0f - aValue) + b * aValue);
        s32 colorA = (s32) max(baseColorA, aValueInt);

        colorR = clamp(colorR, 0, 255);
        colorG = clamp(colorG, 0, 255);
        colorB = clamp(colorB, 0, 255);
        colorA = clamp(colorA, 0, 255);

        *pd = colorR | (colorG << 8) | (colorB << 16) | (colorA << 24);
      }
    }

    ps += stride;
  }
}

void FontContent::CopyGlyph16(u8* src, size_t x, size_t y, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destStride, f32 r, f32 g, f32 b, f32 a, f32 r2, f32 g2, f32 b2, f32 a2, f32 r3, f32 g3, f32 b3, f32 a3, f32 gradient, f32 gradientTop, f32 gradientBottom, f32 gradientStart, f32 gradientRate) {
  u8* ps = src + (x + y * stride);
  s32 rb = clamp((s32) (r * 15), 0, 15);
  s32 gb = clamp((s32) (g * 15), 0, 15);
  s32 bb = clamp((s32) (b * 15), 0, 15);
  f32 useGradient = clamp(gradient, 0.0f, 1.0f);
  f32 useGradientTop = clamp(gradientTop, 0.0f, 1.0f);
  f32 useGradientBottom = clamp(gradientBottom, 0.0f, 1.0f);
  f32 gradientValue = gradientStart;

  for(size_t j = 0; j < h; j++) {
    u8* ps8 = (u8*) ps;
    size_t destOffset = x * sizeof(u16) + (y + j) * destStride;

    s32 inputColorR;
    s32 inputColorG;
    s32 inputColorB;

    if(gradient == -1.0f) {
      inputColorR = (s32) (r * 15);
      inputColorG = (s32) (g * 15);
      inputColorB = (s32) (b * 15);
    }
    else if(useGradient == 0.0f) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradientBottom - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r * t + r2 * omt) * 15);
      inputColorG = (s32) ((g * t + g2 * omt) * 15);
      inputColorB = (s32) ((b * t + b2 * omt) * 15);
    }
    else if(useGradient == 1.0f) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradientBottom - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r2 * t + r3 * omt) * 15);
      inputColorG = (s32) ((g2 * t + g3 * omt) * 15);
      inputColorB = (s32) ((b2 * t + b3 * omt) * 15);
    }
    else if(gradientValue < useGradient) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradient - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r * t + r2 * omt) * 15);
      inputColorG = (s32) ((g * t + g2 * omt) * 15);
      inputColorB = (s32) ((b * t + b2 * omt) * 15);
    }
    else {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradient) / (useGradientBottom - useGradient);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r2 * t + r3 * omt) * 15);
      inputColorG = (s32) ((g2 * t + g3 * omt) * 15);
      inputColorB = (s32) ((b2 * t + b3 * omt) * 15);
    }

    inputColorR = clamp(inputColorR, 0, 15);
    inputColorG = clamp(inputColorG, 0, 15);
    inputColorB = clamp(inputColorB, 0, 15);
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
    u32 color = (inputColorR << 12) | (inputColorG << 8) | (inputColorB << 4);
#else
    u32 color = inputColorR | (inputColorG << 4) | (inputColorB << 8);
#endif

    for(size_t i = 0; i < w; i++) {
      u16* pd = (u16*) dest->GetAddr(destOffset);
      PrimeAssert(pd, "Could not get destination pixel address.");
      destOffset += sizeof(u16);

      s32 value = *ps8++;
      if(a == 1.0f) {
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
        *pd = color | (value >> 4);
#else
        *pd = color | ((value >> 4) << 12);
#endif
      }
      else {
        f32 aValue = value * a;
        s32 aInt = (s32) roundf(aValue);
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
        aInt = clamp(aInt, 0, 15);
#else
        aInt = clamp(aInt, 0, 15) << 12;
#endif
        *pd = color | aInt;
      }
    }

    ps += stride;
    gradientValue += gradientRate;
  }
}

void FontContent::BlendGlyph16(u8* src, size_t x, size_t y, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destStride, f32 r, f32 g, f32 b, f32 a, f32 r2, f32 g2, f32 b2, f32 a2, f32 r3, f32 g3, f32 b3, f32 a3, f32 gradient, f32 gradientTop, f32 gradientBottom, f32 gradientStart, f32 gradientRate) {
  u8* ps = src + (x + y * stride);
  s32 rb = clamp((s32) (r * 15), 0, 15);
  s32 gb = clamp((s32) (g * 15), 0, 15);
  s32 bb = clamp((s32) (b * 15), 0, 15);
  f32 useGradient = clamp(gradient, 0.0f, 1.0f);
  f32 useGradientTop = clamp(gradientTop, 0.0f, 1.0f);
  f32 useGradientBottom = clamp(gradientBottom, 0.0f, 1.0f);
  f32 gradientValue = gradientStart;

  for(size_t j = 0; j < h; j++) {
    u8* ps8 = (u8*) ps;
    size_t destOffset = x * sizeof(u16) + (y + j) * destStride;

    s32 inputColorR;
    s32 inputColorG;
    s32 inputColorB;

    if(gradient == -1.0f) {
      inputColorR = (s32) (r * 15);
      inputColorG = (s32) (g * 15);
      inputColorB = (s32) (b * 15);
    }
    else if(useGradient == 0.0f) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradientBottom - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r * t + r2 * omt) * 15);
      inputColorG = (s32) ((g * t + g2 * omt) * 15);
      inputColorB = (s32) ((b * t + b2 * omt) * 15);
    }
    else if(useGradient == 1.0f) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradientBottom - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r2 * t + r3 * omt) * 15);
      inputColorG = (s32) ((g2 * t + g3 * omt) * 15);
      inputColorB = (s32) ((b2 * t + b3 * omt) * 15);
    }
    else if(gradientValue < useGradient) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradient - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r * t + r2 * omt) * 15);
      inputColorG = (s32) ((g * t + g2 * omt) * 15);
      inputColorB = (s32) ((b * t + b2 * omt) * 15);
    }
    else {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradient) / (useGradientBottom - useGradient);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r2 * t + r3 * omt) * 15);
      inputColorG = (s32) ((g2 * t + g3 * omt) * 15);
      inputColorB = (s32) ((b2 * t + b3 * omt) * 15);
    }

    inputColorR = clamp(inputColorR, 0, 15);
    inputColorG = clamp(inputColorG, 0, 15);
    inputColorB = clamp(inputColorB, 0, 15);
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
    u32 color = (inputColorR << 12) | (inputColorG << 8) | (inputColorB << 4);
#else
    u32 color = inputColorR | (inputColorG << 4) | (inputColorB << 8);
#endif

    for(size_t i = 0; i < w; i++) {
      u16* pd = (u16*) dest->GetAddr(destOffset);
      PrimeAssert(pd, "Could not get destination pixel address.");
      destOffset += sizeof(u16);

      u32 baseColor = *pd;
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
      s32 baseColorA = baseColor & 0xF;
#else
      s32 baseColorA = (baseColor >> 12) & 0xF;
#endif
      s32 value = *ps8++;
      if(baseColorA == 0) {
        if(value > 0) {
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
          *pd = color | (value >> 4);
#else
          *pd = color | ((value >> 4) << 12);
#endif
        }
      }
      else {
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
        s32 baseColorR = (baseColor >> 12) & 0xF;
        s32 baseColorG = (baseColor >> 8) & 0xF;
        s32 baseColorB = (baseColor >> 4) & 0xF;
#else
        s32 baseColorR = baseColor & 0xF;
        s32 baseColorG = (baseColor >> 4) & 0xF;
        s32 baseColorB = (baseColor >> 8) & 0xF;
#endif
        f32 aValue = value / 15.0f;
        f32 aValueAlpha = aValue * a;
        s32 aValueAlphaInt = clamp((s32) roundf(aValueAlpha * 15.0f), 0, 15);

        s32 colorR = (s32) (baseColorR * (1.0f - aValueAlpha) + inputColorR * aValueAlpha);
        s32 colorG = (s32) (baseColorG * (1.0f - aValueAlpha) + inputColorG * aValueAlpha);
        s32 colorB = (s32) (baseColorB * (1.0f - aValueAlpha) + inputColorB * aValueAlpha);
        s32 colorA = (s32) max(baseColorA, aValueAlphaInt);

        colorR = clamp(colorR, 0, 15);
        colorG = clamp(colorG, 0, 15);
        colorB = clamp(colorB, 0, 15);
        colorA = clamp(colorA, 0, 15);

#ifdef FONT_PIXEL_16_REVERSE_FORMAT
        *pd = (colorR << 12) | (colorG << 8) | (colorB << 4) | colorA;
#else
        *pd = colorR | (colorG << 4) | (colorB << 8) | (colorA << 12);
#endif
      }
    }

    ps += stride;
    gradientValue += gradientRate;
  }
}

void FontContent::CopyGlyph32(u8* src, size_t x, size_t y, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destStride, f32 r, f32 g, f32 b, f32 a, f32 r2, f32 g2, f32 b2, f32 a2, f32 r3, f32 g3, f32 b3, f32 a3, f32 gradient, f32 gradientTop, f32 gradientBottom, f32 gradientStart, f32 gradientRate) {
  u8* ps = src + (x + y * stride);
  s32 rb = clamp((s32) (r * 255), 0, 255);
  s32 gb = clamp((s32) (g * 255), 0, 255);
  s32 bb = clamp((s32) (b * 255), 0, 255);
  f32 useGradient = clamp(gradient, 0.0f, 1.0f);
  f32 useGradientTop = clamp(gradientTop, 0.0f, 1.0f);
  f32 useGradientBottom = clamp(gradientBottom, 0.0f, 1.0f);
  f32 gradientValue = gradientStart;

  for(size_t j = 0; j < h; j++) {
    u8* ps8 = (u8*) ps;
    size_t destOffset = x * sizeof(u32) + (y + j) * destStride;

    s32 inputColorR;
    s32 inputColorG;
    s32 inputColorB;

    if(gradient == -1.0f) {
      inputColorR = (s32) (r * 255);
      inputColorG = (s32) (g * 255);
      inputColorB = (s32) (b * 255);
    }
    else if(useGradient == 0.0f) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradientBottom - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r * t + r2 * omt) * 255);
      inputColorG = (s32) ((g * t + g2 * omt) * 255);
      inputColorB = (s32) ((b * t + b2 * omt) * 255);
    }
    else if(useGradient == 1.0f) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradientBottom - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r2 * t + r3 * omt) * 255);
      inputColorG = (s32) ((g2 * t + g3 * omt) * 255);
      inputColorB = (s32) ((b2 * t + b3 * omt) * 255);
    }
    else if(gradientValue < useGradient) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradient - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r * t + r2 * omt) * 255);
      inputColorG = (s32) ((g * t + g2 * omt) * 255);
      inputColorB = (s32) ((b * t + b2 * omt) * 255);
    }
    else {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradient) / (useGradientBottom - useGradient);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r2 * t + r3 * omt) * 255);
      inputColorG = (s32) ((g2 * t + g3 * omt) * 255);
      inputColorB = (s32) ((b2 * t + b3 * omt) * 255);
    }

    inputColorR = clamp(inputColorR, 0, 255);
    inputColorG = clamp(inputColorG, 0, 255);
    inputColorB = clamp(inputColorB, 0, 255);
    u32 color = inputColorR | (inputColorG << 8) | (inputColorB << 16);

    for(size_t i = 0; i < w; i++) {
      u32* pd = (u32*) dest->GetAddr(destOffset);
      PrimeAssert(pd, "Could not get destination pixel address.");
      destOffset += sizeof(u32);

      s32 value = *ps8++;
      if(a == 1.0f) {
        *pd = color | (value << 24);
      }
      else {
        f32 aValue = value * a;
        s32 aInt = (s32) roundf(aValue);
        aInt = clamp(aInt, 0, 255);
        *pd = color | (aInt << 24);
      }
    }

    ps += stride;
    gradientValue += gradientRate;
  }
}

void FontContent::BlendGlyph32(u8* src, size_t x, size_t y, size_t w, size_t h, size_t stride, BlockBuffer* dest, size_t destStride, f32 r, f32 g, f32 b, f32 a, f32 r2, f32 g2, f32 b2, f32 a2, f32 r3, f32 g3, f32 b3, f32 a3, f32 gradient, f32 gradientTop, f32 gradientBottom, f32 gradientStart, f32 gradientRate) {
  u8* ps = src + (x + y * stride);
  s32 rb = clamp((s32) (r * 255), 0, 255);
  s32 gb = clamp((s32) (g * 255), 0, 255);
  s32 bb = clamp((s32) (b * 255), 0, 255);
  f32 useGradient = clamp(gradient, 0.0f, 1.0f);
  f32 useGradientTop = clamp(gradientTop, 0.0f, 1.0f);
  f32 useGradientBottom = clamp(gradientBottom, 0.0f, 1.0f);
  f32 gradientValue = gradientStart;

  for(size_t j = 0; j < h; j++) {
    u8* ps8 = (u8*) ps;
    size_t destOffset = x * sizeof(u32) + (y + j) * destStride;

    s32 inputColorR;
    s32 inputColorG;
    s32 inputColorB;

    if(gradient == -1.0f) {
      inputColorR = (s32) (r * 255);
      inputColorG = (s32) (g * 255);
      inputColorB = (s32) (b * 255);
    }
    else if(useGradient == 0.0f) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradientBottom - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r * t + r2 * omt) * 255);
      inputColorG = (s32) ((g * t + g2 * omt) * 255);
      inputColorB = (s32) ((b * t + b2 * omt) * 255);
    }
    else if(useGradient == 1.0f) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradientBottom - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r2 * t + r3 * omt) * 255);
      inputColorG = (s32) ((g2 * t + g3 * omt) * 255);
      inputColorB = (s32) ((b2 * t + b3 * omt) * 255);
    }
    else if(gradientValue < useGradient) {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradientTop) / (useGradient - useGradientTop);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r * t + r2 * omt) * 255);
      inputColorG = (s32) ((g * t + g2 * omt) * 255);
      inputColorB = (s32) ((b * t + b2 * omt) * 255);
    }
    else {
      f32 t, omt;
      if(gradientValue < useGradientTop || useGradientTop >= useGradientBottom) {
        omt = 0.0f;
      }
      else if(gradientValue > useGradientBottom) {
        omt = 1.0f;
      }
      else {
        omt = (gradientValue - useGradient) / (useGradientBottom - useGradient);
      }
      t = 1.0f - omt;
      inputColorR = (s32) ((r2 * t + r3 * omt) * 255);
      inputColorG = (s32) ((g2 * t + g3 * omt) * 255);
      inputColorB = (s32) ((b2 * t + b3 * omt) * 255);
    }

    inputColorR = clamp(inputColorR, 0, 255);
    inputColorG = clamp(inputColorG, 0, 255);
    inputColorB = clamp(inputColorB, 0, 255);
    u32 color = inputColorR | (inputColorG << 8) | (inputColorB << 16);

    for(size_t i = 0; i < w; i++) {
      u32* pd = (u32*) dest->GetAddr(destOffset);
      PrimeAssert(pd, "Could not get destination pixel address.");
      destOffset += sizeof(u32);

      u32 baseColor = *pd;
      s32 baseColorA = (baseColor >> 24) & 0xFF;
      s32 value = *ps8++;
      if(baseColorA == 0) {
        if(value > 0) {
          *pd = color | (value << 24);
        }
      }
      else {
        s32 baseColorR = baseColor & 0xFF;
        s32 baseColorG = (baseColor >> 8) & 0xFF;
        s32 baseColorB = (baseColor >> 16) & 0xFF;
        f32 aValue = value / 255.0f;
        f32 aValueAlpha = aValue * a;
        s32 aValueAlphaInt = clamp((s32) roundf(aValueAlpha * 255.0f), 0, 255);

        s32 colorR = (s32) (baseColorR * (1.0f - aValueAlpha) + inputColorR * aValueAlpha);
        s32 colorG = (s32) (baseColorG * (1.0f - aValueAlpha) + inputColorG * aValueAlpha);
        s32 colorB = (s32) (baseColorB * (1.0f - aValueAlpha) + inputColorB * aValueAlpha);
        s32 colorA = max(baseColorA, aValueAlphaInt);

        colorR = clamp(colorR, 0, 255);
        colorG = clamp(colorG, 0, 255);
        colorB = clamp(colorB, 0, 255);
        colorA = clamp(colorA, 0, 255);

        *pd = colorR | (colorG << 8) | (colorB << 16) | (colorA << 24);
      }
    }

    ps += stride;
    gradientValue += gradientRate;
  }
}

BlockBuffer* FontContent::ConvertFrom32To16(BlockBuffer* src, size_t w, size_t h, size_t activeH) {
  BlockBuffer* result = new BlockBuffer(0, w * h * sizeof(u16), sizeof(u16));
  if(result) {
    u32 srcOffset = 0;
    u32 destOffset = 0;

    for(size_t y = 0; y < activeH; y++) {
      for(size_t x = 0; x < w; x++) {
        u8* s = (u8*) src->GetAddr(srcOffset);
        PrimeAssert(s, "Could not get source pixel address.");
        srcOffset += sizeof(u32);

        u16* d = (u16*) result->GetAddr(destOffset);
        PrimeAssert(d, "Could not get destination pixel address.");
        destOffset += sizeof(u16);

        u8 r = *s++;
        u8 g = *s++;
        u8 b = *s++;
        u8 a = *s++;
#ifdef FONT_PIXEL_16_REVERSE_FORMAT
        *d = ((r >> 4) << 12) | ((g >> 4) << 8) | ((b >> 4) << 4) | (a >> 4);
#else
        *d = (r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8) | ((a >> 4) << 12);
#endif
      }
    }
  }

  return result;  
}
