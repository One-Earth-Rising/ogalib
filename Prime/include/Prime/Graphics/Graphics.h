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

#include <Prime/Types/PrimitiveStack.h>
#include <Prime/Types/TypeStack.h>
#include <Prime/Types/Viewport.h>
#include <Prime/Graphics/DeviceProgram.h>
#include <Prime/Graphics/IndexBuffer.h>
#include <Prime/Graphics/ArrayBuffer.h>
#include <Prime/Graphics/Tex.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PxGraphics Graphics::GetInstance()

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _GraphicsScreenConfig {
  const char* title;
  u32 w;
  u32 h;
  bool windowed;
  u32 swapInterval;
} GraphicsScreenConfig;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Graphics {
friend class Engine;
protected:

  static Graphics* instance;

public:

  static Graphics& GetInstance();

protected:

  size_t maxTexW;
  size_t maxTexH;
  size_t maxTexUnits;

public:

  TypeStack<Mat44> projection;
  TypeStack<Mat44> view;
  TypeStack<Mat44> model;
  TypeStack<Viewport> viewport;
  PrimitiveStack<bool> depthMask;
  PrimitiveStack<bool> depthEnabled;
  TypeStack<Color> clearScreenColor;
  PrimitiveStack<f64> clearScreenDepth;
  PrimitiveStack<f32> nearZ;
  PrimitiveStack<f32> farZ;
  TypeStack<Vec4> clipPlane[PRIME_DEVICE_PROGRAM_CLIP_PLANE_COUNT];
  PrimitiveStack<bool> clipPlaneEnabled[PRIME_DEVICE_PROGRAM_CLIP_PLANE_COUNT];

  PrimitiveStack<DeviceProgram*> program;

protected:

  Graphics();

public:

  virtual ~Graphics();

protected:

  virtual void Init();
  virtual void Shutdown();

public:

////////////////////////////////////////////////////////////////////////////////

#pragma region Properties

  size_t GetMaxTexW() const {return maxTexW;}
  size_t GetMaxTexH() const {return maxTexH;}
  size_t GetMaxTexUnits() const {return maxTexUnits;}

////////////////////////////////////////////////////////////////////////////////

#pragma region Screen/Window

  virtual void ShowScreen(const GraphicsScreenConfig* config = nullptr);
  virtual f32 GetScreenW() const;
  virtual f32 GetScreenH() const;
  virtual f32 MapWindowToScreenX(f32 x) const;
  virtual f32 MapWindowToScreenY(f32 y) const;

  virtual void RequestFullscreenToggle();
  virtual bool IsFullscreenToggleRequested() const;
  virtual bool IsFullscreenToggled() const;

  virtual void RequestVsyncToggle();
  virtual bool IsVsyncToggleRequested() const;
  virtual bool IsVsyncToggled() const;

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Drawing

  virtual void StartFrame();
  virtual void EndFrame();

  virtual void LoadScreenOrtho();

  virtual void ClearScreen();
  virtual void ClearColor();
  virtual void ClearDepth();

  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, Tex* tex = nullptr);
  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, Tex* tex = nullptr);
  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, Tex* tex, Tex* tex2, ...);
  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, Tex* tex, Tex* tex2, ...);
  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, Tex* tex, Tex* tex2, va_list ap);
  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, Tex* tex, Tex* tex2, va_list ap);
  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, Tex* const* texList, size_t texCount);
  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, Tex* const* texList, size_t texCount);
  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, TexChannelTuple& tuple);
  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, TexChannelTuple& tuple);
  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, TexChannelTuple const* tupleList, size_t tupleCount);
  virtual void Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, TexChannelTuple const* tupleList, size_t tupleCount);

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

};

};
