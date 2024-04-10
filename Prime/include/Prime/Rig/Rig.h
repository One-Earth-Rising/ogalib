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

#include <Prime/Interface/IProcessable.h>
#include <Prime/Interface/IMeasurable.h>
#include <Prime/Rig/RigContent.h>
#include <Prime/Rig/RigChild.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Rig: public RefObject, public IProcessable, public IMeasurable {
private:

  refptr<RigContent> content;
  refptr<RigChild> root;

  Vec3 vertexMin;
  Vec3 vertexMax;

public:

  refptr<RigContent> GetRigContent() const {return content;}
  bool HasContent() const {return (bool) content;}

  const Vec3& GetVertexMin() const override {return vertexMin;}
  const Vec3& GetVertexMax() const override {return vertexMax;}

public:

  Rig();
  ~Rig();

public:

  virtual void SetContent(Content* content);
  virtual void SetContent(RigContent* content);

  virtual void Calc(f32 dt);
  virtual void Draw();

  f32 GetUniformSize() const override;

protected:

  virtual void UpdateVertexSpan();

};

};
