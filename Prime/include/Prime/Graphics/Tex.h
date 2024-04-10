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

#include <Prime/System/RefObject.h>
#include <Prime/System/BlockBuffer.h>
#include <Prime/Types/Color.h>
#include <Prime/Types/Stack.h>
#include <Prime/Types/Dictionary.h>
#include <Prime/Enum/TexFormat.h>
#include <Prime/Enum/WrapMode.h>

////////////////////////////////////////////////////////////////////////////////
// Enums
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef enum {
  TexChannelMain = 0,
  TexChannelDepth,
  TexChannel_Count,
} TexChannel;

};

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _TexPixelR8G8B8A8 {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
} TexPixelR8G8B8A8;

typedef struct _TexPixelR8G8B8 {
  u8 r;
  u8 g;
  u8 b;
} TexPixelR8G8B8;

typedef struct _TexPixelR8G8 {
  u8 r;
  u8 g;
} TexPixelR8G8;

typedef struct _TexPixelR8 {
  u8 r;
} TexPixelR8;

typedef struct _TexPixelR5G6B5 {
  u16 r:5;
  u16 g:6;
  u16 b:5;
} TexPixelR5G6B5;

typedef struct _TexPixelR5G5B5A1 {
  u16 r:5;
  u16 g:5;
  u16 b:5;
  u16 a:1;
} TexPixelR5G5B5A1;

typedef struct _TexPixelR4G4B4A4 {
  u16 r:4;
  u16 g:4;
  u16 b:4;
  u16 a:4;
} TexPixelR4G4B4A4;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Tex;

class TexData {
public:

  BlockBuffer* pixels;
  std::string formatName;
  TexFormat format;
  u32 w;
  u32 h;
  u32 tw;
  u32 th;
  f32 mu;
  f32 mv;

public:

  TexData(): pixels(nullptr), format(TexFormatNone), w(0), h(0), tw(0), th(0), mu(1.0f), mv(1.0f) {}
  TexData(const TexData& other): pixels(nullptr), format(TexFormatNone), w(0), h(0), tw(0), th(0), mu(1.0f), mv(1.0f) {(void) operator=(other);}
  ~TexData() {
    PrimeSafeDelete(pixels);
  }

public:

  TexData& operator=(const TexData& other) {
    PrimeSafeDelete(pixels);

    format = other.format;
    formatName = other.formatName;
    w = other.w;
    h = other.h;
    tw = other.tw;
    th = other.th;
    mu = other.mu;
    mv = other.mv;

    if(other.pixels) {
      pixels = new BlockBuffer(*other.pixels);
      if(!pixels) {
        *this = 0;
      }
    }

    return *this;
  }

  TexData& operator=(s32 value) {
    if(value == 0) {
      PrimeSafeDelete(pixels);
      format = TexFormatNone;
      formatName.clear();
      w = 0;
      h = 0;
      tw = 0;
      th = 0;
      mu = 0.0f;
      mv = 0.0f;
    }

    return *this;
  }

  TexData& TakePixels(TexData& other) {
    PrimeSafeDelete(pixels);

    pixels = other.pixels;
    format = other.format;
    formatName = other.formatName;
    w = other.w;
    h = other.h;
    tw = other.tw;
    th = other.th;
    mu = other.mu;
    mv = other.mv;

    other.pixels = nullptr;
    other = 0;

    return *this;
  }

};

class TexDataLevelSortItem {
public:

  TexData* texData;
  std::string name;

public:

  TexDataLevelSortItem(TexData* texData = nullptr, const std::string& name = std::string()): texData(texData), name(name) {}

  bool operator==(const TexDataLevelSortItem& other) const {
    return texData->tw == other.texData->tw;
  }

  bool operator<(const TexDataLevelSortItem& other) const {
    return texData->tw > other.texData->tw;
  }

};

class TexChannelTuple {
public:

  Tex* tex;
  TexChannel channel;

public:

  TexChannelTuple(Tex* tex = nullptr, TexChannel channel = TexChannelMain): tex(tex), channel(channel) {}
  TexChannelTuple(const TexChannelTuple& other) {(void) operator=(other);}

  TexChannelTuple& operator=(const TexChannelTuple& other) {
    tex = other.tex;
    channel = other.channel;
    return *this;
  }

  TexChannelTuple& operator=(s32 value) {
    if(value == 0) {
      tex = nullptr;
      channel = TexChannel();
    }
    return *this;
  }

  bool operator==(const TexChannelTuple& other) const {
    return tex == other.tex && channel == other.channel;
  }

  bool operator<(const TexChannelTuple& other) const {
    if(tex < other.tex)
      return true;
    else if(tex > other.tex)
      return false;

    return channel < other.channel;
  }

};

class TexUnitTuple {
public:

  Tex* tex;
  u32 unit;
  TexChannel channel;

public:

  TexUnitTuple(Tex* tex = nullptr, u32 unit = 0, TexChannel channel = TexChannelMain): tex(tex), unit(unit), channel(channel) {}
  TexUnitTuple(const TexUnitTuple& other) {(void) operator=(other);}

  TexUnitTuple& operator=(const TexUnitTuple& other) {
    tex = other.tex;
    unit = other.unit;
    channel = other.channel;
    return *this;
  }

  TexUnitTuple& operator=(s32 value) {
    if(value == 0) {
      tex = nullptr;
      unit = 0;
      channel = TexChannel();
    }
    return *this;
  }

  bool operator==(const TexUnitTuple& other) const {
    return tex == other.tex && unit == other.unit && channel == other.channel;
  }

  bool operator<(const TexUnitTuple& other) const {
    if(tex < other.tex)
      return true;
    else if(tex > other.tex)
      return false;

    if(unit < other.unit)
      return true;
    else if(unit > other.unit)
      return false;

    return channel < other.channel;
  }

};

};

#if defined(__cplusplus) && !defined(__INTELLISENSE__)
namespace std {
template<> struct hash<Prime::TexChannelTuple> {
  size_t operator()(const Prime::TexChannelTuple& v) const noexcept {
    return hash<void*>()(v.tex) ^ hash<s64>()(v.channel);
  }
};
};

namespace std {
template<> struct hash<Prime::TexUnitTuple> {
  size_t operator()(const Prime::TexUnitTuple& v) const noexcept {
    return hash<void*>()(v.tex) ^ hash<s64>()(v.unit) ^ hash<s64>()(v.channel);
  }
};
};
#endif

namespace Prime {

class PrimeTexFormat;

class Tex: public RefObject {
friend class Engine;
public:

  static const char* const MutableFormatKey;
  static const char* const RenderBufferFormatKey;

protected:

  Dictionary<std::string, TexData*> texDataLookup;

  bool filteringEnabled;
  WrapMode wrapModeX;
  WrapMode wrapModeY;

  TexFormat renderBufferTexFormat;
  u32 renderBufferW;
  u32 renderBufferH;
  u32 renderBufferTW;
  u32 renderBufferTH;
  bool renderBufferNeedsDepth;

  u32 loadedLevelCount;
  bool loadedIntoVRAM;

  bool hasR;
  bool hasG;
  bool hasB;
  bool hasA;

public:

  bool IsFilteringEnabled() const {return filteringEnabled;}
  WrapMode GetWrapModeX() const {return wrapModeX;}
  WrapMode GetWrapModeY() const {return wrapModeY;}

  u32 GetRenderBufferW() const {return renderBufferW;}
  u32 GetRenderBufferH() const {return renderBufferH;}
  u32 GetRenderBufferTW() const {return renderBufferTW;}
  u32 GetRenderBufferTH() const {return renderBufferTH;}
  bool IsRenderBuffer() const {return renderBufferTexFormat != TexFormatNone;}
  bool GetRenderBufferNeedsDepth() const {return renderBufferNeedsDepth;}

  u32 GetLoadedLevelCount() const {return loadedLevelCount;}
  bool IsLoadedIntoVRAM() const {return loadedIntoVRAM;}

protected:

  Tex(u32 w, u32 h, TexFormat format, const void* pixels, const json& options);
  Tex(u32 w, u32 h, TexFormat format, const json& options);
  Tex(u32 w, u32 h, TexFormat format = TexFormatR8G8B8A8, const void* pixels = nullptr);
  Tex();
  Tex(const std::string& name, const std::string& data);

public:

  ~Tex();

  static Tex* Create(u32 w, u32 h, TexFormat format, const void* pixels, const json& options);
  static Tex* Create(u32 w, u32 h, TexFormat format, const json& options);
  static Tex* Create(u32 w, u32 h, TexFormat format = TexFormatR8G8B8A8, const void* pixels = nullptr);
  static Tex* Create();
  static Tex* Create(const std::string& name, const std::string& data);

public:

  virtual bool LoadIntoVRAM();
  virtual bool UnloadFromVRAM();

  virtual void SetFilteringEnabled(bool enabled);
  virtual void SetWrapModeX(WrapMode wrapModeX);
  virtual void SetWrapModeY(WrapMode wrapModeY);

  virtual void AddTexData(const std::string& name, const std::string& data);
  virtual void AddTexData(const std::string& name, const std::string& data, const json& info);
  virtual void AddTexData(const std::string& name, const TexData& data);
  virtual void RemoveTexData(const std::string& name);
  virtual void RemoveAllTexData();

  virtual void GetTexDataAsLevels(Stack<TexDataLevelSortItem>& levels) const;

  virtual const TexData* GetTexData(const std::string& name) const;
  virtual TexFormat GetFormat(const std::string& name) const;
  virtual const std::string& GetFormatName(const std::string& name) const;
  virtual size_t GetW(const std::string& name) const;
  virtual size_t GetH(const std::string& name) const;
  virtual size_t GetTW(const std::string& name) const;
  virtual size_t GetTH(const std::string& name) const;
  virtual f32 GetMU(const std::string& name) const;
  virtual f32 GetMV(const std::string& name) const;
  virtual bool HasR(const std::string& name) const;
  virtual bool HasG(const std::string& name) const;
  virtual bool HasB(const std::string& name) const;
  virtual bool HasA(const std::string& name) const;
  virtual f32 GetU(const std::string& name, f32 x) const;
  virtual f32 GetV(const std::string& name, f32 y) const;

  virtual Color GetPixel(const std::string& name, u32 x, u32 y) const;
  virtual void* GetPixelAddr(const std::string& name, u32 x, u32 y) const;
  virtual BlockBuffer* GetPixels(const std::string& name) const;
  virtual size_t GetPixelSize(const std::string& name) const;
  virtual Color GetSampledColor(const std::string& name, f32 x, f32 y, f32 w, f32 h) const;

  virtual void SetPixel(const std::string& name, u32 x, u32 y, f32 r, f32 g, f32 b, f32 a);
  virtual void SetPixel(const std::string& name, u32 x, u32 y, const Color& color);
  virtual void SetPixels(const std::string& name, const void* pixels);
  virtual void SetPixelsFromBlockBuffer(const std::string& name, const BlockBuffer* pixels);

  virtual bool HasR() const;
  virtual bool HasG() const;
  virtual bool HasB() const;
  virtual bool HasA() const;

  static size_t GetPixelSize(TexFormat format);
  static bool LoadPixelsFromPNG(const void* data, size_t dataSize, TexData& texData);
  static bool LoadPixelsFromJPEG(const void* data, size_t dataSize, TexData& texData);

protected:

  virtual TexData* GetTexDataInternal(const std::string& name);

  virtual void OnWillDeleteTexData(TexData& texData);

  virtual bool AllocateMutableFormat(TexData& texData);
  virtual void CacheInfo();

};

};
