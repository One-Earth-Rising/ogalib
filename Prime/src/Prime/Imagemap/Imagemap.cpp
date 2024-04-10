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

#include <Prime/Imagemap/Imagemap.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Imagemap::Imagemap():
rectIndex(PrimeNotFound),
filteringEnabled(true),
vertexMin(Vec3(0.0f, 0.0f, 0.0f)),
vertexMax(Vec3(0.0f, 0.0f, 0.0f)) {

}

Imagemap::~Imagemap() {

}

void Imagemap::SetContent(Content* content) {
  SetContent(dynamic_cast<ImagemapContent*>(content));
}

void Imagemap::SetContent(ImagemapContent* content) {
  rectIndex = PrimeNotFound;

  this->content = content;

  if(!content)
    return;

  if(content->GetRectCount() > 0) {
    SetRectByIndex(0);
  }
}

const ImagemapContentRect* Imagemap::GetRect() const {
  if(!HasContent())
    return nullptr;

  return content->GetRectByIndex(0);
}

void Imagemap::SetRect(const char* name) {
  SetRect(std::string(name));
}

void Imagemap::SetRect(const std::string& name) {
  if(!HasContent())
    return;

  rectIndex = content->GetRectIndex(name);
}

void Imagemap::SetRectByIndex(size_t index) {
  static const std::string originStr("origin");

  if(!HasContent())
    return;

  if(index >= content->GetRectCount()) {
    rectIndex = PrimeNotFound;
  }

  rectIndex = index;

  auto rect = content->GetRectByIndex(rectIndex);
  auto point = content->GetRectPointByRectIndex(rectIndex, originStr);

  if(point) {
    vertexMin = Vec3(-point->x, point->y - ((f32) rect->h), 0.0f);
    vertexMax = Vec3(((f32) rect->w) - point->x, point->y, 0.0f);
  }
  else {
    vertexMin = Vec3(0.0f, 0.0f, 0.0f);
    vertexMax = Vec3((f32) rect->w, (f32) rect->h, 0.0f);
  }
}

void Imagemap::SetFilteringEnabled(bool enabled) {
  filteringEnabled = enabled;
}

void Imagemap::Draw() {
  if(!HasContent())
    return;

  if(rectIndex == PrimeNotFound)
    return;

  auto tex = content->GetTex();

  if(tex) {
    if(!filteringEnabled) {
      tex->SetFilteringEnabled(false);
    }

    content->Draw(rectIndex);

    if(!filteringEnabled) {
      tex->SetFilteringEnabled(true);
    }
  }
}
