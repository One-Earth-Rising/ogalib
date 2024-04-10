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

#include <Prime/Graphics/Tex.h>
#include <Prime/Enum/TexFormat.h>
#include <Prime/Enum/WrapMode.h>
#include <Prime/Graphics/opengl/OpenGLInc.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class OpenGLTex: public Tex {
friend class OpenGLGraphics;
private:

  GLuint textureId;
  GLuint depthTextureId;
  GLuint frameBufferId;
  GLuint renderBufferId;
  bool bufferComplete;

  Dictionary<TexData*, GLint> texDataGLLevelLookup;

public:

  GLuint GetTextureId() const {return textureId;}
  GLuint GetDepthTextureId() const {return depthTextureId;}
  GLuint GetFrameBufferId() const {return frameBufferId;}
  GLuint GetRenderBufferId() const {return renderBufferId;}
  bool IsBufferComplete() const {return bufferComplete;}

public:

  OpenGLTex(u32 w, u32 h, TexFormat format, const void* pixels, const json& options);
  OpenGLTex(u32 w, u32 h, TexFormat format, const json& options);
  OpenGLTex(u32 w, u32 h, TexFormat format = TexFormatR8G8B8A8, const void* pixels = nullptr);
  OpenGLTex();
  OpenGLTex(const std::string& name, const std::string& data);
  ~OpenGLTex();

public:

  void SetFilteringEnabled(bool enabled) override;
  void SetWrapModeX(WrapMode wrapModeX) override;
  void SetWrapModeY(WrapMode wrapModeY) override;

  bool LoadIntoVRAM() override;
  bool UnloadFromVRAM() override;

protected:

  void OnWillDeleteTexData(TexData& texData) override;

  virtual bool LoadIntoVRAMOpenGLTex();
  virtual bool LoadIntoVRAMOpenGLRenderBuffer();
  virtual bool UnloadFromVRAMOpenGLTex();
  virtual bool UnloadFromVRAMOpenGLRenderBuffer();

protected:

  static void InitGlobal();
  static void ShutdownGlobal();

};

};

#endif
