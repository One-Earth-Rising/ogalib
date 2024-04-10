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

#include <Prime/System/DataFile.h>
#include <Prime/System/PrimePackFormatItem.h>

////////////////////////////////////////////////////////////////////////////////
// Enums
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef enum {
  PrimePackFormatErrorNone = 0,
  PrimePackFormatErrorFileNotFound,
  PrimePackFormatErrorOutOfMemory,
  PrimePackFormatErrorUnknownHeader,
  PrimePackFormatErrorUnknownVersion,
  PrimePackFormatErrorInvalidFileSize,
  PrimePackFormatErrorContentNone,
  PrimePackFormatErrorChunkNotFoundInPNG,
} PrimePackFormatError;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class PrimePackFormat {
private:

  std::string contentPath;
  BlockBuffer* ppfData;

  void* loadChunk;
  size_t loadChunkSize;

  uint32_t version;
  std::unordered_map<std::string, PrimePackFormatItem*> items;
  std::unordered_map<std::string, BlockBuffer*> addedItems;
  std::unordered_map<std::string, std::string> metadata;
  PrimePackFormatError error;

public:

  bool IsValid() const {return error == PrimePackFormatErrorNone;}
  bool HasItems() const {return items.size() > 0 || addedItems.size() > 0;}
  uint32_t GetVersion() const {return version;}
  const std::unordered_map<std::string, std::string>& GetMetadata() const {return metadata;}
  const std::string& GetContentPath() const {return contentPath;}
  PrimePackFormatError GetError() const {return error;}

public:

  PrimePackFormat();
  PrimePackFormat(void* data, size_t dataSize);
  PrimePackFormat(PrimePackFormatError error);
  virtual ~PrimePackFormat();

public:

  void InitFromData(const void* data, size_t dataSize);
  void SetLoadChunk(const void* chunk, size_t chunkSize);
  void SetContentPath(const std::string& contentPath);

  bool HasItem(const std::string& path) const;
  size_t GetItemCount() const;

  void GetItemPaths(std::vector<std::string>& paths) const;
  BlockBuffer* GetItemData(const std::string& path, size_t blockSize = 0) const;

  void AddItem(const std::string& path, void* data, size_t dataSize, bool replace = false);
  void AddItem(const std::string& path, const std::string& data, bool replace = false);

private:

  PrimePackFormatError ParseVersion1(DataFile& file);
  PrimePackFormatError ParseVersion2(DataFile& file);

};

};
