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

#include <Prime/Config.h>
#include <Prime/System/RefObject.h>
#include <Prime/Enum/IndexFormat.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class IndexBuffer: public RefObject {
protected:

  IndexFormat format;
  size_t indexCount;
  size_t syncCount;
  bool loadedIntoVRAM;
  bool dataModified;

public:

  IndexFormat GetFormat() const {return format;}
  size_t GetIndexCount() const {return indexCount;}
  size_t GetSyncCount() const {return syncCount;}
  bool IsLoadedIntoVRAM() const {return loadedIntoVRAM;}
  bool IsDataModified() const {return dataModified;}

protected:

  IndexBuffer(IndexFormat format, const void* data, size_t count);

public:

  virtual ~IndexBuffer();

  static IndexBuffer* Create(IndexFormat format, const void* data, size_t count);

public:

  virtual bool LoadIntoVRAM();
  virtual bool UnloadFromVRAM();

  virtual size_t GetIndexSize() const;

  virtual size_t GetValue(size_t index) const;
  virtual void SetValue(size_t index, size_t value);
  virtual void SetValues(size_t start, size_t count, const void* data);
  virtual void CopyValueBlock(size_t index, size_t fromIndex, size_t count);

  virtual void SetSyncCount(size_t count);
  virtual void Sync();

};

};
