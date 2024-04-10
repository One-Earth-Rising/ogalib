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

#include <string>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PrimeBlockBufferLoadStop ((size_t) -1)

////////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////////

typedef size_t (*BlockBufferLoadCallback)(void* data, void* b, size_t size);

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class BlockBuffer {
private:

  uint8_t** blocks;
  size_t blockCount;
  size_t blockSize;
  size_t blockAlignment;
  size_t totalSize;

public:

  size_t GetSize() const {return totalSize;}
  size_t GetBlockSize() const {return blockSize;}
  size_t GetBlockAlignment() const {return blockAlignment;}

public:

  BlockBuffer(size_t blockSize = 0, size_t initSize = 0, size_t blockAlignment = 0);
  BlockBuffer(const BlockBuffer& other);
  virtual ~BlockBuffer();
  
public:

  BlockBuffer& operator=(const BlockBuffer& other);

  void Clear();

  size_t Load(BlockBufferLoadCallback callback, size_t size, void* data, size_t tempBufferSize = 0);

  size_t Read(void* p, size_t offset, size_t size) const;
  size_t Append(const void* p, size_t size);

  void SetValue(uint8_t value, size_t offset, size_t size);

  void* GetAddr(size_t offset) const;

  bool CanDirectCopy(const BlockBuffer& other) const;
  size_t CopyTo(void* p, size_t offset, size_t size) const;
  void* ConvertToBytes(size_t* size = NULL, uint32_t alignment = 0) const;
  void* ConsumeToBytes(size_t* size = NULL);
  std::string ToString() const;

};

};
