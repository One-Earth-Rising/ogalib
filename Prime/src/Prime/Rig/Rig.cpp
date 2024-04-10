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

#include <Prime/Rig/Rig.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Rig::Rig():
vertexMin(Vec3(0.0f, 0.0f, 0.0f)),
vertexMax(Vec3(0.0f, 0.0f, 0.0f)) {

}

Rig::~Rig() {

}

void Rig::SetContent(Content* content) {
  SetContent(dynamic_cast<RigContent*>(content));
}

void Rig::SetContent(RigContent* content) {
  root = nullptr;

  this->content = content;

  if(!content)
    return;

  root = new RigChild();

  if(root) {
    auto nodes = content->GetChildren();
    for(auto node: *nodes) {
      if(node->autoActivate) {
        refptr rigObject = new RigChild();
        if(rigObject) {
          rigObject->InitFromNode(node);
          root->children.Add(rigObject);

          refptr initParam = new ContentNodeInitParam(rigObject, this, content, node);

          auto object = node->Activate();
          if(object) {
            rigObject->object = object;
            node->OnActivated(object, initParam);
          }

          node->OnRigSetContent(initParam);
        }
      }
    }
  }

  UpdateVertexSpan();
}

void Rig::Calc(f32 dt) {
  if(root) {
    root->Calc(dt);

    if(vertexMin.IsZero() && vertexMax.IsZero()) {
      UpdateVertexSpan();
    }
  }
}

void Rig::Draw() {
  if(root) {
    root->Draw();
  }
}

f32 Rig::GetUniformSize() const {
  const Vec3& vertexMin = GetVertexMin();
  const Vec3& vertexMax = GetVertexMax();

  f32 sizeX = vertexMax.x - vertexMin.x;
  f32 sizeY = vertexMax.y - vertexMin.y;
  f32 sizeZ = vertexMax.z - vertexMin.z;
  f32 size = max(max(sizeX, sizeY), sizeZ);

  return size;
}

void Rig::UpdateVertexSpan() {
  if(!root)
    return;

  vertexMin = Vec3(0.0f, 0.0f, 0.0f);
  vertexMax = Vec3(0.0f, 0.0f, 0.0f);

  Stack<refptr<RigChild>> children;
  root->GetAllChildren(children);

  for(auto child: children) {
    if(child->object->IsInstance<IMeasurable>()) {
      auto measurable = child->object->GetAs<IMeasurable>();

      Vec3 childVertexMin(0.0f, 0.0f, 0.0f);
      Vec3 childVertexMax(0.0f, 0.0f, 0.0f);

      childVertexMin = measurable->GetVertexMin();
      childVertexMax = measurable->GetVertexMax();

      if(!(childVertexMin.IsZero() && childVertexMax.IsZero())) {
        Stack<refptr<RigChild>> hierarchy;

        root->GetObjectHierarchyToChild(hierarchy, child);

        Mat44 mat;
        mat.LoadIdentity();
        for(auto& object: hierarchy) {
          mat = object->GetLocalTransform() * mat;
        }

        if(vertexMin.IsZero() && vertexMax.IsZero()) {
          vertexMin = Vec3(std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max());
          vertexMax = Vec3(std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest(), std::numeric_limits<f32>::lowest());
        }

        Vec3 vMin = mat * childVertexMin;
        Vec3 vMax = mat * childVertexMax;

        vertexMin.x = min(vertexMin.x, vMin.x);
        vertexMin.y = min(vertexMin.y, vMin.y);
        vertexMin.z = min(vertexMin.z, vMin.z);
        vertexMax.x = max(vertexMax.x, vMax.x);
        vertexMax.y = max(vertexMax.y, vMax.y);
        vertexMax.z = max(vertexMax.z, vMax.z);

        if(vertexMin.x > vertexMax.x) {
          std::swap(vertexMin.x, vertexMax.x);
        }
        if(vertexMin.y > vertexMax.y) {
          std::swap(vertexMin.y, vertexMax.y);
        }
        if(vertexMin.z > vertexMax.z) {
          std::swap(vertexMin.z, vertexMax.z);
        }
      }
    }
  }
}
