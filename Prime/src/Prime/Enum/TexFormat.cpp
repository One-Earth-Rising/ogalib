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

#include <Prime/Enum/TexFormat.h>
#include <Prime/Types/Dictionary.h>

static const char* TexFormatAsStringTable[] = {
  "TexFormatNone",
  "TexFormatNative",
  "TexFormatR8G8B8A8",
  "TexFormatR8G8B8",
  "TexFormatR8G8",
  "TexFormatR8",
  "TexFormatR5G6B5",
  "TexFormatR5G5B5A1",
  "TexFormatR4G4B4A4",
  "TexFormatFPBuffer",
  "TexFormatFPBufferHQ",
  "TexFormatFPBufferNoAlpha",
  "TexFormatFPBufferNoAlphaHQ",
  "TexFormatDepthBuffer",
  "TexFormatShadowMap",
  "TexFormatPositionBuffer",
  "TexFormatNormalBuffer",
  "TexFormatGlowBuffer",
  "TexFormatSpecularBuffer",
};

static const Prime::Dictionary<std::string, TexFormat> TexFormatFromStringLookup = {
  {"None", TexFormatNone},
  {"Native", TexFormatNative},
  {"R8G8B8A8", TexFormatR8G8B8A8},
  {"R8G8B8", TexFormatR8G8B8},
  {"R8G8", TexFormatR8G8},
  {"R8", TexFormatR8},
  {"R5G6B5", TexFormatR5G6B5},
  {"R5G5B5A1", TexFormatR5G5B5A1},
  {"R4G4B4A4", TexFormatR4G4B4A4},
  {"FPBuffer", TexFormatFPBuffer},
  {"FPBufferHQ", TexFormatFPBufferHQ},
  {"FPBufferNoAlpha", TexFormatFPBufferNoAlpha},
  {"FPBufferNoAlphaHQ", TexFormatFPBufferNoAlphaHQ},
  {"DepthBuffer", TexFormatDepthBuffer},
  {"ShadowMap", TexFormatShadowMap},
  {"PositionBuffer", TexFormatPositionBuffer},
  {"NormalBuffer", TexFormatNormalBuffer},
  {"GlowBuffer", TexFormatGlowBuffer},
  {"SpecularBuffer", TexFormatSpecularBuffer},
};

TexFormat GetEnumTexFormatFromString(const std::string& s) {
  if(auto it = TexFormatFromStringLookup.Find(s))
    return it.value();
  else
    return (TexFormat) 0;
}

const char* GetEnumTexFormatAsString(TexFormat v) {
  return TexFormatAsStringTable[v];
}
