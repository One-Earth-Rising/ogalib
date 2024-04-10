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

#include <Prime/Content/Content.h>
#include <Prime/Graphics/Tex.h>
#include <Prime/Graphics/ArrayBuffer.h>
#include <Prime/Graphics/IndexBuffer.h>
#include <Prime/Enum/WrapMode.h>
#include <Prime/Enum/CollisionType.h>
#include <Prime/Enum/CollisionTypeParam.h>

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _ImagemapContentRectConvexPoint {
  f32 x;
  f32 y;

  _ImagemapContentRectConvexPoint():
    x(0.0f),
    y(0.0f) {

  }
} ImagemapContentRectConvexPoint;

typedef struct _ImagemapContentRectConvex {
  std::string name;
  size_t pointCount;
  f32 radius;
  ImagemapContentRectConvexPoint* points;
  CollisionType collisionType;
  CollisionTypeParam collisionTypeParam;
  bool circle;

  _ImagemapContentRectConvex():
    pointCount(0),
    radius(0.0f),
    points(nullptr),
    collisionType(CollisionType()),
    collisionTypeParam(CollisionTypeParam()) {

  }
} ImagemapContentRectConvex;

typedef struct _ImagemapContentRectPoint {
  std::string name;
  f32 x;
  f32 y;
  f32 z;

  _ImagemapContentRectPoint():
    x(0.0f),
    y(0.0f),
    z(0.0f) {

  }
} ImagemapContentRectPoint;

typedef struct _ImagemapContentRect {
  std::string name;
  u32 w;
  u32 h;
  u32 sx;
  u32 sy;
  u32 dw;
  u32 dh;
  f32 colorScaleR;
  f32 colorScaleG;
  f32 colorScaleB;
  f32 colorScaleA;
  size_t pointCount;
  size_t convexCount;
  ImagemapContentRectPoint* points;
  ImagemapContentRectConvex* convexes;
  bool colorScaleIsAvailable;

  _ImagemapContentRect():
    w(0),
    h(0),
    sx(0),
    sy(0),
    dw(0),
    dh(0),
    colorScaleR(0.0f),
    colorScaleG(0.0f),
    colorScaleB(0.0f),
    colorScaleA(0.0f),
    pointCount(0),
    convexCount(0),
    points(nullptr),
    convexes(nullptr),
    colorScaleIsAvailable(false) {

  }
} ImagemapContentRect;

typedef struct _ImagemapContentTexRect {
  u32 x;
  u32 y;
  u32 w;
  u32 h;

  _ImagemapContentTexRect():
    x(0),
    y(0),
    w(0),
    h(0) {

  }
} ImagemapContentTexRect;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class ImagemapContent: public Content {
private:

  refptr<Tex> tex;

  ImagemapContentRect* rects;
  size_t rectCount;
  Dictionary<std::string, size_t> rectLookup;
  ImagemapContentTexRect* texRects;

  refptr<ArrayBuffer> ab;
  refptr<IndexBuffer> ib;

  WrapMode wrapModeX;
  WrapMode wrapModeY;

public:

  size_t GetRectCount() const {return rectCount;}

public:

  refptr<Tex> GetTex() const {return tex;}

public:

  ImagemapContent();
  ~ImagemapContent();

public:

  bool Load(const json& data, const json& info) override;
  bool Load(const void* data, size_t dataSize, const json& info) override;
  virtual bool LoadFromBC(const void* data, size_t dataSize, const json& info);
  virtual bool LoadFromPNG(const void* data, size_t dataSize, const json& info);
  virtual bool LoadFromJPEG(const void* data, size_t dataSize, const json& info);

  virtual u32 GetRectW(size_t index = 0) const;
  virtual u32 GetRectH(size_t index = 0) const;
  virtual size_t GetRectIndex(const std::string& name);
  virtual const ImagemapContentRect* GetRect(const std::string& name, size_t* rectIndex = nullptr);
  virtual const ImagemapContentRect* GetRectByIndex(size_t index);
  virtual const ImagemapContentTexRect* GetTexRectByIndex(size_t index);
  virtual size_t GetRectPointCount(const std::string& rectName, size_t* rectIndex = nullptr);
  virtual const std::string& GetRectPointName(const std::string& rectName, size_t pointIndex, size_t* rectIndex = nullptr);
  virtual const ImagemapContentRectPoint* GetRectPoint(const std::string& rectName, const std::string& pointName, size_t* pointIndex = nullptr, size_t* rectIndex = nullptr);
  virtual const ImagemapContentRectPoint* GetRectPointByRectIndex(size_t rectIndex, const std::string& pointName, size_t* pointIndex = nullptr);

  virtual void Draw(size_t index = 0);

protected:

  virtual void CreateBuffers();

};

};
