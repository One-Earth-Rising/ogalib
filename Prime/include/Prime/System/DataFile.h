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
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class DataFile {
private:

  const void* data;
  size_t dataSize;
  size_t pos;

public:

  DataFile(const void* data, size_t dataSize);
  ~DataFile();
  
public:

  int8_t ReadS8();
  int16_t ReadS16();
  int32_t ReadS32();
  int64_t ReadS64();

  uint8_t ReadU8();
  uint16_t ReadU16();
  uint32_t ReadU32();
  uint64_t ReadU64();

  float ReadF32();
  double ReadF64();

  uint32_t ReadU32V();
  int32_t ReadS32V();
  uint64_t ReadU64V();
  size_t ReadSizeV();
  bool ReadBool();
  char* ReadUTF8Data(uint32_t* readSize = nullptr);
  std::string ReadUTF8();
  size_t ReadBytes(void* p, size_t size);

  template <class T>
  T ReadEnum() {
    return (T) ReadU32V();
  }

  template <class T>
  T ReadEnumS32V() {
    return (T) ReadS32V();
  }

  void Read(int8_t& v);
  void Read(int16_t& v);
  void Read(int32_t& v);
  void Read(int64_t& v);

  void Read(uint8_t& v);
  void Read(uint16_t& v);
  void Read(uint32_t& v);
  void Read(uint64_t& v);

  void Read(float& v);
  void Read(double& v);
  void Read(bool& v);
  void Read(std::string& v);

  template <class T>
  void ReadEnum(T& t) {
    t = (T) ReadU32V();
  }

  template <class T>
  void ReadEnumS32(T& t) {
    t = (T) ReadS32V();
  }

  int64_t ReadPrimitiveSigned(uint32_t size);
  uint64_t ReadPrimitiveUnsigned(uint32_t size);

};

};
