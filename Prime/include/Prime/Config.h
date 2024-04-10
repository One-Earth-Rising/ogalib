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

#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#define PrimeTargetWindows 1
#define PrimeTargetOpenGL 1
#elif defined(__ORBIS__)
#define PrimeTargetPS4 1
#elif defined(__PROSPERO__)
#define PrimeTargetPS5 1
#endif

#include <ogalib/json.h>
#if defined(__cplusplus)
namespace Prime {
using ogalib::json;
};
#endif

#if defined(__cplusplus)
#define PrimeExternCBegin extern "C" {
#define PrimeExternCEnd };
#else
#define PrimeExternCBegin
#define PrimeExternCEnd
#endif

#include <stdio.h>
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#if defined(_DEBUG)
#if defined(PrimeTargetWindows)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#ifdef __cplusplus
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif
#endif

#if defined(PrimeTargetWindows)
#define _GLFW_WIN32
#define _GLFW_WGL
#endif

////////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////////

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

typedef float f32;
typedef double f64;

#define PrimeNotFound (-1)

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG)
#define dbgprintf printf
#define PrimeAssert(b, f, ...) {if(!(b)) {Prime::AssertCore(__FILE__, __LINE__, f, ##__VA_ARGS__);}} ((void)0)
namespace Prime {
extern void AssertCore(const char* file, u32 line, const char* f, ...);
};
#else
#define dbgprintf(f, ...) ((void)0)
#define PrimeAssert(b, f, ...) ((void)0)
#endif

#define PxRequireMainThread PrimeAssert(Thread::IsMainThread(), "Must be on main thread.")
#define PxRequireNonMainThread PrimeAssert(!Thread::IsMainThread(), "Must not be on main thread.")

#define PrimeSafeFree(p) if(p) {free(p); (p) = nullptr;} ((void) 0)
#define PrimeSafeDelete(p) if(p) {delete (p); (p) = nullptr;} ((void) 0)
#define PrimeSafeDeleteArray(p) if(p) {delete[] (p); (p) = nullptr;} ((void) 0)

#define AlignMem(alignment, size) (((size) + (alignment) - 1) & ~((alignment) - 1))

namespace Prime {

typedef enum {
  AlignNone         = 0,
  AlignTop          = 0x01,
  AlignBottom       = 0x02,
  AlignVCenter      = 0x04,
  AlignLeft         = 0x08,
  AlignRight        = 0x10,
  AlignHCenter      = 0x20,
  AlignCenter       = (AlignHCenter | AlignVCenter),
  AlignBottomLeft   = (AlignBottom | AlignLeft),
} Align;

};

#define PrimePi           3.14159265358979323846
#define PrimePiF          3.14159265358979323846f
#define Prime2Pi          6.28318530717958647692
#define Prime2PiF         6.28318530717958647692f
#define Prime6Pi          18.8495559215387594308
#define Prime6PiF         18.8495559215387594308f
#define PrimeInvPi        0.31830988618379067153
#define PrimeInvPiF       0.31830988618379067153f
#define PrimePiBy2        (PrimePi / 2.0)
#define PrimePiBy2F       (PrimePiF / 2.0)
#define PrimePiBy4        (PrimePi / 4.0)
#define PrimePiBy4F       (PrimePiF / 4.0)
#define PrimePiBy8        (PrimePi / 8.0)
#define PrimePiBy8F       (PrimePiF / 8.0)
#define PrimeInv180       0.00555555555555555555
#define PrimeInv360       0.00277777777777777778
#define PrimeInv180F      0.00555555555555555555f
#define PrimeInv360F      0.00277777777777777778f
#define PrimeDegToRad     (PrimePi * PrimeInv180)
#define PrimeRadToDeg     (180.0 * PrimeInvPi)
#define PrimeDegToRadF    (PrimePiF * PrimeInv180F)
#define PrimeRadToDegF    (180.0f * PrimeInvPiF)
#define PrimeGoldenRatio  1.61803398875
#define Prime1OverRoot2   0.7071067811865475244
#define Prime1OverRoot2F  0.7071067811865475244f

// Must before min/max declaration.
#include <Prime/System/Random.h>

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

static __inline void* memalign(size_t alignment, size_t size) {
  return malloc(AlignMem(alignment, size));
}

template <typename T>
T min(const T& a, const T& b) {
  return a < b ? a : b;
}

template <typename T>
T max(const T& a, const T& b) {
  return a > b ? a : b;
}

template <typename T>
T clamp(const T& value, const T& low, const T& high) {
  return value < low ? low : (value > high ? high : value); 
}

#include <ogalib/ogalib.h>

namespace Prime {
class Content;

template <class T>
class Stack;

extern f64 GetSystemTime();
extern f64 GetTargetRTCSeconds();
extern void* ReadFile(const std::string& uri, size_t* size);
extern void ReadFile(const std::string& uri, const std::function<void (void*, size_t)>& callback);
extern void GetContent(const std::string& uri, const std::function<void (Content*)>& callback);
extern void GetContent(const std::string& uri, const json& info, const std::function<void (Content*)>& callback);
extern void GetContentRaw(const std::string& uri, const std::function<void (const void*, size_t)>& callback);
extern void GetContentRaw(const std::string& uri, const json& info, const std::function<void (const void*, size_t)>& callback);
extern void MapContentURI(const std::string& mappedURI, const std::string& uri);
extern const std::string& GetMapppedContentURI(const std::string& uri);
extern void GetPackFilenames(const std::string& uri, Stack<std::string>& filenames);
extern bool LockSetjmpMutex();
extern bool UnlockSetjmpMutex();

extern bool IsFormatJSON(const void* data, size_t dataSize, const json& info);
extern bool IsFormatJSON(const void* data, size_t dataSize, const json& info, json& output);
extern bool IsFormatJSONWithValue(const void* data, size_t dataSize, const json& info, const std::string& key, std::string& value);
extern bool IsFormatJSONWithArray(const void* data, size_t dataSize, const json& info, const std::string& key);
extern bool IsFormatPNG(const void* data, size_t dataSize, const json& info);
extern bool IsFormatJPEG(const void* data, size_t dataSize, const json& info);
extern bool IsFormatBC(const void* data, size_t dataSize, const json& info);
extern bool IsFormatGLTF(const void* data, size_t dataSize, const json& info);
extern bool IsFormatFBX(const void* data, size_t dataSize, const json& info);
extern bool IsFormatOTF(const void* data, size_t dataSize, const json& info);

using ogalib::SetGlobalSendURLParams;
using ogalib::SendURL;
using ogalib::string_printf;
using ogalib::string_vprintf;
using ogalib::Job;
using ogalib::JobType;
using ogalib::Thread;
using ogalib::ThreadMutex;
using ogalib::ThreadCondition;

static __inline std::wstring towstring(const std::string& s) {
  return std::wstring(s.begin(), s.end()).c_str();
}

static __inline bool StartsWith(std::string_view str, std::string_view prefix) {
  return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

static __inline bool EndsWith(std::string_view str, std::string_view suffix) {
  return str.size() >= suffix.size() && str.compare(str.size()-suffix.size(), suffix.size(), suffix) == 0;
}

static __inline std::string ToLower(const std::string& str) {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
    return std::tolower(c);
  });
  return result;
}

static __inline f32 GetLerp(f32 a, f32 b, f32 t) {
  if(t == 0.0f)
    return a;
  else if(t == 1.0f)
    return b;
  else
    return a + (b - a) * t;
}

static __inline size_t GetNextPowerOf2(size_t v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  v++;
  return v;
}
};
