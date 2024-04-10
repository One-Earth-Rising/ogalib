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

#include <Prime/Imagemap/ImagemapContent.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/Graphics.h>
#include <png/png.h>
#include <png/pngstruct.h>
#include <png/pnginfo.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

typedef struct _ImagemapRectVertex {
  f32 x, y;
  f32 u, v;
} ImagemapRectVertex;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ImagemapContent::ImagemapContent():
tex(nullptr),
rects(nullptr),
rectCount(0),
wrapModeX(WrapMode()),
wrapModeY(WrapMode()) {

}

ImagemapContent::~ImagemapContent() {
  PrimeSafeDeleteArray(texRects);
  
  if(rects) {
    for(size_t i = 0; i < rectCount; i++) {
      ImagemapContentRect& rect = rects[i];

      PrimeSafeDeleteArray(rect.points);

      if(rect.convexes) {
        for(size_t j = 0; j < rect.convexCount; j++) {
          ImagemapContentRectConvex& convex = rect.convexes[j];
          PrimeSafeDeleteArray(convex.points);
        }

        PrimeSafeDeleteArray(rect.convexes);
      }
    }

    PrimeSafeDeleteArray(rects);
  }
}

bool ImagemapContent::Load(const json& data, const json& info) {
  if(!Content::Load(data, info))
    return false;

  if(!data.IsObject())
    return false;

  Stack<ImagemapContentRect*> parsedRects;
  Dictionary<ImagemapContentRect*, Stack<ImagemapContentRectPoint*>*> parsedRectPointsLookup;
  Stack<ImagemapContentTexRect*> parsedTexRects;
  Dictionary<std::string, ImagemapContentTexRect*> parsedTexRectLookupByName;

  if(auto it = data.find("wrapModeX")) {
    auto& value = it.value();
    if(value.IsNumber()) {
      wrapModeX = (WrapMode) value.GetInt();
    }
    else if(value.IsString()) {
      wrapModeX = GetEnumWrapModeFromString(it.GetString());
    }
  }

  if(auto it = data.find("wrapModeY")) {
    auto& value = it.value();
    if(value.IsNumber()) {
      wrapModeY = (WrapMode) value.GetInt();
    }
    else if(value.IsString()) {
      wrapModeY = GetEnumWrapModeFromString(it.GetString());
    }
  }

  if(auto itRects = data.find("rects")) {
    if(itRects.IsArray()) {
      for(auto& rect: itRects) {
        if(rect.IsObject()) {
          ImagemapContentRect* parsedRect = new ImagemapContentRect();
          if(parsedRect) {
            if(auto it = rect.find("name"))
              parsedRect->name = it.GetString();

            if(auto it = rect.find("w")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedRect->w = value.GetUint();
              }
            }

            if(auto it = rect.find("h")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedRect->h = value.GetUint();
              }
            }

            if(auto it = rect.find("sx")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedRect->sx = value.GetUint();
              }
            }

            if(auto it = rect.find("sy")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedRect->sy = value.GetUint();
              }
            }

            if(auto it = rect.find("dw")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedRect->dw = value.GetUint();
              }
            }

            if(auto it = rect.find("dh")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedRect->dh = value.GetUint();
              }
            }

            parsedRect->colorScaleR = 1.0f;
            parsedRect->colorScaleG = 1.0f;
            parsedRect->colorScaleB = 1.0f;
            parsedRect->colorScaleA = 1.0f;

            if(auto it = rect.find("colorScaleR")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedRect->colorScaleR = value.GetFloat();
              }
            }

            if(auto it = rect.find("colorScaleG")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedRect->colorScaleG = value.GetFloat();
              }
            }

            if(auto it = rect.find("colorScaleB")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedRect->colorScaleB = value.GetFloat();
              }
            }

            if(auto it = rect.find("colorScaleA")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedRect->colorScaleA = value.GetFloat();
              }
            }

            parsedRect->colorScaleIsAvailable = parsedRect->colorScaleR != 1.0f || parsedRect->colorScaleG != 1.0f || parsedRect->colorScaleB != 1.0f || parsedRect->colorScaleA != 1.0f;

            if(auto itRectPoints = rect.find("points")) {
              if(itRectPoints.IsArray()) {
                Stack<ImagemapContentRectPoint*>* parsedRectPoints = new Stack<ImagemapContentRectPoint*>();
                if(parsedRectPoints) {
                  parsedRectPointsLookup[parsedRect] = parsedRectPoints;

                  for(auto& rectPoint: itRectPoints) {
                    if(rectPoint.IsObject()) {
                      ImagemapContentRectPoint* parsedRectPoint = new ImagemapContentRectPoint();
                      if(parsedRectPoint) {
                        if(auto it = rectPoint.find("name"))
                          parsedRectPoint->name = it.GetString();

                        if(auto it = rectPoint.find("x")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedRectPoint->x = value.GetFloat();
                          }
                        }

                        if(auto it = rectPoint.find("y")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedRectPoint->y = value.GetFloat();
                          }
                        }

                        if(auto it = rectPoint.find("z")) {
                          auto& value = it.value();
                          if(value.IsNumber()) {
                            parsedRectPoint->z = value.GetFloat();
                          }
                        }

                        parsedRectPoints->Add(parsedRectPoint);
                      }
                    }
                  }
                }
              }
            }

            parsedRects.Add(parsedRect);
          }
        }
      }
    }
  }

  if(auto itTexRects = data.find("texRects")) {
    if(itTexRects.IsArray()) {
      for(auto& texRect: itTexRects) {
        if(texRect.IsObject()) {
          ImagemapContentTexRect* parsedTexRect = new ImagemapContentTexRect();
          if(parsedTexRect) {
            if(auto it = texRect.find("name"))
              parsedTexRectLookupByName[it.GetString()] = parsedTexRect;

            if(auto it = texRect.find("x")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedTexRect->x = value.GetUint();
              }
            }

            if(auto it = texRect.find("y")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedTexRect->y = value.GetUint();
              }
            }

            if(auto it = texRect.find("w")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedTexRect->w = value.GetUint();
              }
            }

            if(auto it = texRect.find("h")) {
              auto& value = it.value();
              if(value.IsNumber()) {
                parsedTexRect->h = value.GetUint();
              }
            }

            parsedTexRects.Add(parsedTexRect);
          }
        }
      }
    }
  }

  rectCount = parsedRects.GetCount();
  if(rectCount) {
    rects = new ImagemapContentRect[rectCount];

    for(size_t i = 0; i < rectCount; i++) {
      ImagemapContentRect& rect = rects[i];
      ImagemapContentRect* parsedRect = parsedRects[i];

      rect = *parsedRect;

      rectLookup[rect.name] = i;

      if(auto it = parsedRectPointsLookup.Find(parsedRect)) {
        Stack<ImagemapContentRectPoint*>* parsedRectPoints = it.value();

        rect.pointCount = parsedRectPoints->GetCount();
        if(rect.pointCount) {
          rect.points = new ImagemapContentRectPoint[rect.pointCount];

          for(size_t j = 0; j < rect.pointCount; j++) {
            ImagemapContentRectPoint& rectPoint = rect.points[j];
            ImagemapContentRectPoint* parsedRectPoint = (*parsedRectPoints)[j];

            rectPoint = *parsedRectPoint;

            PrimeSafeDelete(parsedRectPoint);
          }
        }

        PrimeSafeDelete(parsedRectPoints);
      }

      PrimeSafeDelete(parsedRect);
    }
  }

  size_t texRectCount = parsedTexRects.GetCount();

  if(rectCount) {
    texRects = new ImagemapContentTexRect[rectCount];

    for(auto it: parsedTexRectLookupByName) {
      const auto& name = it.key();
      ImagemapContentTexRect* parsedTexRect = it.value();
      if(parsedTexRect) {
        if(auto itRectIndex = rectLookup.Find(name)) {
          size_t rectIndex = itRectIndex.value();
          if(rectIndex < texRectCount) {
            ImagemapContentTexRect& texRect = texRects[rectIndex];
            texRect = *parsedTexRect;
          }
        }
      }
    }
  }

  if(texRectCount) {
    for(size_t i = 0; i < texRectCount; i++) {
      ImagemapContentTexRect* parsedTexRect = parsedTexRects[i];
      PrimeSafeDelete(parsedTexRect);
    }
  }

  if(auto it = data.find("imgPath")) {
    std::string imgPath = it.GetString();

    if(!imgPath.empty()) {
      new Job(nullptr, [=](Job& job) {
        GetContentRaw(imgPath, [=](const void* data, size_t dataSize) {
          tex = Tex::Create();
          tex->AddTexData("", std::string((const char*) data, dataSize));
        });
      });
    }
  }

  return true;
}

bool ImagemapContent::Load(const void* data, size_t dataSize, const json& info) {
  if(IsFormatBC(data, dataSize, info)) {
    return LoadFromBC(data, dataSize, info);
  }
  else if(IsFormatPNG(data, dataSize, info)) {
    return LoadFromPNG(data, dataSize, info);
  }
  else if(IsFormatJPEG(data, dataSize, info)) {
    return LoadFromJPEG(data, dataSize, info);
  }

  return false;
}

bool ImagemapContent::LoadFromBC(const void* data, size_t dataSize, const json& info) {
  if(data == nullptr || dataSize == 0)
    return false;

  u32 w = 0;
  u32 h = 0;

  if(auto itWidth = info.find("width")) {
    w = itWidth.GetUint();
  }
  else if(auto itWidth = info.find("w")) {
    w = itWidth.GetUint();
  }

  if(auto itHeight = info.find("height")) {
    h = itHeight.GetUint();
  }
  else if(auto itHeight = info.find("h")) {
    h = itHeight.GetUint();
  }

  if(w == 0 || h == 0)
    return false;

  std::string dataCopy((const char*) data, dataSize);

  const size_t rectIndex = 0;
  rectCount = 1;
  rects = new ImagemapContentRect[rectCount];
  if(rects) {
    ImagemapContentRect& rect = rects[rectIndex];

    rect.w = w;
    rect.h = h;
    rect.sx = 0;
    rect.sy = 0;
    rect.dw = w;
    rect.dh = h;

    rect.colorScaleR = 1.0f;
    rect.colorScaleG = 1.0f;
    rect.colorScaleB = 1.0f;
    rect.colorScaleA = 1.0f;

    rectLookup[rect.name] = rectIndex;
  }

  texRects = new ImagemapContentTexRect[rectCount];
  if(texRects) {
    ImagemapContentTexRect& texRect = texRects[rectIndex];
    texRect.x = 0;
    texRect.y = 0;
    texRect.w = w;
    texRect.h = h;
  }

  wrapModeX = WrapModeNone;
  wrapModeY = WrapModeNone;

  new Job(nullptr, [=](Job& job) {
    tex = Tex::Create();
    tex->AddTexData("", dataCopy, info);
  });

  return true;
}

bool ImagemapContent::LoadFromPNG(const void* data, size_t dataSize, const json& info) {
  if(data == nullptr || dataSize == 0)
    return false;

  TexData texData;

  if(!Tex::LoadPixelsFromPNG(data, dataSize, texData))
    return false;

  std::string dataCopy((const char*) data, dataSize);
  u32 w = texData.w;
  u32 h = texData.h;

  const size_t rectIndex = 0;
  rectCount = 1;
  rects = new ImagemapContentRect[rectCount];
  if(rects) {
    ImagemapContentRect& rect = rects[rectIndex];

    rect.w = w;
    rect.h = h;
    rect.sx = 0;
    rect.sy = 0;
    rect.dw = w;
    rect.dh = h;

    rect.colorScaleR = 1.0f;
    rect.colorScaleG = 1.0f;
    rect.colorScaleB = 1.0f;
    rect.colorScaleA = 1.0f;

    rectLookup[rect.name] = rectIndex;
  }

  texRects = new ImagemapContentTexRect[rectCount];
  if(texRects) {
    ImagemapContentTexRect& texRect = texRects[rectIndex];
    texRect.x = 0;
    texRect.y = 0;
    texRect.w = w;
    texRect.h = h;
  }

  wrapModeX = WrapModeNone;
  wrapModeY = WrapModeNone;

  new Job(nullptr, [=](Job& job) {
    tex = Tex::Create();
    tex->AddTexData("", texData);
  });

  return true;
}

bool ImagemapContent::LoadFromJPEG(const void* data, size_t dataSize, const json& info) {
  if(data == nullptr || dataSize == 0)
    return false;

  TexData texData;

  if(!Tex::LoadPixelsFromJPEG(data, dataSize, texData))
    return false;

  std::string dataCopy((const char*) data, dataSize);
  u32 w = texData.w;
  u32 h = texData.h;

  const size_t rectIndex = 0;
  rectCount = 1;
  rects = new ImagemapContentRect[rectCount];
  if(rects) {
    ImagemapContentRect& rect = rects[rectIndex];

    rect.w = w;
    rect.h = h;
    rect.sx = 0;
    rect.sy = 0;
    rect.dw = w;
    rect.dh = h;

    rect.colorScaleR = 1.0f;
    rect.colorScaleG = 1.0f;
    rect.colorScaleB = 1.0f;
    rect.colorScaleA = 1.0f;

    rectLookup[rect.name] = rectIndex;
  }

  texRects = new ImagemapContentTexRect[rectCount];
  if(texRects) {
    ImagemapContentTexRect& texRect = texRects[rectIndex];
    texRect.x = 0;
    texRect.y = 0;
    texRect.w = w;
    texRect.h = h;
  }

  wrapModeX = WrapModeNone;
  wrapModeY = WrapModeNone;

  new Job(nullptr, [=](Job& job) {
    tex = Tex::Create();
    tex->AddTexData("", texData);
  });

  return true;
}

u32 ImagemapContent::GetRectW(size_t index) const {
  PrimeAssert(index < rectCount, "Invalid rect index.");

  if(index < rectCount)
    return rects[index].w;
  else
    return 0;
}

u32 ImagemapContent::GetRectH(size_t index) const {
  PrimeAssert(index < rectCount, "Invalid rect index.");

  if(index < rectCount)
    return rects[index].h;
  else
    return 0;
}

size_t ImagemapContent::GetRectIndex(const std::string& name) {
  if(auto it = rectLookup.Find(name)) {
    return it.value();
  }
  else {
    return PrimeNotFound;
  }
}

const ImagemapContentRect* ImagemapContent::GetRect(const std::string& name, size_t* rectIndex) {
  if(auto it = rectLookup.Find(name)) {
    size_t foundRectIndex = it.value();
    if(rectIndex)
      *rectIndex = foundRectIndex;
    return &rects[foundRectIndex];
  }
  else {
    return nullptr;
  }
}

const ImagemapContentRect* ImagemapContent::GetRectByIndex(size_t index) {
  if(index == PrimeNotFound)
    return nullptr;
  else if(rectCount)
    return &rects[index % rectCount];
  else
    return nullptr;
}

const ImagemapContentTexRect* ImagemapContent::GetTexRectByIndex(size_t index) {
  if(index == PrimeNotFound) {
    return nullptr;
  }
  else {
    if(rectCount && texRects) {
      return &texRects[index % rectCount];
    }
  }

  return nullptr;
}

const ImagemapContentRectPoint* ImagemapContent::GetRectPoint(const std::string& rectName, const std::string& pointName, size_t* pointIndex, size_t* rectIndex) {
  size_t useRectIndex;
  const ImagemapContentRect* rect = GetRect(rectName, &useRectIndex);
  if(rect) {
    if(rectIndex)
      *rectIndex = useRectIndex;
    return GetRectPointByRectIndex(useRectIndex, pointName, pointIndex);
  }

  if(pointIndex)
    *pointIndex = PrimeNotFound;

  return nullptr;
}

const ImagemapContentRectPoint* ImagemapContent::GetRectPointByRectIndex(size_t rectIndex, const std::string& pointName, size_t* pointIndex) {
  if(rectCount) {
    const ImagemapContentRect* rect = &rects[rectIndex % rectCount];
    for(size_t i = 0; i < rect->pointCount; i++) {
      if(pointName == rect->points[i].name) {
        if(pointIndex)
          *pointIndex = i;
        return &rect->points[i];
      }
    }
  }

  if(pointIndex)
    *pointIndex = PrimeNotFound;

  return nullptr;
}

size_t ImagemapContent::GetRectPointCount(const std::string& rectName, size_t* rectIndex) {
  size_t useRectIndex;
  const ImagemapContentRect* rect = GetRect(rectName, &useRectIndex);
  if(rect) {
    if(rectIndex)
      *rectIndex = useRectIndex;
    return rect->pointCount;
  }

  return 0;
}

const std::string& ImagemapContent::GetRectPointName(const std::string& rectName, size_t pointIndex, size_t* rectIndex) {
  size_t useRectIndex;
  const ImagemapContentRect* rect = GetRect(rectName, &useRectIndex);
  if(rect) {
    if(rectIndex)
      *rectIndex = useRectIndex;

    if(pointIndex < rect->pointCount) {
      return rect->points[pointIndex].name;
    }
  }

  static const std::string noName;
  return noName;
}

void ImagemapContent::Draw(size_t index) {
  if(rects == nullptr || rectCount == 0 || texRects == nullptr)
    return;

  if(index >= rectCount)
    return;

  if(!ab || !ib) {
    CreateBuffers();
  }

  if(ab && ib) {
    Graphics& g = PxGraphics;
    g.Draw(ab, ib, index * 6, 6, tex);
  }
}

void ImagemapContent::CreateBuffers() {
  static const std::string originStr("origin");

  if(rects == nullptr || rectCount == 0 || texRects == nullptr)
    return;

  if(!tex)
    return;

  const TexData* texData = tex->GetTexData("");
  if(!texData || texData->format == TexFormatNone)
    return;

  size_t vertexCount = 4 * rectCount;
  size_t indexCount = 6 * rectCount;

  ImagemapRectVertex* vertices = (ImagemapRectVertex*) calloc(vertexCount, sizeof(ImagemapRectVertex));
  
  void* indices;
  IndexFormat indexFormat;
  if(vertexCount < 0x100) {
    indices = calloc(indexCount, sizeof(u8));
    indexFormat = IndexFormatSize8;
  }
  else if(vertexCount < 0x10000) {
    indices = calloc(indexCount, sizeof(u16));
    indexFormat = IndexFormatSize16;
  }
  else {
    indices = calloc(indexCount, sizeof(u32));
    indexFormat = IndexFormatSize32;
  }

  if(vertices && indices) {
    for(size_t i = 0; i < rectCount; i++) {
      const ImagemapContentRect& rect = rects[i];
      const ImagemapContentTexRect& texRect = texRects[i];
      ImagemapRectVertex* v = &vertices[i * 4];
      f32 u1 = tex->GetU("", (f32) texRect.x);
      f32 v2 = tex->GetV("", (f32) texRect.y);
      f32 u2 = tex->GetU("", (f32) (texRect.x + texRect.w));
      f32 v1 = tex->GetV("", (f32) (texRect.y + texRect.h));

      f32 originX = 0.0f;
      f32 originY = 0.0f;
      const ImagemapContentRectPoint* origin = GetRectPointByRectIndex(i, originStr);
      if(origin) {
        originX = -origin->x;
        originY = origin->y - rect.h;
      }

      f32 x1 = rect.sx + originX;
      f32 y1 = (f32) rect.h - (f32) rect.dh - (f32) rect.sy + originY;
      f32 x2 = (f32) (x1 + rect.dw);
      f32 y2 = (f32) (y1 + rect.dh);

      v[0].x = x1;
      v[0].y = y1;
      v[0].u = u1;
      v[0].v = v1;

      v[1].x = x2;
      v[1].y = y1;
      v[1].u = u2;
      v[1].v = v1;

      v[2].x = x1;
      v[2].y = y2;
      v[2].u = u1;
      v[2].v = v2;

      v[3].x = x2;
      v[3].y = y2;
      v[3].u = u2;
      v[3].v = v2;

      if(vertexCount < 0x100) {
        u8* index = &(static_cast<u8*>(indices))[i * 6];
        u8 iv = (u8) (i * 4);
        *index++ = iv;
        *index++ = iv + 1;
        *index++ = iv + 2;
        *index++ = iv + 1;
        *index++ = iv + 3;
        *index++ = iv + 2;
      }
      else if(vertexCount < 0x10000) {
        u16* index = &(static_cast<u16*>(indices))[i * 6];
        u16 iv = (u16) (i * 4);
        *index++ = iv;
        *index++ = iv + 1;
        *index++ = iv + 2;
        *index++ = iv + 1;
        *index++ = iv + 3;
        *index++ = iv + 2;
      }
      else {
        u32* index = &(static_cast<u32*>(indices))[i * 6];
        u32 iv = (u32) (i * 4);
        *index++ = iv;
        *index++ = iv + 1;
        *index++ = iv + 2;
        *index++ = iv + 1;
        *index++ = iv + 3;
        *index++ = iv + 2;
      }
    }

    ab = ArrayBuffer::Create(sizeof(ImagemapRectVertex), vertices, vertexCount, BufferPrimitiveTriangles);
    ab->LoadAttribute("vPos", sizeof(f32) * 2);
    ab->LoadAttribute("vUV", sizeof(f32) * 2);

    ib = IndexBuffer::Create(indexFormat, indices, indexCount);
  }

  PrimeSafeFree(indices);
  PrimeSafeFree(vertices);
}
