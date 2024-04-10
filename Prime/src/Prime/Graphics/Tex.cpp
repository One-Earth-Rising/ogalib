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

#include <Prime/Graphics/Tex.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>
#include <png/png.h>
#include <png/pngstruct.h>
#include <png/pnginfo.h>
#include <jpeg/jpeglib.h>
#include <jpeg/jerror.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

typedef struct _PNGDataRead {
  const u8* data;
  size_t dataSize;
  size_t pos;
} PNGDataRead;

struct jpegErrorManager {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

static char jpegLastErrorMsg[JMSG_LENGTH_MAX];
static void jpegErrorExit(j_common_ptr cinfo);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

static void ReadPNGData(png_structp png, png_bytep data, png_size_t size);

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

const char* const Tex::MutableFormatKey = "__TexMutableFormatKey__";
const char* const Tex::RenderBufferFormatKey = "__RenderBufferFormatKey__";

static const bool TexFormatHasRTable[] = {
  false,  // TexFormatNone
  true,   // TexFormatNative
  true,   // TexFormatR8G8B8A8
  true,   // TexFormatR8G8B8
  true,   // TexFormatR8G8
  true,   // TexFormatR8
  true,   // TexFormatR5G6B5
  true,   // TexFormatR5G5B5A1
  true,   // TexFormatR4G4B4A4
  true,   // TexFormatFPBuffer
  true,   // TexFormatFPBufferHQ
  true,   // TexFormatFPBufferNoAlpha
  true,   // TexFormatFPBufferNoAlphaHQ
  true,   // TexFormatDepthBuffer
  false,  // TexFormatShadowMap
  true,   // TexFormatPositionBuffer
  true,   // TexFormatNormalBuffer
  true,   // TexFormatGlowBuffer
  true,   // TexFormatSpecularBuffer
};

static const bool TexFormatHasGTable[] = {
  false,  // TexFormatNone
  true,   // TexFormatNative
  true,   // TexFormatR8G8B8A8
  true,   // TexFormatR8G8B8
  true,   // TexFormatR8G8
  false,  // TexFormatR8
  true,   // TexFormatR5G6B5
  true,   // TexFormatR5G5B5A1
  true,   // TexFormatR4G4B4A4
  true,   // TexFormatFPBuffer
  true,   // TexFormatFPBufferHQ
  true,   // TexFormatFPBufferNoAlpha
  true,   // TexFormatFPBufferNoAlphaHQ
  false,  // TexFormatDepthBuffer
  false,  // TexFormatShadowMap
  true,   // TexFormatPositionBuffer
  true,   // TexFormatNormalBuffer
  false,  // TexFormatGlowBuffer
  true,   // TexFormatSpecularBuffer
};

static const bool TexFormatHasBTable[] = {
  false,  // TexFormatNone
  true,   // TexFormatNative
  true,   // TexFormatR8G8B8A8
  true,   // TexFormatR8G8B8
  false,  // TexFormatR8G8
  false,  // TexFormatR8
  true,   // TexFormatR5G6B5
  true,   // TexFormatR5G5B5A1
  true,   // TexFormatR4G4B4A4
  true,   // TexFormatFPBuffer
  true,   // TexFormatFPBufferHQ
  true,   // TexFormatFPBufferNoAlpha
  true,   // TexFormatFPBufferNoAlphaHQ
  false,  // TexFormatDepthBuffer
  false,  // TexFormatShadowMap
  true,   // TexFormatPositionBuffer
  true,   // TexFormatNormalBuffer
  false,  // TexFormatGlowBuffer
  false,  // TexFormatSpecularBuffer
};

static const bool TexFormatHasATable[] = {
  false,  // TexFormatNone
  true,   // TexFormatNative
  true,   // TexFormatR8G8B8A8
  false,  // TexFormatR8G8B8
  false,  // TexFormatR8G8
  false,  // TexFormatR8
  false,  // TexFormatR5G6B5
  true,   // TexFormatR5G5B5A1
  true,   // TexFormatR4G4B4A4
  true,   // TexFormatFPBuffer
  true,   // TexFormatFPBufferHQ
  false,  // TexFormatFPBufferNoAlpha
  false,  // TexFormatFPBufferNoAlphaHQ
  false,  // TexFormatShadowMap
  false,  // TexFormatDepthBuffer
  false,  // TexFormatPositionBuffer
  false,  // TexFormatNormalBuffer
  false,  // TexFormatGlowBuffer
  false,  // TexFormatSpecularBuffer
};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Tex::Tex(u32 w, u32 h, TexFormat format, const void* pixels, const json& options):
filteringEnabled(true),
wrapModeX(WrapMode()),
wrapModeY(WrapMode()),
renderBufferTexFormat(TexFormatNone),
renderBufferW(0),
renderBufferH(0),
renderBufferTW(0),
renderBufferTH(0),
renderBufferNeedsDepth(true),
loadedLevelCount(0),
loadedIntoVRAM(false),
hasR(false),
hasG(false),
hasB(false),
hasA(false) {
  PrimeAssert(w > 0 && h > 0, "Invalid tex data size.");

  bool optionsRenderBuffer = true;

  if(auto it = options.find("RenderBuffer")) {
    optionsRenderBuffer = it.GetBool();
  }

  if(auto it = options.find("RenderBufferNeedsDepth")) {
    renderBufferNeedsDepth = it.GetBool();
  }

  if(optionsRenderBuffer) {
    TexData* texData = GetTexDataInternal("");
    if(texData) {
      renderBufferTexFormat = format;
      texData->format = TexFormatNative;
      texData->formatName = RenderBufferFormatKey;
      texData->w = w;
      texData->h = h;
      texData->tw = (u32) GetNextPowerOf2(texData->w);
      texData->th = (u32) GetNextPowerOf2(texData->h);
      texData->mu = w / (f32) texData->tw;
      texData->mv = h / (f32) texData->th;
    }
  }
  else {
    TexData* texData = GetTexDataInternal("");
    if(texData) {
      texData->format = TexFormatNative;
      texData->formatName = MutableFormatKey;
      texData->w = w;
      texData->h = h;
      texData->tw = (u32) GetNextPowerOf2(texData->w);
      texData->th = (u32) GetNextPowerOf2(texData->h);
      texData->mu = w / (f32) texData->tw;
      texData->mv = h / (f32) texData->th;
      if(AllocateMutableFormat(*texData)) {
        if(pixels) {
          SetPixels(0, pixels);
        }
        else if(auto it = options.find("Pixels")) {
          SetPixels(0, it.GetVoidPtr());
        }
        else if(auto it = options.find("PixelsAsBlockBuffer")) {
          SetPixelsFromBlockBuffer(0, static_cast<const BlockBuffer*>(it.GetVoidPtr()));
        }
      }
      else {
        *texData = 0;
      }
    }
  }
}

Tex::Tex(u32 w, u32 h, TexFormat format, const json& options): Tex(w, h, format, nullptr, options) {

}

Tex::Tex(u32 w, u32 h, TexFormat format, const void* pixels): Tex(w, h, format, pixels, json()) {

}

Tex::Tex():
filteringEnabled(true),
wrapModeX(WrapMode()),
wrapModeY(WrapMode()),
renderBufferTexFormat(TexFormatNone),
renderBufferW(0),
renderBufferH(0),
renderBufferTW(0),
renderBufferTH(0),
renderBufferNeedsDepth(true),
loadedLevelCount(0),
loadedIntoVRAM(false),
hasR(false),
hasG(false),
hasB(false),
hasA(false) {

}

Tex::Tex(const std::string& name, const std::string& data):
filteringEnabled(true),
wrapModeX(WrapMode()),
wrapModeY(WrapMode()),
renderBufferTexFormat(TexFormatNone),
renderBufferW(0),
renderBufferH(0),
renderBufferTW(0),
renderBufferTH(0),
renderBufferNeedsDepth(true),
loadedLevelCount(0),
loadedIntoVRAM(false),
hasR(false),
hasG(false),
hasB(false),
hasA(false) {
  AddTexData(name, data);
}

Tex::~Tex() {
  for(auto it: texDataLookup) {
    TexData* texData = it.value();
    PrimeSafeDelete(texData);
  }
}

bool Tex::LoadIntoVRAM() {
  return false;
}

bool Tex::UnloadFromVRAM() {
  loadedLevelCount = 0;
  return true;
}

void Tex::SetFilteringEnabled(bool enabled) {
  filteringEnabled = enabled;
}

void Tex::SetWrapModeX(WrapMode wrapModeX) {
  this->wrapModeX = wrapModeX;
}

void Tex::SetWrapModeY(WrapMode wrapModeY) {
  this->wrapModeY = wrapModeY;
}

void Tex::AddTexData(const std::string& name, const std::string& data) {
  AddTexData(name, data, json());
}

void Tex::AddTexData(const std::string& name, const std::string& data, const json& info) {
  PxRequireMainThread;

  IncRef();

  new Job([=](Job& job) {
    std::string format;
    if(auto itFormat = info.find("format")) {
      format = itFormat.GetString();

      if(format == "bc") {
        if(auto itSubFormat = info.find("subFormat")) {
          std::string subFormat = itSubFormat.GetString();

          u32 w = 0;
          u32 h = 0;

          if(auto itWidth = info.find("width")) {
            w = itWidth.GetUint();
          }
          else if(auto itWidth = info.find("w")) {
            w = itWidth.GetUint();
          }

          if(auto itHeight = info.find("height")) {
            h = itHeight.GetUint();
          }
          else if(auto itHeight = info.find("h")) {
            h = itHeight.GetUint();
          }

          if(w > 0 && h > 0) {
            size_t blockSize;
            bool supported = true;

            if(subFormat == "bc1") {
              blockSize = w >> 1;
            }
            else if(subFormat == "bc3") {
              blockSize = w;
            }
            else {
              blockSize = 0;
              supported = false;
            }

            if(supported) {
              TexData* texData = new TexData();
              if(texData) {
                BlockBuffer* pixels = nullptr;
                if(auto itPixels = info.find("pixels")) {
                  pixels = itPixels.GetPtr<BlockBuffer>();
                }

                if(pixels) {
                  texData->pixels = pixels;
                }
                else {
                  texData->pixels = new BlockBuffer(blockSize);
                  if(texData->pixels) {
                    texData->pixels->Append(data.c_str(), data.size());
                  }
                }

                if(texData->pixels) {
                  texData->format = TexFormatNative;
                  texData->formatName = subFormat;
                  texData->w = w;
                  texData->h = h;
                  texData->tw = texData->w;
                  texData->th = texData->h;

                  job.data["texData"] = texData;
                }
              }
            }
          }
        }
      }
      else if(format == "raw") {
        if(auto itSubFormat = info.find("subFormat")) {
          TexFormat subFormat;
          std::string formatName;

          if(itSubFormat.IsString()) {
            bool subFormatAsNative = false;
            if(auto itSubFormatAsNative = info.find("subFormatAsNative")) {
              subFormatAsNative = itSubFormatAsNative.GetBool();
            }

            if(subFormatAsNative) {
              subFormat = TexFormatNative;
              formatName = itSubFormat.GetString();
            }
            else {
              subFormat = GetEnumTexFormatFromString(itSubFormat.GetString());
            }
          }
          else {
            subFormat = itSubFormat.GetEnum<TexFormat>();
          }

          u32 w = 0;
          u32 h = 0;

          if(auto itWidth = info.find("width")) {
            w = itWidth.GetUint();
          }
          else if(auto itWidth = info.find("w")) {
            w = itWidth.GetUint();
          }

          if(auto itHeight = info.find("height")) {
            h = itHeight.GetUint();
          }
          else if(auto itHeight = info.find("h")) {
            h = itHeight.GetUint();
          }

          if(w > 0 && h > 0) {
            size_t blockSize;
            bool supported = true;

            switch(subFormat) {
            case TexFormatR8G8B8A8:
              blockSize = w * sizeof(u32);
              break;

            case TexFormatR4G4B4A4:
              blockSize = w * sizeof(u16);
              break;

            case TexFormatNative:
              if(formatName == "R8G8B8A8_sRGB") {
                blockSize = w * 4;
              }
              else if(formatName == "R8G8B8_sRGB") {
                blockSize = w * 3;
              }
              else if(formatName == "R16G16B16A16_sRGB") {
                blockSize = w * 8;
              }
              else if(formatName == "R16G16B16_sRGB") {
                blockSize = w * 6;
              }
              else {
                blockSize = 0;
                supported = false;
              }
              break;

            default:
              blockSize = 0;
              supported = false;
              break;
            }

            if(supported) {
              TexData* texData = new TexData();
              if(texData) {
                BlockBuffer* pixels = nullptr;
                if(auto itPixels = info.find("pixels")) {
                  pixels = itPixels.GetPtr<BlockBuffer>();
                }

                if(pixels) {
                  texData->pixels = pixels;
                }
                else {
                  texData->pixels = new BlockBuffer(blockSize);
                  if(texData->pixels) {
                    texData->pixels->Append(data.c_str(), data.size());
                  }
                }

                if(texData->pixels) {
                  texData->format = subFormat;
                  if(texData->format == TexFormatNative) {
                    texData->formatName = formatName;
                  }
                  texData->w = w;
                  texData->h = h;
                  texData->tw = texData->w;
                  texData->th = texData->h;

                  job.data["texData"] = texData;
                }
              }
            }
          }
        }
      }
    }
    else {
      if(IsFormatPNG(data.c_str(), data.size(), info)) {
        TexData* texData = new TexData();
        if(texData && LoadPixelsFromPNG(data.c_str(), data.size(), *texData)) {
          job.data["texData"] = texData;
        }
      }
      else if(IsFormatJPEG(data.c_str(), data.size(), info)) {
        TexData* texData = new TexData();
        if(texData && LoadPixelsFromJPEG(data.c_str(), data.size(), *texData)) {
          job.data["texData"] = texData;
        }
      }
    }
  }, [=](Job& job) {
    if(auto it = job.data.find("texData")) {
      TexData* tempTexData = it.GetPtr<TexData>();
      if(tempTexData) {
        TexData* texData;
        
        if(auto it = texDataLookup.Find(name)) {
          texData = it.value();
        }
        else {
          texData = new TexData();
          if(texData) {
            texDataLookup[name] = texData;
          }
        }

        if(texData) {
          texData->TakePixels(*tempTexData);
          CacheInfo();
          UnloadFromVRAM();
          LoadIntoVRAM();
        }

        PrimeSafeDelete(tempTexData);
      }
    }

    DecRef();
  });
}

void Tex::AddTexData(const std::string& name, const TexData& data) {
  PxRequireMainThread;

  TexData* texData;

  if(auto it = texDataLookup.Find(name)) {
    texData = it.value();
  }
  else {
    texData = new TexData();
    if(texData) {
      texDataLookup[name] = texData;
    }
  }

  if(texData) {
    *texData = data;
    CacheInfo();
    UnloadFromVRAM();
    LoadIntoVRAM();
  }
}

void Tex::RemoveTexData(const std::string& name) {
  if(auto it = texDataLookup.Find(name)) {
    texDataLookup.Remove(name);

    TexData* texData = it.value();
    if(texData) {
      OnWillDeleteTexData(*texData);
      PrimeSafeDelete(texData);
    }

    CacheInfo();
  }
}

void Tex::RemoveAllTexData() {
  Stack<TexData*> texDataToDelete;

  for(auto it: texDataLookup) {
    TexData* texData = it.value();

    if(texData) {
      texDataToDelete.Add(texData);
    }
  }

  texDataLookup.Clear();

  for(auto texData: texDataToDelete) {
    OnWillDeleteTexData(*texData);
    PrimeSafeDelete(texData);
  }

  CacheInfo();
}

void Tex::GetTexDataAsLevels(Stack<TexDataLevelSortItem>& levels) const {
  for(auto it: texDataLookup) {
    TexData* texData = it.value();
    if(texData && texData->pixels) {
      levels.Add(TexDataLevelSortItem(texData, it.key()));
    }
  }

  levels.Sort();
}

const TexData* Tex::GetTexData(const std::string& name) const {
  if(auto it = texDataLookup.Find(name)) {
    return it.value();
  }

  return nullptr;
}

TexFormat Tex::GetFormat(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return texData->format;
  }

  return TexFormatNone;
}

const std::string& Tex::GetFormatName(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return texData->formatName;
  }

  static const std::string noFormatName;
  return noFormatName;
}

size_t Tex::GetW(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return texData->w;
  }

  return 0;
}

size_t Tex::GetH(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return texData->h;
  }

  return 0;
}

size_t Tex::GetTW(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return texData->tw;
  }

  return 0;
}

size_t Tex::GetTH(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return texData->th;
  }

  return 0;
}

f32 Tex::GetMU(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return texData->mu;
  }

  return 0;
}

f32 Tex::GetMV(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return texData->mv;
  }

  return 0;
}

bool Tex::HasR(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return TexFormatHasRTable[texData->format];
  }

  return false;
}

bool Tex::HasG(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return TexFormatHasGTable[texData->format];
  }

  return false;
}

bool Tex::HasB(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return TexFormatHasBTable[texData->format];
  }

  return false;
}

bool Tex::HasA(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return TexFormatHasATable[texData->format];
  }

  return false;
}

f32 Tex::GetU(const std::string& name, f32 x) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return texData->tw > 0 ? (x / (f32) texData->tw) : 0.0f;
  }

  return 0.0f;
}

f32 Tex::GetV(const std::string& name, f32 y) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    return texData->th > 0 ? (y / (f32) texData->th) : 0.0f;
  }

  return 0.0f;
}

Color Tex::GetPixel(const std::string& name, u32 x, u32 y) const {
  const TexData* texData = GetTexData(name);
  if(texData) {
    void* pixel = GetPixelAddr(name, x, y);
    if(pixel) {
      switch(texData->format) {
      case TexFormatR8G8B8A8: {
        TexPixelR8G8B8A8* p = static_cast<TexPixelR8G8B8A8*>(pixel);
        return Color(p->r / 255.0f, p->g / 255.0f, p->b / 255.0f, p->a / 255.0f);
      }

      case TexFormatR8G8B8: {
        TexPixelR8G8B8* p = static_cast<TexPixelR8G8B8*>(pixel);
        return Color(p->r / 255.0f, p->g / 255.0f, p->b / 255.0f, 1.0f);
      }

      case TexFormatR8G8: {
        TexPixelR8G8* p = static_cast<TexPixelR8G8*>(pixel);
        return Color(p->r / 255.0f, p->g / 255.0f, 0.0f, 1.0f);
      }

      case TexFormatR8: {
        TexPixelR8* p = static_cast<TexPixelR8*>(pixel);
        return Color(p->r / 255.0f, 0.0f, 0.0f, 1.0f);
      }

      case TexFormatR5G6B5: {
        TexPixelR5G6B5* p = static_cast<TexPixelR5G6B5*>(pixel);
        return Color(p->r / 32.0f, p->g / 64.0f, p->b / 32.0f, 1.0f);
      }

      case TexFormatR5G5B5A1: {
        TexPixelR5G5B5A1* p = static_cast<TexPixelR5G5B5A1*>(pixel);
        return Color(p->r / 32.0f, p->g / 32.0f, p->b / 32.0f, p->a);
      }

      case TexFormatR4G4B4A4: {
        TexPixelR4G4B4A4* p = static_cast<TexPixelR4G4B4A4*>(pixel);
        return Color(p->r / 16.0f, p->g / 16.0f, p->b / 16.0f, p->a / 16.0f);
      }

      default:
        break;
      }
    }
  }

  return Color(0.0f, 0.0f, 0.0f, 0.0f);
}

void* Tex::GetPixelAddr(const std::string& name, u32 x, u32 y) const {
  const TexData* texData = GetTexData(name);
  if(texData && texData->pixels) {
    size_t pixelSize = GetPixelSize(texData->format);
    size_t stride = texData->tw * pixelSize;
    return texData->pixels->GetAddr(x * pixelSize + y * stride);
  }

  return nullptr;
}

BlockBuffer* Tex::GetPixels(const std::string& name) const {
  if(auto it = texDataLookup.Find(name)) {
    return it.value()->pixels;
  }

  return nullptr;
}

size_t Tex::GetPixelSize(const std::string& name) const {
  const TexData* texData = GetTexData(name);
  if(texData)
    return GetPixelSize(texData->format);

  return 0;
}

Color Tex::GetSampledColor(const std::string& name, f32 x, f32 y, f32 w, f32 h) const {
  const TexData* texData = GetTexData(name);
  if(!texData)
    return Color(0.0f, 0.0f, 0.0f, 0.0f);

  size_t texW = texData->w;
  size_t texH = texData->h;
  f32 px, py; // the integer part of the pixel position
  f32 fx, fy; // the floating point part of the pixel position
  f32 cx, cy; // remaining pixels left in loop
  f32 nx, ny; // current floating-point pixel position in loop
  u32 ix, iy; // integer part with type u32
  f32 ax, ay; // amount of pixel to contribute
  f32 sr, sg, sb, sa;
  f32 amount;
  f32 fullAmount;

  if(w <= 0.0f || h <= 0.0f || x >= (f32) texW || y >= (f32) texH) {
    return Color(0.0f, 0.0f, 0.0f, 0.0f);
  }

  if(x < 0.0f) {
    w += x;
    x = 0.0f;
  }

  if(y < 0.0f) {
    h += y;
    y = 0.0f;
  }

  if(texW <= 0 || texH <= 0) {
    return Color(0.0f, 0.0f, 0.0f, 0.0f);
  }

  if(x + w >= texW) {
    w -= x + w - texW;
  }

  if(y + h >= texH) {
    h -= y + h - texH;
  }

  if(texW <= 0 || texH <= 0) {
    return Color(0.0f, 0.0f, 0.0f, 0.0f);
  }

  px = floorf(x);
  py = floorf(y);
  ix = (u32) px;
  iy = (u32) py;
  fx = x - px;
  fy = y - py;

  sr = 0;
  sg = 0;
  sb = 0;
  sa = 0;

  fullAmount = 0.0f;
  cy = h;
  ny = y;
  while(cy > 0.0f) {
    cx = w;
    nx = x;
    iy = (u32) ny;

    if(fy > 0.0f)
      ay = 1.0f - fy;
    else
      ay = 1.0f;
    if(ay > cy)
      ay = cy;

    while(cx > 0.0f) {
      ix = (u32) nx;

      if(fx > 0.0f)
        ax = 1.0f - fx;
      else
        ax = 1.0f;
      if(ax > cx)
        ax = cx;

      if(ix < texW && iy < texH) {
        amount = ax * ay;

        void* pixel = GetPixelAddr(name, ix, iy);
        if(pixel) {
          switch(texData->format) {
          case TexFormatR8G8B8A8: {
            TexPixelR8G8B8A8* p = static_cast<TexPixelR8G8B8A8*>(pixel);
            sr += (p->r / 255.0f) * amount;
            sg += (p->g / 255.0f) * amount;
            sb += (p->b / 255.0f) * amount;
            sa += (p->a / 255.0f) * amount;
            break;
          }

          case TexFormatR8G8B8: {
            TexPixelR8G8B8* p = static_cast<TexPixelR8G8B8*>(pixel);
            sr += (p->r / 255.0f) * amount;
            sg += (p->g / 255.0f) * amount;
            sb += (p->b / 255.0f) * amount;
            break;
          }

          case TexFormatR8G8: {
            TexPixelR8G8* p = static_cast<TexPixelR8G8*>(pixel);
            sr += (p->r / 255.0f) * amount;
            sg += (p->g / 255.0f) * amount;
            break;
          }

          case TexFormatR8: {
            TexPixelR8* p = static_cast<TexPixelR8*>(pixel);
            sr += (p->r / 255.0f) * amount;
            break;
          }

          case TexFormatR5G6B5: {
            TexPixelR5G6B5* p = static_cast<TexPixelR5G6B5*>(pixel);
            sr += (p->r / 31.0f) * amount;
            sg += (p->g / 63.0f) * amount;
            sb += (p->b / 31.0f) * amount;
            break;
          }

          case TexFormatR5G5B5A1: {
            TexPixelR5G5B5A1* p = static_cast<TexPixelR5G5B5A1*>(pixel);
            sr += (p->r / 31.0f) * amount;
            sg += (p->g / 31.0f) * amount;
            sb += (p->b / 31.0f) * amount;
            sa += p->a * amount;
            break;
          }

          case TexFormatR4G4B4A4: {
            TexPixelR4G4B4A4* p = static_cast<TexPixelR4G4B4A4*>(pixel);
            sr += (p->r / 15.0f) * amount;
            sg += (p->g / 15.0f) * amount;
            sb += (p->b / 15.0f) * amount;
            sa += (p->a / 15.0f) * amount;
            break;
          }

          default:
            break;
          }
        }

        fullAmount += amount;
      }

      nx += ax;
      cx -= ax;
    }

    ny += ay;
    cy -= ay;
  }

  if(fullAmount > 0.0f) {
    sr /= fullAmount;
    sg /= fullAmount;
    sb /= fullAmount;
    sa /= fullAmount;
  }

  if(sr > 1.0f)
    sr = 1.0f;
  if(sg > 1.0f)
    sg = 1.0f;
  if(sb > 1.0f)
    sb = 1.0f;
  if(sa > 1.0f)
    sa = 1.0f;

  return Color(sr, sg, sb, sa);
}

void Tex::SetPixel(const std::string& name, u32 x, u32 y, f32 r, f32 g, f32 b, f32 a) {
  TexData* texData = GetTexDataInternal(0);
  if(texData && texData->pixels) {
    void* pixel = GetPixelAddr(name, x, y);
    if(pixel) {
      switch(texData->format) {
      case TexFormatR8G8B8A8: {
        TexPixelR8G8B8A8* p = static_cast<TexPixelR8G8B8A8*>(pixel);
        p->r = clamp((s32) (r * 255), 0, 255);
        p->g = clamp((s32) (g * 255), 0, 255);
        p->b = clamp((s32) (b * 255), 0, 255);
        p->a = clamp((s32) (a * 255), 0, 255);
      }

      case TexFormatR8G8B8: {
        TexPixelR8G8B8* p = static_cast<TexPixelR8G8B8*>(pixel);
        p->r = clamp((s32) (r * 255), 0, 255);
        p->g = clamp((s32) (g * 255), 0, 255);
        p->b = clamp((s32) (b * 255), 0, 255);
      }

      case TexFormatR5G6B5: {
        TexPixelR5G6B5* p = static_cast<TexPixelR5G6B5*>(pixel);
        p->r = clamp((s32) (r * 31), 0, 31);
        p->g = clamp((s32) (g * 63), 0, 63);
        p->b = clamp((s32) (b * 31), 0, 31);
      }

      case TexFormatR5G5B5A1: {
        TexPixelR5G5B5A1* p = static_cast<TexPixelR5G5B5A1*>(pixel);
        p->r = clamp((s32) (r * 31), 0, 31);
        p->g = clamp((s32) (g * 31), 0, 31);
        p->b = clamp((s32) (b * 31), 0, 31);
        p->a = a == 0.0f ? 0 : 1;
      }

      case TexFormatR4G4B4A4: {
        TexPixelR4G4B4A4* p = static_cast<TexPixelR4G4B4A4*>(pixel);
        p->r = clamp((s32) (r * 15), 0, 15);
        p->g = clamp((s32) (g * 15), 0, 15);
        p->b = clamp((s32) (b * 15), 0, 15);
        p->a = clamp((s32) (b * 15), 0, 15);
      }

      default:
        break;
      }
    }
  }
}

void Tex::SetPixel(const std::string& name, u32 x, u32 y, const Color& color) {
  SetPixel(0, x, y, color.r, color.g, color.b, color.a);
}

void Tex::SetPixels(const std::string& name, const void* pixels) {
  TexData* texData = GetTexDataInternal(name);
  if(!texData)
    return;

  BlockBuffer* destPixels = texData->pixels;

  if(pixels) {
    u8* s = (u8*) pixels;
    size_t pixelSize = GetPixelSize(texData->format);
    size_t w = texData->w;
    size_t h = texData->h;
    size_t stride = texData->tw * pixelSize;
    size_t rowSize = w * pixelSize;
    size_t destOffset = 0;

    for(size_t y = 0; y < h; y++) {
      void* d = destPixels->GetAddr(destOffset);
      memcpy(d, s, stride);
      destOffset += stride;
      s += rowSize;
    }
  }
  else {
    destPixels->SetValue(0, 0, destPixels->GetSize());
  }
}

void Tex::SetPixelsFromBlockBuffer(const std::string& name, const BlockBuffer* pixels) {
  TexData* texData = GetTexDataInternal(name);
  if(!texData)
    return;

  PrimeSafeDelete(texData->pixels);

  if(pixels) {
    if(AllocateMutableFormat(*texData)) {
      if(texData->pixels && texData->pixels->GetSize() == pixels->GetSize()) {
        if(texData->pixels->CanDirectCopy(*pixels)) {
          *texData->pixels = *pixels;
        }
        else {
          size_t stride = texData->tw * GetPixelSize(texData->format);
          size_t destOffset = 0;
          for(size_t y = 0; y < texData->th; y++) {
            void* p = texData->pixels->GetAddr(destOffset);
            pixels->CopyTo(p, destOffset, stride);
            destOffset += stride;
          }
        }
      }
    }

    texData->pixels = new BlockBuffer(*pixels);
  }
}

bool Tex::HasR() const {
  return hasR;
}

bool Tex::HasG() const {
  return hasG;
}

bool Tex::HasB() const {
  return hasB;
}

bool Tex::HasA() const {
  return hasA;
}

bool Tex::LoadPixelsFromPNG(const void* data, size_t dataSize, TexData& texData) {
  png_structp png;
  png_infop pngInfo;
  PNGDataRead dr;

  if(dataSize < 8 || png_sig_cmp((png_bytep) data, 0, 8)) {
    texData = 0;
    return false;
  }

  memset(&dr, 0, sizeof(dr));
  dr.data = (const u8*) data;
  dr.dataSize = dataSize;
  dr.pos = 0;

  if(png_sig_cmp((png_bytep) data, 0, 8)) {
    texData = 0;
    dbgprintf("[Warning] Buffer does not contain valid PNG data.");
    return false;
  }

  dr.pos += 8;

  png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if(!png) {
    texData = 0;
    dbgprintf("[Warning] Could not create PNG read struct.");
    return false;
  }

  pngInfo = png_create_info_struct(png);
  if(!pngInfo) {
    texData = 0;
    png_destroy_read_struct(&png, nullptr, nullptr);
    dbgprintf("[Warning] Could not craete PNG info struct.");
    return false;
  }

  png_set_read_fn(png, &dr, ReadPNGData);
  png_set_sig_bytes(png, 8);
  png_set_option(png, PNG_SKIP_sRGB_CHECK_PROFILE, PNG_OPTION_OFF);
  png_read_info(png, pngInfo);

  texData.w = pngInfo->width;
  texData.h = pngInfo->height;

  if(texData.w == 0 || texData.h == 0) {
    texData = 0;
    png_destroy_read_struct(&png, &pngInfo, nullptr);
    return false;
  }

  png_byte colorType = pngInfo->color_type;
  png_byte bitDepth = pngInfo->bit_depth;
  u32 channelSize = (bitDepth >> 3) + ((bitDepth & 0x7) != 0 ? 1 : 0);

  bool hasR = false;
  bool hasG = false;
  bool hasB = false;
  bool hasA = (colorType & PNG_COLOR_MASK_ALPHA) != 0;

  if(colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
    hasR = true;
  }
  else {
    hasR = true;
    hasG = true;
    hasB = true;
  }

  u32 pixelSize = 0;
  if(hasR)
    pixelSize++;
  if(hasG)
    pixelSize++;
  if(hasB)
    pixelSize++;
  if(hasA)
    pixelSize++;

  if(pixelSize == 4) {
    texData.format = TexFormatNative;
    texData.formatName = "R8G8B8A8_sRGB";
  }
  else if(pixelSize == 3) {
    texData.format = TexFormatNative;
    texData.formatName = "R8G8B8_sRGB";
  }
  else if(pixelSize == 2) {
    texData.format = TexFormatR8G8;
  }
  else if(pixelSize == 1) {
    texData.format = TexFormatR8;
  }
  else {
    texData = 0;
    png_destroy_read_struct(&png, &pngInfo, nullptr);
    dbgprintf("[Warning] Unsupported pixel size for texture format.");
    return false;
  }

  texData.tw = (u32) GetNextPowerOf2(texData.w);
  texData.th = (u32) GetNextPowerOf2(texData.h);
  texData.mu = texData.tw > 0 ? (texData.w / (f32) texData.tw) : 0.0f;
  texData.mv = texData.th > 0 ? (texData.h / (f32) texData.th) : 0.0f;

  size_t dataStride = texData.tw * pixelSize;
  size_t pixelsSize = texData.th * dataStride;
  texData.pixels = new BlockBuffer(dataStride, pixelsSize);

  int passCount = png_set_interlace_handling(png);
  png_read_update_info(png, pngInfo);

  png_bytep* rows = (png_bytep*) malloc(sizeof(png_bytep) * texData.h);
  if(!rows) {
    texData = 0;
    png_destroy_read_struct(&png, &pngInfo, nullptr);
    dbgprintf("Error reading PNG data.\n");
    return false;
  }

  for(u32 y = 0; y < texData.h; y++) {
    void* addr = texData.pixels->GetAddr(dataStride * y);
    if(addr) {
      rows[y] = (png_byte*) addr;
    }
    else {
      texData = 0;
      png_destroy_read_struct(&png, &pngInfo, nullptr);
      dbgprintf("[Warning] Could not locate pixel destination for row %d.", y);
      return false;
    }
  }

  png_read_image(png, rows);
  png_destroy_read_struct(&png, &pngInfo, nullptr);
  free(rows);

  return true;
}

bool Tex::LoadPixelsFromJPEG(const void* data, size_t dataSize, TexData& texData) {
  LockSetjmpMutex();

  struct jpeg_decompress_struct jpegInfo;
  jpegErrorManager jerr;

  jpegInfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = jpegErrorExit;

  jpeg_create_decompress(&jpegInfo);

  if(setjmp(jerr.setjmp_buffer)) {
    // If we get here, the JPEG code has signaled an error.
    texData = 0;
    jpeg_destroy_decompress(&jpegInfo);
    UnlockSetjmpMutex();
    return false;
  }

  jpeg_mem_src(&jpegInfo, (const unsigned char*) data, dataSize);
  int result = jpeg_read_header(&jpegInfo, FALSE);
  jpeg_start_decompress(&jpegInfo);

  texData.w = jpegInfo.output_width;
  texData.h = jpegInfo.output_height;

  if(texData.w == 0 || texData.h == 0) {
    texData = 0;
    jpeg_destroy_decompress(&jpegInfo);
    UnlockSetjmpMutex();
    return false;
  }

  u32 bitDepth = jpegInfo.data_precision;
  u32 channelSize = (bitDepth >> 3) + ((bitDepth & 0x7) != 0 ? 1 : 0);

  bool hasR = false;
  bool hasG = false;
  bool hasB = false;
  bool hasA = false;

  if(jpegInfo.num_components == 1) {
    hasR = true;
  }
  else if(jpegInfo.num_components == 2) {
    hasR = true;
    hasG = true;
  }
  else if(jpegInfo.num_components == 3) {
    hasR = true;
    hasG = true;
    hasB = true;
  }
  else {
    hasR = true;
    hasG = true;
    hasB = true;
    hasA = true;
  }

  u32 pixelSize = 0;
  if(hasR)
    pixelSize++;
  if(hasG)
    pixelSize++;
  if(hasB)
    pixelSize++;
  if(hasA)
    pixelSize++;

  if(pixelSize == 4) {
    texData.format = TexFormatNative;
    texData.formatName = "R8G8B8A8_sRGB";
  }
  else if(pixelSize == 3) {
    texData.format = TexFormatNative;
    texData.formatName = "R8G8B8_sRGB";
  }
  else if(pixelSize == 2) {
    texData.format = TexFormatR8G8;
  }
  else if(pixelSize == 1) {
    texData.format = TexFormatR8;
  }
  else {
    texData = 0;
    jpeg_destroy_decompress(&jpegInfo);
    dbgprintf("[Warning] Unsupported pixel size for texture format.");
    UnlockSetjmpMutex();
    return false;
  }

  texData.tw = (u32) GetNextPowerOf2(texData.w);
  texData.th = (u32) GetNextPowerOf2(texData.h);
  texData.mu = texData.tw > 0 ? (texData.w / (f32) texData.tw) : 0.0f;
  texData.mv = texData.th > 0 ? (texData.h / (f32) texData.th) : 0.0f;

  size_t dataStride = texData.tw * pixelSize;
  size_t pixelsSize = texData.th * dataStride;
  texData.pixels = new BlockBuffer(dataStride, pixelsSize);

  while(jpegInfo.output_scanline < jpegInfo.output_height) {
    void* row = texData.pixels->GetAddr(jpegInfo.output_scanline * dataStride);
    if(row) {
      u8* rows[1] = {(u8*) row};
      jpeg_read_scanlines(&jpegInfo, rows, 1);
    }
  }

  jpeg_destroy_decompress(&jpegInfo);

  UnlockSetjmpMutex();

  return true;
}

size_t Tex::GetPixelSize(TexFormat format) {
  switch(format) {
  case TexFormatR8G8B8A8:
    return sizeof(TexPixelR8G8B8A8);

  case TexFormatR8G8B8:
    return sizeof(TexPixelR8G8B8);

  case TexFormatR8G8:
    return sizeof(TexPixelR8G8);

  case TexFormatR8:
    return sizeof(TexPixelR8);

  case TexFormatR5G6B5:
    return sizeof(TexPixelR5G6B5);

  case TexFormatR5G5B5A1:
    return sizeof(TexPixelR5G5B5A1);

  case TexFormatR4G4B4A4:
    return sizeof(TexPixelR4G4B4A4);

  default:
    PrimeAssert(false, "Unknown tex format: %d", format);
    return 0;
  }
}

TexData* Tex::GetTexDataInternal(const std::string& name) {
  if(auto it = texDataLookup.Find(name)) {
    return it.value();
  }

  return nullptr;
}

void Tex::OnWillDeleteTexData(TexData& texData) {

}

bool Tex::AllocateMutableFormat(TexData& texData) {
  bool result = true;

  PrimeSafeDelete(texData.pixels);

  switch(texData.format) {
  case TexFormatR8G8B8A8: {
    size_t stride = sizeof(TexPixelR8G8B8A8) * texData.tw;
    texData.pixels = new BlockBuffer(stride, texData.th * stride);
    break;
  }

  case TexFormatR8G8B8: {
    size_t stride = sizeof(TexPixelR8G8B8) * texData.tw;
    texData.pixels = new BlockBuffer(stride, texData.th * stride);
    break;
  }

  case TexFormatR5G6B5: {
    size_t stride = sizeof(TexPixelR5G6B5) * texData.tw;
    texData.pixels = new BlockBuffer(stride, texData.th * stride);
    break;
  }

  case TexFormatR5G5B5A1: {
    size_t stride = sizeof(TexPixelR5G5B5A1) * texData.tw;
    texData.pixels = new BlockBuffer(stride, texData.th * stride);
    break;
  }

  case TexFormatR4G4B4A4: {
    size_t stride = sizeof(TexPixelR4G4B4A4) * texData.tw;
    texData.pixels = new BlockBuffer(stride, texData.th * stride);
    break;
  }

  default:
    result = false;
    break;
  }

  return result;
}

void Tex::CacheInfo() {
  hasR = false;
  hasG = false;
  hasB = false;
  hasA = false;

  for(auto it: texDataLookup) {
    const std::string& name = it.key();

    if(!hasR)
      hasR = HasR(name);

    if(!hasG)
      hasG = HasG(name);

    if(!hasB)
      hasB = HasB(name);

    if(!hasA)
      hasA = HasA(name);

    if(hasR && hasG && hasB && hasA) {
      break;
    }
  }
}

void ReadPNGData(png_structp png, png_bytep data, png_size_t size) {
  PNGDataRead* dr = (PNGDataRead*) png_get_io_ptr(png);
  size_t readSize;

  if(dr->pos + size > dr->dataSize) {
    readSize = dr->dataSize - dr->pos;
  }
  else {
    readSize = size;
  }

  if(readSize > 0) {
    memcpy(data, &dr->data[dr->pos], readSize);
  }

  dr->pos += readSize;
}

static void jpegErrorExit(j_common_ptr cinfo) {
  jpegErrorManager* myerr = (jpegErrorManager*) cinfo->err;
  (*(cinfo->err->format_message)) (cinfo, jpegLastErrorMsg);
  longjmp(myerr->setjmp_buffer, 1);
}
