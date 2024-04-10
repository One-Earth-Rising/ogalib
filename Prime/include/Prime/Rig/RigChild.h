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

#include <Prime/Content/ContentNode.h>
#include <Prime/Types/Set.h>
#include <Prime/Types/Stack.h>
#include <Prime/Types/Vec3.h>
#include <Prime/Types/Mat44.h>
#include <Prime/Types/Color.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class RigChild: public RefObject {
public:

  struct {
    std::string name;
  } info;

  struct {
    Vec3 pos;
    Vec3 scale;
    Vec3 angle;
    bool hflip;
    bool vflip;
    Vec3 vertexMin;
    Vec3 vertexMax;
  } transform;

  struct {
    Color color;
  } effects;

  refptr<RefObject> object;
  Set<refptr<RigChild>> children;

public:

  RigChild();
  ~RigChild();

public:

  virtual void InitFromNode(refptr<ContentNode> node);

  virtual Mat44 GetLocalTransform() const;

  virtual void GetAllChildren(Stack<refptr<RigChild>>& objects, bool recurse = true);
  virtual bool GetObjectHierarchyToChild(Stack<refptr<RigChild>>& objects, refptr<RigChild> child);

  virtual void Calc(f32 dt);
  virtual void Draw();

};

};
