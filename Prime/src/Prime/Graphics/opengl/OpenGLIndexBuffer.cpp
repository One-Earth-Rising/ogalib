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

#include <Prime/Graphics/opengl/OpenGLIndexBuffer.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/opengl/OpenGLInc.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

static const size_t IndexBufferDataSizeTable[] = {
  0,
  sizeof(u8),
  sizeof(u16),
  sizeof(u32),
};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

OpenGLIndexBuffer::OpenGLIndexBuffer(IndexFormat format, const void* data, size_t indexCount): IndexBuffer(format, data, indexCount),
iboId(GL_NONE) {
  dataSize = indexCount * IndexBufferDataSizeTable[format];

  this->data = malloc(dataSize);
  PrimeAssert(this->data, "Could not create data.");
  if(data) {
    memcpy(this->data, data, dataSize);
  }
  else {
    memset(this->data, 0, dataSize);
  }
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() {
  UnloadFromVRAM();
  PrimeSafeFree(data);
}

bool OpenGLIndexBuffer::LoadIntoVRAM() {
  if(loadedIntoVRAM)
    return true;

  GLint id;
  GLCMD(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &id));

  GLCMD(glGenBuffers(1, &iboId));
  if(IsOpenGLOutOfMemory()) {
    PrimeAssert(false, "Out of memory.");
  }

  GLCMD(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId));

  GLCMD(glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferDataSizeTable[format] * syncCount, data, GL_STATIC_DRAW));
  if(IsOpenGLOutOfMemory()) {
    PrimeAssert(false, "Out of memory.");
  }

  GLCMD(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id));

  dataModified = false;

  loadedIntoVRAM = true;

  return true;
}

bool OpenGLIndexBuffer::UnloadFromVRAM() {
  if(!loadedIntoVRAM)
    return true;

  if(iboId) {
    GLCMD(glDeleteBuffers(1, &iboId));
  }

  loadedIntoVRAM = false;

  return true;
}

size_t OpenGLIndexBuffer::GetIndexSize() const {
  return IndexBufferDataSizeTable[format];
}

size_t OpenGLIndexBuffer::GetValue(size_t index) const {
  if(indexCount == 0)
    return 0;

  size_t useIndex = (index < indexCount) ? index : (index % indexCount);
  if(format == IndexFormatSize8) {
    u8* data8 = static_cast<u8*>(data);
    return data8[useIndex];
  }
  else if(format == IndexFormatSize16) {
    u16* data16 = static_cast<u16*>(data);
    return data16[useIndex];
  }
  else if(format == IndexFormatSize32) {
    size_t* data32 = static_cast<size_t*>(data);
    return data32[useIndex];
  }
  else {
    PrimeAssert(false, "Invalid index format.");
    return 0;
  }
}

void OpenGLIndexBuffer::SetValue(size_t index, size_t value) {
  if(indexCount == 0)
    return;

  size_t useIndex = (index < indexCount) ? index : (index % indexCount);
  if(format == IndexFormatSize8) {
    u8* data8 = static_cast<u8*>(data);
    data8[useIndex] = (u8) (value & 0xFF);
    dataModified = true;
  }
  else if(format == IndexFormatSize16) {
    u16* data16 = static_cast<u16*>(data);
    data16[useIndex] = (u16) (value & 0xFFff);
    dataModified = true;
  }
  else if(format == IndexFormatSize32) {
    size_t* data32 = static_cast<size_t*>(data);
    data32[useIndex] = value;
    dataModified = true;
  }
  else {
    PrimeAssert(false, "Invalid index format.");
  }
}

void OpenGLIndexBuffer::SetValues(size_t start, size_t count, const void* data) {
  if(indexCount == 0 || count == 0)
    return;

  size_t useStart = (start < indexCount) ? start : (start % indexCount);
  size_t useCount = (useStart + count > indexCount) ? (indexCount - useStart) : count;
  if(useCount > 0) {
    u8* data8 = static_cast<u8*>(this->data);
    const size_t itemSize = IndexBufferDataSizeTable[format];
    memcpy(&data8[useStart * itemSize], data, itemSize * useCount);
    dataModified = true;
  }
}

void OpenGLIndexBuffer::CopyValueBlock(size_t index, size_t fromIndex, size_t count) {
  if(indexCount == 0 || count == 0)
    return;

  u8* data8 = static_cast<u8*>(this->data);
  const size_t itemSize = IndexBufferDataSizeTable[format];
  if(itemSize == 1) {
    u8* d = &((u8*) data8)[index];
    u8* s = &((u8*) data8)[fromIndex];
    u8* e = d + count;
    while(d != e) {
      *d++ = *s++;
    }
  }
  else if(itemSize == 2) {
    u16* d = &((u16*) data8)[index];
    u16* s = &((u16*) data8)[fromIndex];
    u16* e = d + count;
    while(d != e) {
      *d++ = *s++;
    }
  }
  else {
    PrimeAssert(itemSize == 4, "Expected final option to be 4-byte item size.");
    size_t* d = &((size_t*) data8)[index];
    size_t* s = &((size_t*) data8)[fromIndex];
    size_t* e = d + count;
    while(d != e) {
      *d++ = *s++;
    }
  }

  dataModified = true;
}

void OpenGLIndexBuffer::Sync() {
  if(!loadedIntoVRAM)
    return;

  GLint id;
  GLCMD(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &id));
  GLCMD(glBindBuffer(GL_ARRAY_BUFFER, iboId));
  GLCMD(glBufferData(GL_ARRAY_BUFFER, IndexBufferDataSizeTable[format] * syncCount, data, GL_STATIC_DRAW));
  GLCMD(glBindBuffer(GL_ARRAY_BUFFER, id));

  dataModified = false;
}

#endif
