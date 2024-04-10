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

#include <Prime/Rig/RigChild.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Content/ContentNode.h>
#include <Prime/Graphics/Graphics.h>
#include <Prime/Types/Quat.h>
#include <Prime/Interface/IProcessable.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

RigChild::RigChild() {
  transform.pos = Vec3(0.0f, 0.0f, 0.0f);
  transform.scale = Vec3(1.0f, 1.0f, 1.0f);
  transform.angle = Vec3(0.0f, 0.0f, 0.0f);
  transform.hflip = false;
  transform.vflip = false;
  transform.vertexMin = Vec3(0.0f, 0.0f, 0.0f);
  transform.vertexMax = Vec3(0.0f, 0.0f, 0.0f);
}

RigChild::~RigChild() {

}

void RigChild::Calc(f32 dt) {
  if(object) {
    if(object->IsInstance<IProcessable>()) {
      object->GetAs<IProcessable>()->Calc(dt);
    }
  }

  for(auto child: children) {
    if(child) {
      child->Calc(dt);
    }
  }
}

void RigChild::InitFromNode(refptr<ContentNode> node) {
  info.name = node->name;

  transform.pos = node->pos;
  transform.scale = node->scale;
  transform.angle = node->angle;
  transform.hflip = node->hflip;
  transform.vflip = node->vflip;

  effects.color = node->color;
}

Mat44 RigChild::GetLocalTransform() const {
  Mat44 mat;
  mat.LoadTranslation(transform.pos);
  mat.Multiply(Quat().ConvertFromEulerAnglesDeg(transform.angle).GetRotationMat44());
  mat.Scale(transform.hflip ? -transform.scale.x : transform.scale.x, transform.vflip ? -transform.scale.y : transform.scale.y);
  return mat;
}

void RigChild::GetAllChildren(Stack<refptr<RigChild>>& objects, bool recurse) {
  for(auto child: children) {
    objects.Add(child);
  }

  if(recurse) {
    for(auto child: children) {
      child->GetAllChildren(objects, recurse);
    }
  }
}

bool RigChild::GetObjectHierarchyToChild(Stack<refptr<RigChild>>& objects, refptr<RigChild> child) {
  if(child == this) {
    objects.Add(this);
    return true;
  }

  for(auto currChild: children) {
    if(currChild->GetObjectHierarchyToChild(objects, child)) {
      objects.Add(this);
      return true;
    }
  }

  return false;
}

void RigChild::Draw() {
  Graphics& g = PxGraphics;

  g.model.Push().Multiply(GetLocalTransform());

  if(object) {
    if(object->IsInstance<IProcessable>()) {
      object->GetAs<IProcessable>()->Draw();
    }
  }

  for(auto child: children) {
    if(child) {
      child->Draw();
    }
  }

  g.model.Pop();
}
