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

#include <Prime/Config.h>
#if defined(PrimeTargetOpenGL)

#include <Prime/Graphics/opengl/OpenGLArrayBuffer.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/opengl/OpenGLInc.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

OpenGLArrayBuffer::OpenGLArrayBuffer(size_t itemSize, const void* data, size_t itemCount, BufferPrimitive primitive): ArrayBuffer(itemSize, data, itemCount, primitive),
aboId(GL_NONE) {
  PrimeAssert(itemSize > 0, "Invalid array buffer item size.");
  PrimeAssert(itemCount > 0, "Invalid array buffer item count.");

  dataSize = itemCount * itemSize;

  this->data = malloc(dataSize);
  PrimeAssert(this->data, "Could not create data.");
  if(data)
    memcpy(this->data, data, dataSize);
  else
    memset(this->data, 0, dataSize);
}

OpenGLArrayBuffer::~OpenGLArrayBuffer() {
  UnloadFromVRAM();
  PrimeSafeFree(data);
}

bool OpenGLArrayBuffer::LoadIntoVRAM() {
  if(loadedIntoVRAM)
    return true;

  ProcessAttributes();

  GLint id;
  GLCMD(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &id));

  GLCMD(glGenBuffers(1, &aboId));
  if(IsOpenGLOutOfMemory()) {
    PrimeAssert(false, "Out of memory.");
  }

  GLCMD(glBindBuffer(GL_ARRAY_BUFFER, aboId));

  GLCMD(glBufferData(GL_ARRAY_BUFFER, itemSize * syncCount, data, GL_STATIC_DRAW));
  if(IsOpenGLOutOfMemory()) {
    PrimeAssert(false, "Out of memory.");
  }

  GLCMD(glBindBuffer(GL_ARRAY_BUFFER, id));

  dataModified = false;
  loadedIntoVRAM = true;

  return true;
}

bool OpenGLArrayBuffer::UnloadFromVRAM() {
  if(!loadedIntoVRAM)
    return true;

  if(aboId) {
    GLCMD(glDeleteBuffers(1, &aboId));
  }

  loadedIntoVRAM = false;

  return true;
}

const void* OpenGLArrayBuffer::GetItem(size_t index) const {
  if(itemCount == 0)
    return NULL;

  size_t useIndex = (index < itemCount) ? index : (index % itemCount);
  u8* data8 = static_cast<u8*>(data);
  return &data8[useIndex * itemSize];
}

void* OpenGLArrayBuffer::GetItem(size_t index) {
  if(itemCount == 0)
    return NULL;

  dataModified = true;
  size_t useIndex = (index < itemCount) ? index : (index % itemCount);
  u8* data8 = static_cast<u8*>(data);
  return &data8[useIndex * itemSize];
}

void OpenGLArrayBuffer::SetItem(size_t index, const void* data) {
  if(itemCount == 0)
    return;

  dataModified = true;
  size_t useIndex = (index < itemCount) ? index : (index % itemCount);
  u8* data8 = static_cast<u8*>(this->data);
  memcpy(&data8[useIndex * itemSize], data, itemSize);
}

void OpenGLArrayBuffer::Sync() {
  if(!loadedIntoVRAM)
    return;

  GLint id;
  GLCMD(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &id));
  GLCMD(glBindBuffer(GL_ARRAY_BUFFER, aboId));
  GLCMD(glBufferData(GL_ARRAY_BUFFER, itemSize * syncCount, data, GL_STATIC_DRAW));
  GLCMD(glBindBuffer(GL_ARRAY_BUFFER, id));

  dataModified = false;
}

#endif
