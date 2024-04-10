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

#include <Prime/System/DataFile.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

DataFile::DataFile(const void* data, size_t dataSize):
data(data),
dataSize(dataSize),
pos(0) {

}

DataFile::~DataFile() {

}

int8_t DataFile::ReadS8() {
  return (int8_t) ReadPrimitiveSigned(sizeof(int8_t));
}

int16_t DataFile::ReadS16() {
  return (int16_t) ReadPrimitiveSigned(sizeof(int16_t));
}

int32_t DataFile::ReadS32() {
  return (int32_t) ReadPrimitiveSigned(sizeof(int32_t));
}

int64_t DataFile::ReadS64() {
  return ReadPrimitiveSigned(sizeof(int64_t));
}

uint8_t DataFile::ReadU8() {
  return (uint8_t) ReadPrimitiveUnsigned(sizeof(uint8_t));
}

uint16_t DataFile::ReadU16() {
  return (uint16_t) ReadPrimitiveUnsigned(sizeof(uint16_t));
}

uint32_t DataFile::ReadU32() {
  return (uint32_t) ReadPrimitiveUnsigned(sizeof(uint32_t));
}

uint64_t DataFile::ReadU64() {
  return ReadPrimitiveUnsigned(sizeof(uint64_t));
}

float DataFile::ReadF32() {
  float result;
  size_t bytesRead = ReadBytes(&result, sizeof(float));
  return bytesRead == sizeof(float) ? result : 0.0f;
}

double DataFile::ReadF64() {
  double result;
  size_t bytesRead = ReadBytes(&result, sizeof(double));
  return bytesRead == sizeof(double) ? result : 0.0;
}

uint32_t DataFile::ReadU32V() {
  uint32_t result = 0;
  uint32_t v;

  v = ReadU8();
  result = (v & 0x7F);
  if(v & 0x80) {
    v = ReadU8();
    result |= (v & 0x7F) << 7;
    if(v & 0x80) {
      v = ReadU8();
      result |= (v & 0x7F) << 14;
      if(v & 0x80) {
        v = ReadU8();
        result |= (v & 0x7F) << 21;
        if(v & 0x80) {
          v = ReadU8();
          result |= (v & 0xF) << 28;
        }
      }
    }
  }

  return result;
}

int32_t DataFile::ReadS32V() {
  int32_t v = (int32_t) ReadU32V();
  return v;
}

uint64_t DataFile::ReadU64V() {
  uint64_t result = 0;
  uint64_t v;

  v = ReadU8();
  result = (v & 0x7F);
  if(v & 0x80) {
    v = ReadU8();
    result |= (v & 0x7F) << 7;
    if(v & 0x80) {
      v = ReadU8();
      result |= (v & 0x7F) << 14;
      if(v & 0x80) {
        v = ReadU8();
        result |= (v & 0x7F) << 21;
        if(v & 0x80) {
          v = ReadU8();
          result |= (v & 0x7F) << 28;
          if(v & 0x80) {
            v = ReadU8();
            result |= (v & 0x7F) << 35;
            if(v & 0x80) {
              v = ReadU8();
              result |= (v & 0x7F) << 42;
              if(v & 0x80) {
                v = ReadU8();
                result |= (v & 0x7F) << 49;
                if(v & 0x80) {
                  v = ReadU8();
                  result |= (v & 0x7F) << 56;
                  if(v & 0x80) {
                    v = ReadU8();
                    result |= (v & 0x1) << 63;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return result;
}

size_t DataFile::ReadSizeV() {
  if(sizeof(size_t) == 8) {
    return ReadU64V();
  }
  else if(sizeof(size_t) == 4) {
    return ReadU32V();
  }
  else {
    return ReadU64V();
  }
}

bool DataFile::ReadBool() {
  return ReadPrimitiveUnsigned(sizeof(uint8_t)) != 0;
}

char* DataFile::ReadUTF8Data(uint32_t* readSize) {
  uint32_t size = ReadU32V();
  if(readSize)
    *readSize = size;
  if(size) {
    char* result = (char*) malloc(size + 1);
    if(result) {
      size_t bytesRead = ReadBytes(result, size);
      result[bytesRead] = 0;
    }
    return result;
  }
  return NULL;
}

std::string DataFile::ReadUTF8() {
  char* buffer = ReadUTF8Data();
  if(buffer) {
    std::string result = buffer;
    if(buffer)
      free(buffer);
    return result;
  }

  return std::string();
}

size_t DataFile::ReadBytes(void* p, size_t size) {
  if(data != nullptr && dataSize > 0) {
    if(pos >= dataSize) {
      return 0;
    }
    else {
      size_t bytesRead = std::min(size, dataSize - pos);
      memcpy(p, &static_cast<const uint8_t*>(data)[pos], bytesRead);
      pos += bytesRead;
      return bytesRead;
    }
  }
  else {
    return 0;
  }
}

void DataFile::Read(int8_t& v) {
  v = ReadS8();
}

void DataFile::Read(int16_t& v) {
  v = ReadS16();
}

void DataFile::Read(int32_t& v) {
  v = ReadS32();
}

void DataFile::Read(int64_t& v) {
  v = ReadS64();
}

void DataFile::Read(uint8_t& v) {
  v = ReadU8();
}

void DataFile::Read(uint16_t& v) {
  v = ReadU16();
}

void DataFile::Read(uint32_t& v) {
  v = ReadU32();
}

void DataFile::Read(uint64_t& v) {
  v = ReadU64();
}

void DataFile::Read(float& v) {
  v = ReadF32();
}

void DataFile::Read(double& v) {
  v = ReadF64();
}

void DataFile::Read(bool& v) {
  v = ReadBool();
}

void DataFile::Read(std::string& v) {
  v = ReadUTF8();
}

int64_t DataFile::ReadPrimitiveSigned(uint32_t size) {
  int64_t result = 0;
  ReadBytes(&result, size);
  if(size == 1) {
    return *(int8_t*) &result;
  }
  else if(size == 2) {
    return *(int16_t*) &result;
  }
  else if(size == 4) {
    return *(int32_t*) &result;
  }
  else {
    return result;
  }
}

uint64_t DataFile::ReadPrimitiveUnsigned(uint32_t size) {
  size_t result = 0;
  ReadBytes(&result, size);
  if(size == 1) {
    return *(uint8_t*) &result;
  }
  else if(size == 2) {
    return *(uint16_t*) &result;
  }
  else if(size == 4) {
    return *(uint32_t*) &result;
  }
  else {
    return result;
  }
}
