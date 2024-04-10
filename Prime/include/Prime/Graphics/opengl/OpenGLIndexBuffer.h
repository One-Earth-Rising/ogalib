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

#include <Prime/Graphics/IndexBuffer.h>
#include <Prime/Graphics/opengl/OpenGLInc.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class OpenGLIndexBuffer: public IndexBuffer {
private:

  void* data;
  size_t dataSize;
  GLuint iboId;

public:

  GLuint GetIBOId() const {return iboId;}

public:

  OpenGLIndexBuffer(IndexFormat format, const void* data, size_t indexCount);
  ~OpenGLIndexBuffer();

public:

  bool LoadIntoVRAM() override;
  bool UnloadFromVRAM() override;

  size_t GetIndexSize() const override;

  size_t GetValue(size_t index) const override;
  void SetValue(size_t index, size_t value) override;
  void SetValues(size_t start, size_t count, const void* data) override;
  void CopyValueBlock(size_t index, size_t fromIndex, size_t count) override;

  void Sync() override;

};

};

#endif
