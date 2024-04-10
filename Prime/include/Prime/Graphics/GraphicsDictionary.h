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
#include <Prime/Types/Mat44.h>
#include <Prime/Types/Dictionary.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class GraphicsDictionaryKey {
public:

  std::string name;
  size_t arrayIndex;
  bool isArray;

public:

  GraphicsDictionaryKey(): arrayIndex(0), isArray(false) {}
  GraphicsDictionaryKey(const char* name): name(name), arrayIndex(0), isArray(false) {}
  GraphicsDictionaryKey(const std::string& name): name(name), arrayIndex(0), isArray(false) {}
  GraphicsDictionaryKey(const std::string& name, size_t arrayIndex): name(name), arrayIndex(arrayIndex), isArray(true) {}
  GraphicsDictionaryKey(const GraphicsDictionaryKey& other) {(void) operator=(other);}

  operator const char*() const {return name.c_str();}
  operator const std::string&() const {return name;}

  GraphicsDictionaryKey& operator=(const GraphicsDictionaryKey& other) {
    name = other.name;
    arrayIndex = other.arrayIndex;
    isArray = other.isArray;
    return *this;
  }

  GraphicsDictionaryKey& operator=(s32 value) {
    if(value == 0) {
      name.clear();
      arrayIndex = 0;
      isArray = false;
    }
    return *this;
  }

  bool operator==(const GraphicsDictionaryKey& other) const {
    return name == other.name && arrayIndex == other.arrayIndex && isArray == other.isArray;
  }

  bool operator<(const GraphicsDictionaryKey& other) const {
    if(name < other.name)
      return true;
    else if(name > other.name)
      return false;

    if(arrayIndex < other.arrayIndex)
      return true;
    else if(arrayIndex > other.arrayIndex)
      return false;
  
    return !isArray && other.isArray;
  }

};

typedef enum {
  GraphicsDictionaryValueTypeNone,
  GraphicsDictionaryValueTypeF32,
  GraphicsDictionaryValueTypeS32,
  GraphicsDictionaryValueTypeVec2,
  GraphicsDictionaryValueTypeVec3,
  GraphicsDictionaryValueTypeVec4,
  GraphicsDictionaryValueTypeMat44,
} GraphicsDictionaryValueType;

class GraphicsDictionaryValue {
private:

  GraphicsDictionaryValueType type;

public:

  union {
    Mat44 mat;
    s32 _s32;
  };

public:

  GraphicsDictionaryValue(): type(GraphicsDictionaryValueTypeNone) {}
  ~GraphicsDictionaryValue() {}
  GraphicsDictionaryValue(f32 value): type(GraphicsDictionaryValueTypeF32) {*this = value;}
  GraphicsDictionaryValue(s32 value): type(GraphicsDictionaryValueTypeS32) {*this = value;}
  GraphicsDictionaryValue(const Vec2& value): type(GraphicsDictionaryValueTypeVec2) {*this = value;}
  GraphicsDictionaryValue(const Vec3& value): type(GraphicsDictionaryValueTypeVec3) {*this = value;}
  GraphicsDictionaryValue(const Vec4& value): type(GraphicsDictionaryValueTypeVec4) {*this = value;}
  GraphicsDictionaryValue(const Mat44& value): type(GraphicsDictionaryValueTypeMat44) {*this = value;}

  operator f32() const {return GetF32();}
  operator s32() const {return GetS32();}
  operator Vec2() const {return GetVec2();}
  operator Vec3() const {return GetVec3();}
  operator Vec4() const {return GetVec4();}
  operator Mat44() const {return mat;}

  GraphicsDictionaryValue& operator=(const GraphicsDictionaryValue& other) {
    type = other.type;
    mat = other.mat;
    return *this;
  }

  GraphicsDictionaryValue& operator=(f32 value) {
    type = GraphicsDictionaryValueTypeF32;
    mat.e[0] = value;
    return *this;
  }

  GraphicsDictionaryValue& operator=(s32 value) {
    type = GraphicsDictionaryValueTypeS32;
    _s32 = value;
    return *this;
  }

  GraphicsDictionaryValue& operator=(const Vec2& value) {
    type = GraphicsDictionaryValueTypeVec2;
    mat.e[0] = value.x;
    mat.e[1] = value.y;
    return *this;
  }

  GraphicsDictionaryValue& operator=(const Vec3& value) {
    type = GraphicsDictionaryValueTypeVec3;
    mat.e[0] = value.x;
    mat.e[1] = value.y;
    mat.e[2] = value.z;
    return *this;
  }

  GraphicsDictionaryValue& operator=(const Vec4& value) {
    type = GraphicsDictionaryValueTypeVec4;
    mat.e[0] = value.x;
    mat.e[1] = value.y;
    mat.e[2] = value.z;
    mat.e[3] = value.w;
    return *this;
  }

  GraphicsDictionaryValue& operator=(const Mat44& value) {
    type = GraphicsDictionaryValueTypeMat44;
    mat = value;
    return *this;
  }

  bool operator==(const GraphicsDictionaryValue& other) const {
    if(type != other.type)
      return false;

    if(type == GraphicsDictionaryValueTypeS32)
      return _s32 == other._s32;
    else if(type == GraphicsDictionaryValueTypeF32)
      return mat.e[0] == other.mat.e[0];
    else if(type == GraphicsDictionaryValueTypeVec2)
      return mat.e[0] == other.mat.e[0] && mat.e[1] == other.mat.e[1];
    else if(type == GraphicsDictionaryValueTypeVec3)
      return mat.e[0] == other.mat.e[0] && mat.e[1] == other.mat.e[1] && mat.e[2] == other.mat.e[2];
    else if(type == GraphicsDictionaryValueTypeVec4)
      return mat.e[0] == other.mat.e[0] && mat.e[1] == other.mat.e[1] && mat.e[2] == other.mat.e[2] && mat.e[3] == other.mat.e[3];
    else
      return mat == other.mat;
  }

  GraphicsDictionaryValueType GetType() const {return type;}
  bool IsType(GraphicsDictionaryValueType type) const {return this->type == type;}

  f32 GetF32() const {return mat.e[0];}
  s32 GetS32() const {return _s32;}
  Vec2 GetVec2() const {return Vec2(mat.e[0], mat.e[1]);}
  Vec3 GetVec3() const {return Vec3(mat.e[0], mat.e[1], mat.e[2]);}
  Vec4 GetVec4() const {return Vec4(mat.e[0], mat.e[1], mat.e[2], mat.e[3]);}
  const Mat44& GetMat44() const {return mat;}

};

typedef Dictionary<GraphicsDictionaryKey, GraphicsDictionaryValue> GraphicsDictionary;

};

#if !defined(__INTELLISENSE__)
namespace std {
  template<> struct hash<Prime::GraphicsDictionaryKey> {
    size_t operator()(const Prime::GraphicsDictionaryKey& v) const noexcept {
      return std::hash<std::string>()(v.name) ^ ~v.arrayIndex;
    }
  };
};
#endif
