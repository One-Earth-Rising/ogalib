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

#include <Prime/Imagemap/ImagemapContent.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Imagemap: public RefObject {
private:

  refptr<ImagemapContent> content;

  size_t rectIndex;
  bool filteringEnabled;

  Vec3 vertexMin;
  Vec3 vertexMax;

public:

  refptr<ImagemapContent> GetImagemapContent() const {return content;}
  bool HasContent() const {return (bool) content;}

  size_t GetRectIndex() const {return rectIndex;}

  const Vec3& GetVertexMin() const {return vertexMin;}
  const Vec3& GetVertexMax() const {return vertexMax;}

public:

  Imagemap();
  ~Imagemap();

public:

  virtual void SetContent(Content* content);
  virtual void SetContent(ImagemapContent* content);

  virtual const ImagemapContentRect* GetRect() const;

  virtual void SetRect(const char* name);
  virtual void SetRect(const std::string& name);
  virtual void SetRectByIndex(size_t index);

  virtual void SetFilteringEnabled(bool enabled);

  virtual void Draw();

};

};
