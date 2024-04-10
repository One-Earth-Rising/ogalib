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

#include <Prime/System/RefObject.h>
#include <Prime/Types/Stack.h>
#include <Prime/Types/Dictionary.h>
#include <Prime/Graphics/BufferPrimitive.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class ArrayBufferAttribute {
friend class ArrayBuffer;
private:

  std::string name;
  size_t size;
  size_t offset;

public:

  const std::string& GetName() const {return name;}
  const size_t GetSize() const {return size;}
  const size_t GetOffset() const {return offset;}

public:

  ArrayBufferAttribute(const std::string& name = std::string(), size_t size = 0, size_t offset = 0):
    name(name), size(size), offset(offset) {}
  ArrayBufferAttribute(const ArrayBufferAttribute& other) {(void) operator=(other);}
  ~ArrayBufferAttribute() {}

public:

  ArrayBufferAttribute& operator=(const ArrayBufferAttribute& other) {
    name = other.name;
    size = other.size;
    offset = other.offset;
    return *this;
  }

  ArrayBufferAttribute& operator=(s32 value) {
    if(value == 0) {
      name.clear();
      size = 0;
      offset = 0;
    }
    return *this;
  }

  bool operator==(const ArrayBufferAttribute& other) const {
    return name == other.name && size == other.size && offset == other.offset;
  }

  bool operator<(const ArrayBufferAttribute& other) const {
    if(name < other.name)
      return true;
    else if(name > other.name)
      return false;

    if(size < other.size)
      return true;
    else if(size > other.size)
      return false;

    return offset < other.offset;
  }

};

class ArrayBuffer: public RefObject {
protected:

  Stack<ArrayBufferAttribute> attributes;
  Dictionary<std::string, size_t> attributeLookup;

  size_t itemSize;
  size_t itemCount;
  size_t syncCount;
  bool loadedIntoVRAM;
  bool dataModified;
  BufferPrimitive primitive;

public:

  size_t GetItemSize() const {return itemSize;}
  size_t GetItemCount() const {return itemCount;}
  size_t GetSyncCount() const {return syncCount;}
  BufferPrimitive GetPrimitive() const {return primitive;}
  bool IsLoadedIntoVRAM() const {return loadedIntoVRAM;}
  bool IsDataModified() const {return dataModified;}

protected:

  ArrayBuffer(size_t itemSize, const void* data, size_t itemCount, BufferPrimitive primitive = BufferPrimitiveTriangles);

public:

  virtual ~ArrayBuffer();

  static ArrayBuffer* Create(size_t itemSize, const void* data, size_t itemCount, BufferPrimitive primitive = BufferPrimitiveTriangles);

public:

  virtual bool LoadIntoVRAM();
  virtual bool UnloadFromVRAM();

  virtual void LoadAttribute(const std::string& name, size_t size);
  virtual const std::string& GetAttributeName(size_t index) const;
  virtual const ArrayBufferAttribute* GetAttribute(const std::string& name);
  virtual size_t GetAttributeCount() const;
  virtual void ProcessAttributes();

  virtual const void* GetItem(size_t index) const;
  virtual void* GetItem(size_t index);
  virtual void SetItem(size_t index, const void* data);

  virtual void SetSyncCount(size_t count);
  virtual void Sync();

};

};
