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

#include <Prime/Config.h>

typedef enum {
  TexFormatNone = 0,
  TexFormatNative = 1,
  TexFormatR8G8B8A8 = 2,
  TexFormatR8G8B8 = 3,
  TexFormatR8G8 = 4,
  TexFormatR8 = 5,
  TexFormatR5G6B5 = 6,
  TexFormatR5G5B5A1 = 7,
  TexFormatR4G4B4A4 = 8,
  TexFormatFPBuffer = 9,
  TexFormatFPBufferHQ = 10,
  TexFormatFPBufferNoAlpha = 11,
  TexFormatFPBufferNoAlphaHQ = 12,
  TexFormatDepthBuffer = 13,
  TexFormatShadowMap = 14,
  TexFormatPositionBuffer = 15,
  TexFormatNormalBuffer = 16,
  TexFormatGlowBuffer = 17,
  TexFormatSpecularBuffer = 18,
  TexFormat_Count = 19
} TexFormat;

#if defined(__cplusplus) && !defined(__INTELLISENSE__)
namespace std {
  template<> struct hash<TexFormat> {
    size_t operator()(const TexFormat& v) const noexcept {
      return hash<s32>()(v);
    }
  };
};
#endif

extern TexFormat GetEnumTexFormatFromString(const std::string& s);
extern const char* GetEnumTexFormatAsString(TexFormat v);

#define GetEnumTexFormatCount() TexFormat_Count
#define GetEnumTexFormatCountAsInt() 19
