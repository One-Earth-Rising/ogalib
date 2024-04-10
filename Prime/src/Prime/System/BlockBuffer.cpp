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

#include <Prime/System/BlockBuffer.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Config.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define BLOCK_BUFFER_DEFAULT_BLOCK_SIZE (16 * 1024)

#if defined(_DEBUG) && 0
#define BlockBufferDprintf dbgprintf
#else
static __inline void BlockBufferDprintf(const char* f, ...) {}
#endif

////////////////////////////////////////////////////////////////////////////////
// Fnctions
////////////////////////////////////////////////////////////////////////////////

static __inline size_t GetBlockBufferNextPowerOf2(size_t v) {
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

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

BlockBuffer::BlockBuffer(size_t blockSize, size_t initSize, size_t blockAlignment):
blocks(nullptr),
blockCount(0),
blockAlignment(0),
totalSize(0) {

  if(blockSize == 0)
    this->blockSize = BLOCK_BUFFER_DEFAULT_BLOCK_SIZE;
  else
    this->blockSize = blockSize;

  if(blockAlignment > 0)
    this->blockAlignment = GetBlockBufferNextPowerOf2(blockAlignment);

  if(initSize > 0 && this->blockSize > initSize)
    this->blockSize = initSize;

  if(initSize)
    Append(nullptr, initSize);
}

BlockBuffer::BlockBuffer(const BlockBuffer& other):
blocks(nullptr),
blockCount(0),
totalSize(0),
blockSize(other.blockSize),
blockAlignment(other.blockAlignment) {
  (void) operator=(other);
}

BlockBuffer::~BlockBuffer() {
  Clear();
}

BlockBuffer& BlockBuffer::operator=(const BlockBuffer& other) {
  if(CanDirectCopy(other)) {
    size_t p = 0;
    while(p < other.totalSize) {
      void* addr = GetAddr(p);
      if(addr) {
        size_t bytesRead = other.Read(addr, p, blockSize);
        p += bytesRead;
      }
      else {
        break;
      }
    }
  }
  else {
    Clear();

    if(other.totalSize > 0) {
      if(blockSize == 0)
        blockSize = BLOCK_BUFFER_DEFAULT_BLOCK_SIZE;

      if(blockSize > other.totalSize)
        blockSize = other.totalSize;

      void* buffer = blockAlignment > 0 ? memalign(blockAlignment, blockSize) : malloc(blockSize);
      if(buffer) {
        size_t p = 0;
        while(p < other.totalSize) {
          size_t bytesRead = other.Read(buffer, p, blockSize);
          Append(buffer, bytesRead);
          p += bytesRead;
        }

        PrimeSafeFree(buffer);
      }
      else {
        PrimeAssert(buffer, "Could not create buffer.");
      }
    }
  }

  return *this;
}

void BlockBuffer::Clear() {
  if(!blocks)
    return;

  for(size_t i = 0; i < blockCount; i++) {
    if(blocks[i]) {
      free(blocks[i]);
    }
  }

  free(blocks);
  blocks = nullptr;

  blockCount = 0;
  totalSize = 0;
}

size_t BlockBuffer::Load(BlockBufferLoadCallback callback, size_t size, void* data, size_t tempBufferSize) {
  if(callback == nullptr || size == 0)
    return 0;

  BlockBufferDprintf("Creating block with size %d bytes, block size %d bytes\n", size, blockSize);

  uint8_t* tempBuffer = nullptr;
  size_t useTempBufferSize = 0;
  if(tempBufferSize > 0) {
    useTempBufferSize = min(size, tempBufferSize);
    BlockBufferDprintf("Creating temp buffer with size %d bytes\n", useTempBufferSize);
    tempBuffer = (uint8_t*) malloc(useTempBufferSize);
    PrimeAssert(tempBuffer, "Could not create temp buffer.");
  }

  blockSize = size < blockSize ? size : blockSize;
  totalSize = size;
  blockCount = (totalSize / blockSize) + ((totalSize % blockSize) > 0 ? 1 : 0);

  BlockBufferDprintf("- block size adjusted to = %d bytes\n", blockSize);
  BlockBufferDprintf("- block count = %d\n", blockCount);

  blocks = (uint8_t**) calloc(blockCount, sizeof(uint8_t*));
  if(!blocks) {
    BlockBufferDprintf("- ERROR: could not allocate space for %d blocks\n", blockCount);
    PrimeAssert(false, "Could not create block array.");
    Clear();
    return 0;
  }

  for(size_t i = 0; i < blockCount; i++) {
    blocks[i] = (uint8_t*) (blockAlignment > 0 ? memalign(blockAlignment, blockSize) : malloc(blockSize));
    if(!blocks[i]) {
      BlockBufferDprintf("- ERROR: could not allocate space for all blocks: %d/%d allocated\n", i, blockCount);
      PrimeAssert(tempBuffer, "Could not allocate space for all blocks.");
      Clear();
      return 0;
    }
  }

  BlockBufferDprintf("- now reading bytes\n");

  size_t bytesRead = 0;

  if(tempBuffer) {
    size_t tempBufferBytesAvail = 0;
    size_t tempBufferBytesRead = 0;
    for(size_t i = 0; i < blockCount; i++) {
      size_t sizeToRead;
      size_t bytesLeft;
      uint8_t* currentBlock = blocks[i];
      if(bytesRead + blockSize > totalSize) {
        sizeToRead = totalSize - bytesRead;
      }
      else {
        sizeToRead = blockSize;
      }
      bytesLeft = sizeToRead;
      while(bytesLeft > 0) {
        size_t loopBytesRead = sizeToRead - bytesLeft;

        BlockBufferDprintf("    - calling callback for %d bytes\n", bytesLeft);

        if(tempBufferBytesAvail == 0) {
          size_t tempBufferBytesToRead = useTempBufferSize;
          s64 callbackBytesRead = callback(data, tempBuffer, tempBufferBytesToRead);
          if(callbackBytesRead > 0) {
            tempBufferBytesAvail = callbackBytesRead;
            tempBufferBytesRead = 0;
          }
          else if(callbackBytesRead == PrimeBlockBufferLoadStop) {
            break;
          }
        }

        if(tempBufferBytesAvail > 0) {
          size_t tempBufferBytesToRead = min(bytesLeft, tempBufferBytesAvail);
          memcpy(&currentBlock[loopBytesRead], tempBuffer + tempBufferBytesRead, tempBufferBytesToRead);
          tempBufferBytesAvail -= tempBufferBytesToRead;
          tempBufferBytesRead += tempBufferBytesToRead;
          bytesLeft -= tempBufferBytesToRead;
        }
      }
      bytesRead += sizeToRead;
    }
  }
  else {
    for(size_t i = 0; i < blockCount; i++) {
      size_t sizeToRead;
      size_t bytesLeft;
      uint8_t* currentBlock = blocks[i];
      if(bytesRead + blockSize > totalSize) {
        sizeToRead = totalSize - bytesRead;
      }
      else {
        sizeToRead = blockSize;
      }
      bytesLeft = sizeToRead;
      while(bytesLeft > 0) {
        size_t loopBytesRead = sizeToRead - bytesLeft;

        BlockBufferDprintf("    - calling callback for %d bytes\n", bytesLeft);

        s64 callbackBytesRead = callback(data, &currentBlock[loopBytesRead], bytesLeft);
        if(callbackBytesRead > 0) {
          bytesLeft -= callbackBytesRead;
        }
        else if(callbackBytesRead == PrimeBlockBufferLoadStop) {
          break;
        }
      }
      bytesRead += sizeToRead;
    }
  }

  PrimeSafeFree(tempBuffer);

  if(bytesRead != totalSize) {
    BlockBufferDprintf("- ERROR: did not completely read all bytes from callback\n");
    Clear();
    return 0;
  }
  else {
    BlockBufferDprintf("- successfully read block buffer with a total size of %d bytes\n", totalSize);
  }

  return bytesRead;
}

size_t BlockBuffer::Read(void* p, size_t offset, size_t size) const {
  if(!blocks || !p || size == 0)
    return 0;

  size_t useSize;
  size_t result;
  size_t m;   // byte marker
  uint8_t* s;      // source
  uint8_t* d;      // destination

  if(offset >= totalSize)
    return 0;

  if(offset + size >= totalSize)
    useSize = totalSize - offset;
  else
    useSize = size;

  result = 0;
  m = offset;
  d = (uint8_t*) p;
  while(result < useSize) {
    size_t blockIndex = m / blockSize;
    uint8_t* block = blocks[blockIndex];
    size_t maxBytes;
    size_t mWrap = m % blockSize;

    if(blockIndex == blockCount - 1) {
      size_t lastBlockSize;
      if(blockCount == 1 && totalSize == blockSize)
        lastBlockSize = totalSize;
      else
        lastBlockSize = blockSize - ((blockCount * blockSize) - totalSize);
      maxBytes = lastBlockSize - mWrap;
    }
    else {
      maxBytes = blockSize - mWrap;
    }

    if(result + maxBytes > size)
      maxBytes = size - result;

    s = &block[mWrap];

    if(maxBytes > useSize)
      maxBytes = useSize;

#if SIZE_T_MAX > 0xFFFFFFFFU
    if(maxBytes >= (1ull << 32))
      maxBytes = 0xFFFFFFFF;
#endif

    PrimeAssert(maxBytes > 0, "Invalid amount of bytes to read.");

    memcpy(d, s, maxBytes);

    d += maxBytes;
    result += maxBytes;
    m += maxBytes;
  }

  return result;
}

size_t BlockBuffer::Append(const void* p, size_t size) {
  if(size == 0)
    return 0;

  if(!blocks) {
    uint8_t* block = (uint8_t*) (blockAlignment > 0 ? memalign(blockAlignment, blockSize) : malloc(blockSize));
    if(block) {
      blocks = (uint8_t**) calloc(1, sizeof(uint8_t*));
      if(blocks) {
        blocks[0] = block;
        blockCount = 1;
      }
      else {
        free(block);
      }
    }
    else {
      PrimeAssert(false, "Could not allocate block.");
    }
  }

  if(!blocks)
    return 0;

  size_t bytesWrote = 0;
  uint8_t* d = (uint8_t*) p;

  while(bytesWrote < size) {
    if(blockCount * blockSize == totalSize) {
      uint8_t* newBlock = (uint8_t*) (blockAlignment > 0 ? memalign(blockAlignment, blockSize) : malloc(blockSize));
      if(newBlock) {
        uint8_t** newBlocks = (uint8_t**) realloc(blocks, (blockCount + 1) * sizeof(uint8_t*));
        if(newBlocks) {
          blocks = newBlocks;
          blocks[blockCount] = newBlock;
          blockCount++;
        }
        else {
          PrimeAssert(false, "Could not scale-up block count.");
          free(newBlock);
        }
      }
      else {
        PrimeAssert(false, "Could not allocate new block.");
      }
    }

    if(blockCount * blockSize == totalSize) {
      break;
    }

    uint8_t* currentBlock = blocks[blockCount - 1];
    size_t blockEnd = totalSize % blockSize;
    size_t availSize = blockSize - blockEnd;
    size_t sizeToWrite;
    
    if((size - bytesWrote) < availSize) {
      sizeToWrite = size - bytesWrote;
    }
    else {
      sizeToWrite = availSize;
    }

#if SIZE_T_MAX > 0xFFFFFFFFU
    if(sizeToWrite >= (1ull << 32))
      sizeToWrite = 0xFFFFFFFF;
#endif

    if(p) {
      memcpy(&currentBlock[blockEnd], &d[bytesWrote], sizeToWrite);
    }
    else {
      memset(&currentBlock[blockEnd], 0, sizeToWrite);
    }

    bytesWrote += sizeToWrite;
    totalSize += sizeToWrite;
  }

  return bytesWrote;
}

void BlockBuffer::SetValue(uint8_t value, size_t offset, size_t size) {
  if(offset + size > totalSize)
    Append(nullptr, offset + size - totalSize);

  if(totalSize == 0)
    return;

  size_t useSize = size;
  if(useSize > totalSize)
    useSize = totalSize;

  size_t bytesWrote = 0;
  size_t b = offset;

  while(bytesWrote < useSize) {
    size_t blockIndex = b / blockSize;
    if(blockIndex >= blockCount)
      break;

    uint8_t* currentBlock = blocks[blockIndex];
    if(!currentBlock)
      break;

    size_t blockOffset = b % blockSize;
    size_t blockEnd = (blockIndex == blockCount - 1) ? (totalSize % blockSize) : 0;
    size_t availSize = blockSize - blockEnd - blockOffset;
    size_t sizeToWrite;
    
    if((useSize - bytesWrote) < availSize) {
      sizeToWrite = useSize - bytesWrote;
    }
    else {
      sizeToWrite = availSize;
    }

#if SIZE_T_MAX > 0xFFFFFFFFU
    if(sizeToWrite >= (1ull << 32))
      sizeToWrite = 0xFFFFFFFF;
#endif

    memset(&currentBlock[blockOffset], value, sizeToWrite);

    bytesWrote += sizeToWrite;
    totalSize += sizeToWrite;
    b += sizeToWrite;
  }

  PrimeAssert(bytesWrote <= useSize, "Set too many bytes.");
}

void* BlockBuffer::GetAddr(size_t offset) const {
  if(!blocks)
    return nullptr;

  size_t blockIndex = offset / blockSize;
  size_t blockOffset = offset % blockSize;
  if(blockIndex < blockCount) {
    uint8_t* result = blocks[blockIndex];
    if(result) {
      return result + blockOffset;
    }
  }

  return nullptr;
}

bool BlockBuffer::CanDirectCopy(const BlockBuffer& other) const {
  return blockCount == other.blockCount && blockSize == other.blockSize && blockAlignment == other.blockAlignment && totalSize == other.totalSize;
}

size_t BlockBuffer::CopyTo(void* p, size_t offset, size_t size) const {
  return Read(p, offset, size);
}

void* BlockBuffer::ConvertToBytes(size_t* size, uint32_t alignment) const {
  if(totalSize == 0 || blockSize == 0) {
    if(size)
      *size = 0;
    return nullptr;
  }

  size_t convertSize = size ? *size : totalSize;
  if(convertSize > totalSize)
    convertSize = totalSize;

  void* result = alignment > 0 ? memalign(alignment, totalSize) : malloc(totalSize);
  if(!result) {
    if(size)
      *size = 0;

    PrimeAssert(false, "Could not allocate output buffer.");

    return nullptr;
  }

  Read(result, 0, totalSize);

  if(size)
    *size = totalSize;

  return result;
}

void* BlockBuffer::ConsumeToBytes(size_t* size) {
  if(totalSize == 0 || blockSize == 0) {
    if(size)
      *size = 0;
    return nullptr;
  }

  size_t convertSize = size ? *size : totalSize;
  if(convertSize > totalSize)
    convertSize = totalSize;

  uint8_t* result = nullptr;
  size_t resultSize = 0;

  for(uint32_t i = 0; i < blockCount; i++) {
    void* block = blocks[i];
    if(block) {
      size_t currBlockSize = (resultSize + blockSize > totalSize) ? (totalSize - resultSize) : blockSize;
      if(currBlockSize > 0) {
        uint8_t* newResult = (uint8_t*) realloc(result, resultSize + currBlockSize);
        if(newResult) {
          result = newResult;
          memcpy(result + resultSize, block, currBlockSize);
          resultSize += currBlockSize;
          free(block);
        }
        else {
          PrimeAssert(false, "Could not reallocate result buffer.");
          break;
        }
      }
      else {
        break;
      }
    }
    else {
      break;
    }
  }

  free(blocks);
  blocks = nullptr;
  blockCount = 0;
  totalSize = 0;

  if(size)
    *size = resultSize;

  return result;
}

std::string BlockBuffer::ToString() const {
  size_t dataSize;
  void* data = ConvertToBytes(&dataSize);
  if(data) {
    std::string result;
    result.append((const char*) data, dataSize);
    PrimeSafeFree(data);
    return result;
  }

  return std::string();
}
