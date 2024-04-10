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
#if defined(PrimeTargetOpenGL)

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/Graphics.h>
#include <Prime/Graphics/opengl/OpenGLProgram.h>
#include <Prime/Graphics/ArrayBuffer.h>
#include <Prime/Graphics/IndexBuffer.h>
#include <Prime/Graphics/Tex.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PxOpenGLGraphics OpenGLGraphics::GetInstance()

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _OpenGLGraphicsCurrentTexture {
  refptr<Tex> tex;
  TexChannel channel;
  bool enabled;
  bool hasAlpha;
} OpenGLGraphicsCurrentTexture;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class OpenGLGraphics: public Graphics {
public:

  static OpenGLGraphics& GetInstance();

private:

  GLFWwindow* screenWindow;

  TypeStack<Mat44> drawMatMVP;  
  TypeStack<Mat44> drawMatModel;
  TypeStack<Mat44> drawMatView;
  TypeStack<Mat44> drawMatVP;
  TypeStack<Mat44> drawMatMV;

  // Render State
  TypeStack<OpenGLGraphicsCurrentTexture>* currentTextureStacks;
  PrimitiveStack<GLuint> currentIBOId;
  PrimitiveStack<GLuint> currentABOId;
  PrimitiveStack<GLuint> currentProgramId;
  Color currentClearScreenColor;
  f64 currentClearScreenDepth;
  Viewport currentViewport;
  bool currentDepthMask;
  bool currentDepthEnabled;

public:

  OpenGLGraphics();
  ~OpenGLGraphics();

protected:

  void Init() override;
  void Shutdown() override;

public:

  void ShowScreen(const GraphicsScreenConfig* config = nullptr) override;
  f32 GetScreenW() const override;
  f32 GetScreenH() const override;

  void RequestFullscreenToggle() override;
  bool IsFullscreenToggleRequested() const override;
  bool IsFullscreenToggled() const override;

  void RequestVsyncToggle() override;
  bool IsVsyncToggleRequested() const override;
  bool IsVsyncToggled() const override;

  void StartFrame() override;
  void EndFrame() override;

  void ClearScreen() override;
  void ClearColor() override;
  void ClearDepth() override;

  void Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, TexChannelTuple const* tupleList, size_t tupleCount) override;

  virtual GLFWwindow* GetOpenGLGLFWScreenWindow() const;

protected:

  virtual void ResetRenderState();

  virtual void PushDrawTexChannelTupleList(TexChannelTuple const* tupleList, size_t tupleCount);
  virtual void PushDrawTex(Tex* tex, size_t unit, TexChannel channel = TexChannelMain);
  virtual void PushDrawIndexBuffer(IndexBuffer* ib);
  virtual void PushDrawArrayBuffer(ArrayBuffer* ab);
  virtual void PushDrawProgram(DeviceProgram* deviceProgram);
  virtual void PushDrawMatrices();

  virtual void PopDrawTexChannelTupleList();
  virtual void PopDrawTex(size_t unit);
  virtual void PopDrawIndexBuffer();
  virtual void PopDrawArrayBuffer();
  virtual void PopDrawProgram();
  virtual void PopDrawMatrices();

  virtual void LoadDrawViewport();
  virtual void LoadDrawDepth();

};

};

#endif
