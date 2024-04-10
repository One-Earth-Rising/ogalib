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

#include <Prime/System/PrimePackFormat.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <png/png.h>
#include <png/pngstruct.h>
#include <png/pnginfo.h>
#include <zlib/zlib.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PPFBlockBufferBlockSize (512 * 1024)
#define PPFBlockBufferReadSize (2 * 1024 * 1024)

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

static const char PrimePackFormatHeader[] = {'\xE3', 'P', 'P', 'F', '\x0D', '\x0A', '\x01', '\0'};

static const std::unordered_map<std::string, std::string> PrimePackFormatEmptyMetadata;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

static void ReadPNGFromFile(png_structp png, png_bytep data, png_size_t size);
static int ReadPNGChunk(png_structp png, png_unknown_chunkp chunk);

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

PrimePackFormat::PrimePackFormat():
ppfData(nullptr),
loadChunk(nullptr),
loadChunkSize(0),
version(0),
error(PrimePackFormatErrorNone) {

}

PrimePackFormat::PrimePackFormat(void* data, size_t dataSize):
ppfData(nullptr),
loadChunk(nullptr),
loadChunkSize(0),
version(0),
error(PrimePackFormatErrorNone) {
  InitFromData(data, dataSize);
}

PrimePackFormat::PrimePackFormat(PrimePackFormatError error):
ppfData(nullptr),
loadChunk(nullptr),
loadChunkSize(0),
version(0),
error(error) {

}

PrimePackFormat::~PrimePackFormat() {
  for(auto it: addedItems) {
    BlockBuffer* blockBuffer = it.second;
    if(blockBuffer) {
      delete blockBuffer;
    }
  }

  for(auto it: items) {
    auto item = it.second;
    if(item) {
      delete item;
    }
  }

  if(loadChunk)
    free(loadChunk);

  if(ppfData)
    delete ppfData;
}

bool PrimePackFormat::HasItem(const std::string& path) const {
  if(addedItems.find(path) != addedItems.end()) {
    return true;
  }

  if(items.find(path) != items.end()) {
    return true;
  }

  return false;
}

size_t PrimePackFormat::GetItemCount() const {
  return items.size() + addedItems.size();
}

void PrimePackFormat::GetItemPaths(std::vector<std::string>& paths) const {
  for(auto it: addedItems) {
    const std::string& path = it.first;
    if(std::find(paths.begin(), paths.end(), path) == paths.end()) {
      paths.push_back(path);
    }
  }

  for(auto it: items) {
    const std::string& path = it.first;
    if(std::find(paths.begin(), paths.end(), path) == paths.end()) {
      paths.push_back(path);
    }
  }
}

BlockBuffer* PrimePackFormat::GetItemData(const std::string& path, size_t blockSize) const {
  if(error) {
    return nullptr;
  }
  
  auto itAddedItem = addedItems.find(path);
  if(itAddedItem != addedItems.end()) {
    BlockBuffer* blockBuffer = itAddedItem->second;
    if(blockBuffer) {
      return new BlockBuffer(*blockBuffer);
    }
    else {
      return nullptr;
    }
  }

  auto itItem = items.find(path);
  if(itItem == items.end())
    return nullptr;

  PrimePackFormatItem* itemPtr = itItem->second;
  if(!itemPtr)
    return nullptr;

  PrimePackFormatItem& item = *itemPtr;
  BlockBuffer* blockBuffer = nullptr;

  if(item.size > 0) {
    size_t ppfBlockBufferBlockSize = PPFBlockBufferBlockSize;
    size_t ppfBlockBufferReadSize = PPFBlockBufferReadSize;

    size_t useBlockSize = blockSize == 0 ? ppfBlockBufferBlockSize : blockSize;
    if(useBlockSize > item.size)
      useBlockSize = item.size;

    blockBuffer = new BlockBuffer(useBlockSize);
    if(blockBuffer) {
      size_t readBufferSize = std::min(ppfBlockBufferReadSize, item.size);
      uint8_t* buffer = (uint8_t*) malloc(readBufferSize);
      if(buffer) {
        size_t bytesRemaining = item.size;
        size_t bytesRead;
        do {
          size_t bytesToRead = std::min(readBufferSize, bytesRemaining);
          bytesRead = ppfData->Read(buffer, item.offset + (item.size - bytesRemaining), bytesToRead);
          if(bytesRead > 0) {
            blockBuffer->Append(buffer, bytesRead);
            bytesRemaining -= bytesRead;
          }
          else {
            break;
          }
        }
        while(bytesRemaining > 0);
        free(buffer);
      }
      else {
        if(blockBuffer) {
          delete blockBuffer;
        }
      }

      if(blockBuffer) {
        if(item.compression == 1) {
          BlockBuffer* uncompressedBlockBuffer = new BlockBuffer(useBlockSize);
          void* bufferIn = malloc(ppfBlockBufferReadSize);
          void* bufferOut = malloc(ppfBlockBufferReadSize);

          z_stream stream;
          stream.zalloc = Z_NULL;
          stream.zfree = Z_NULL;
          stream.opaque = Z_NULL;

          inflateInit(&stream);

          size_t bytesRead = 0;
          size_t bytesUncompressed = 0;
          while(bytesUncompressed < item.size) {
            size_t readSize = blockBuffer->CopyTo(bufferIn, bytesRead, ppfBlockBufferReadSize);
            if(readSize > 0) {
              bool inflateFailure = false;
              stream.avail_in = (uInt) readSize;
              stream.next_in = (Bytef*) bufferIn;

              do {
                stream.avail_out = (uInt) ppfBlockBufferReadSize;
                stream.next_out = (Bytef*) bufferOut;

                int inflateResult = inflate(&stream, Z_NO_FLUSH);
                if(inflateResult == Z_OK || inflateResult == Z_STREAM_END) {
                  size_t uncompressedSize = ppfBlockBufferReadSize - stream.avail_out;
                  uncompressedBlockBuffer->Append(bufferOut, uncompressedSize);
                  bytesUncompressed += uncompressedSize;
                }
                else {
                  inflateFailure = true;
                  break;
                }
              }
              while(stream.avail_in > 0 && stream.avail_out == 0);

              if(inflateFailure)
                break;

              bytesRead += readSize;
            }
            else {
              break;
            }
          }

          inflateEnd(&stream);

          free(bufferOut);
          free(bufferIn);
          delete blockBuffer;

          blockBuffer = uncompressedBlockBuffer;
        }
      }
    }
  }

  return blockBuffer;
}

void PrimePackFormat::AddItem(const std::string& path, void* data, size_t dataSize, bool replace) {
  auto itAddedItem = addedItems.find(path);
  if(itAddedItem != addedItems.end()) {
    auto existingData = itAddedItem->second;
    if(existingData) {
      if(replace) {
        return;
      }
      else {
        if(existingData) {
          delete existingData;
        }
      }
    }
  }

  addedItems.erase(path);

  if(data && dataSize > 0) {
    size_t ppfBlockBufferBlockSize = PPFBlockBufferBlockSize;

    size_t useBlockSize = ppfBlockBufferBlockSize;
    if(useBlockSize > dataSize)
      useBlockSize = dataSize;

    BlockBuffer* blockBuffer = new BlockBuffer(useBlockSize);
    if(blockBuffer) {
      size_t bytesAppended = blockBuffer->Append(data, dataSize);
      if(bytesAppended == dataSize) {
        addedItems[path] = blockBuffer;
      }
      else {
        if(blockBuffer) {
          delete blockBuffer;
        }
      }
    }
  }
}

void PrimePackFormat::AddItem(const std::string& path, const std::string& data, bool replace) {
  AddItem(path, (void*) data.c_str(), data.size(), replace);
}

void PrimePackFormat::InitFromData(const void* data, size_t dataSize) {
  if(data == nullptr || dataSize == 0)
    return;

  bool dataIsPNG = false;

  DataFile* file = new DataFile(data, dataSize);
  if(file) {
    if(dataSize >= 8 && png_sig_cmp((png_bytep) data, 0, 8) == 0) {
      png_structp png;
      png_infop pngInfo = nullptr;
      png_infop pngEndInfo = nullptr;

      dataIsPNG = true;

      png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
      if(png) {
        pngInfo = png_create_info_struct(png);
        if(pngInfo) {
          pngEndInfo = png_create_info_struct(png);
          if(pngEndInfo) {
            png_byte chunkList[] = {
              "cPPF",
            };

            if(setjmp(png_jmpbuf(png))) {
              goto endReadPNG;
            }

            png_set_keep_unknown_chunks(png, PNG_HANDLE_CHUNK_IF_SAFE, chunkList, 1);
            png_set_read_user_chunk_fn(png, this, ReadPNGChunk);
            png_set_read_fn(png, file, ReadPNGFromFile);
            png_set_option(png, PNG_SKIP_sRGB_CHECK_PROFILE, PNG_OPTION_OFF);

            if(setjmp(png_jmpbuf(png))) {
              goto endReadPNG;
            }

            png_read_info(png, pngInfo);

            uint32_t h = pngInfo->height;
            for(uint32_t i = 0; i < h; i++) {
              png_read_row(png, nullptr, nullptr);
            }

            if(setjmp(png_jmpbuf(png))) {
              goto endReadPNG;
            }

            png_read_end(png, pngEndInfo);
          }
        }
      }

endReadPNG:
      png_destroy_read_struct(&png, &pngInfo, &pngEndInfo);
    }

    delete file;
  }

  if(dataIsPNG && !loadChunk) {
    error = PrimePackFormatErrorChunkNotFoundInPNG;
    return;
  }

  if(loadChunk) {
    size_t chunkSize = loadChunkSize;
    void* chunk = malloc(chunkSize);
    if(chunk) {
      memcpy(chunk, loadChunk, chunkSize);
    }
    else {
      chunkSize = 0;
    }

    SetLoadChunk(nullptr, 0);

    if(chunk) {
      InitFromData(chunk, chunkSize);
      if(chunk) {
        free(chunk);
      }
    }
    else {
      error = PrimePackFormatErrorOutOfMemory;
    }

    return;
  }

  file = new DataFile(data, dataSize);
  if(file) {
    do {
      char header[sizeof(PrimePackFormatHeader)];
      uint64_t headerSize = file->ReadBytes(header, sizeof(header));

      if(headerSize != sizeof(header)) {
        error = PrimePackFormatErrorUnknownHeader;
        break;
      }

      if(memcmp(header, PrimePackFormatHeader, sizeof(header)) != 0) {
        error = PrimePackFormatErrorUnknownHeader;
        break;
      }

      version = file->ReadU32V();

      if(version == 1) {
        error = ParseVersion1(*file);
        if(error) {
          break;
        }
      }
      else if(version == 2) {
        uint64_t fileSize = file->ReadU64();
        if(fileSize != dataSize) {
          error = PrimePackFormatErrorInvalidFileSize;
        }
        else {
          error = ParseVersion2(*file);
          if(error) {
            break;
          }
        }
      }
      else {
        error = PrimePackFormatErrorUnknownVersion;
        break;
      }

      // At this point, mark the format as valid.
      // Exit this block.
      break;
    }
    while(true);

    delete file;
  }
  else {
    error = PrimePackFormatErrorFileNotFound;
  }

  if(error) {
    version = 0;
    for(auto it: items) {
      auto item = it.second;
      if(item) {
        delete item;
      }
    }
    items.clear();
    metadata.clear();
  }
  else {
    size_t ptfBlockBufferBlockSize = PPFBlockBufferBlockSize;
    size_t useBlockSize = ptfBlockBufferBlockSize;
    if(useBlockSize > dataSize)
      useBlockSize = dataSize;

    ppfData = new BlockBuffer(useBlockSize);
    if(ppfData) {
      ppfData->Append(data, dataSize);
    }
    else {
      error = PrimePackFormatErrorOutOfMemory;
    }

    if(error) {
      version = 0;
      for(auto it: items) {
        auto item = it.second;
        if(item) {
          delete item;
        }
      }
      items.clear();
      metadata.clear();
    }
  }
}

void PrimePackFormat::SetLoadChunk(const void* chunk, size_t chunkSize) {
  if(loadChunk) {
    free(loadChunk);
    loadChunk = nullptr;
  }
  loadChunkSize = 0;

  if(chunk && chunkSize > 0) {
    loadChunkSize = chunkSize;
    loadChunk = malloc(loadChunkSize);
    if(loadChunk) {
      memcpy(loadChunk, chunk, loadChunkSize);
    }
    else {
      loadChunkSize = 0;
    }
  }
}

void PrimePackFormat::SetContentPath(const std::string& contentPath) {
  this->contentPath = contentPath;
}

PrimePackFormatError PrimePackFormat::ParseVersion1(DataFile& file) {
  uint32_t metadataCount = file.ReadU32V();

  for(uint32_t i = 0; i < metadataCount; i++) {
    std::string name;
    std::string value;
    file.Read(name);
    file.Read(value);
    metadata[name] = value;
  }

  uint32_t itemCount = file.ReadU32V();
  if(itemCount > 0) {
    for(uint32_t i = 0; i < itemCount; i++) {
      PrimePackFormatItem* itemPtr = new PrimePackFormatItem();
      if(itemPtr) {
        PrimePackFormatItem& item = *itemPtr;

        file.Read(item.path);

        item.size = file.ReadU32V();
        item.binaryFormat = file.ReadU32V();
        item.compression = file.ReadU32V();
        item.dataSize  = file.ReadU32V();
        item.offset = file.ReadU32();

        uint32_t itemMetadataCount = file.ReadU32V();

        for(uint32_t j = 0; j < itemMetadataCount; j++) {
          std::string name;
          std::string value;
          file.Read(name);
          file.Read(value);
          item.metadata[name] = value;
        }

        auto itItem = items.find(item.path);
        if(itItem != items.end()) {
          PrimePackFormatItem* existingItem = itItem->second;
          if(existingItem) {
            delete existingItem;
          }
        }

        items[item.path] = itemPtr;
      }
    }
  }

  return PrimePackFormatErrorNone;
}

PrimePackFormatError PrimePackFormat::ParseVersion2(DataFile& file) {
  uint64_t metadataCount = file.ReadU64V();

  for(uint64_t i = 0; i < metadataCount; i++) {
    std::string name;
    std::string value;
    file.Read(name);
    file.Read(value);
    metadata[name] = value;
  }

  uint64_t itemCount = file.ReadU64V();
  if(itemCount > 0) {
    for(uint64_t i = 0; i < itemCount; i++) {
      PrimePackFormatItem* itemPtr = new PrimePackFormatItem();
      if(itemPtr) {
        PrimePackFormatItem& item = *itemPtr;

        file.Read(item.path);

        item.size = file.ReadU64V();
        item.binaryFormat = file.ReadU32V();
        item.compression = file.ReadU32V();
        item.dataSize  = file.ReadU64V();
        item.offset = file.ReadU64();

        uint64_t itemMetadataCount = file.ReadU64V();

        for(uint64_t j = 0; j < itemMetadataCount; j++) {
          std::string name;
          std::string value;
          file.Read(name);
          file.Read(value);
          item.metadata[name] = value;
        }

        auto itItem = items.find(item.path);
        if(itItem != items.end()) {
          PrimePackFormatItem* existingItem = itItem->second;
          if(existingItem) {
            delete existingItem;
          }
        }

        items[item.path] = itemPtr;
      }
    }
  }

  return PrimePackFormatErrorNone;
}

void ReadPNGFromFile(png_structp png, png_bytep data, png_size_t size) {
  DataFile* file = (DataFile*) png_get_io_ptr(png);
  file->ReadBytes(data, size);
}

int ReadPNGChunk(png_structp png, png_unknown_chunkp chunk) {
  PrimePackFormat* ppf = (PrimePackFormat*) png_get_user_chunk_ptr(png);
  ppf->SetLoadChunk(chunk->data, chunk->size);
  return 1;
}
