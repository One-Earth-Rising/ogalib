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

#include <Prime/Content/ContentNode.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Rig/Rig.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ContentNode::ContentNode() {

}

ContentNode::~ContentNode() {

}

bool ContentNode::Load(const json& data, const json& info) {
  content.clear();
  name.clear();

  pos = Vec3(0.0f, 0.0f, 0.0f);
  scale = Vec3(1.0f, 1.0f, 1.0f);
  angle = Vec3(0.0f, 0.0f, 0.0f);

  hflip = false;
  vflip = false;

  color = Color(1.0f, 1.0f, 1.0f, 1.0f);

  autoActivate = true;

  if(auto it = data.find("content"))
    content = it.GetString();

  if(auto it = data.find("name"))
    name = it.GetString();

  if(auto it = data.find("x")) {
    if(it.IsNumber()) {
      pos.x = it.GetFloat();
    }
  }

  if(auto it = data.find("y")) {
    if(it.IsNumber()) {
      pos.y = it.GetFloat();
    }
  }

  if(auto it = data.find("z")) {
    if(it.IsNumber()) {
      pos.z = it.GetFloat();
    }
  }

  if(auto it = data.find("scaleX")) {
    if(it.IsNumber()) {
      scale.x = it.GetFloat();
    }
  }

  if(auto it = data.find("scaleY")) {
    if(it.IsNumber()) {
      scale.y = it.GetFloat();
    }
  }

  if(auto it = data.find("scaleZ")) {
    if(it.IsNumber()) {
      scale.z = it.GetFloat();
    }
  }

  if(auto it = data.find("angleX")) {
    if(it.IsNumber()) {
      angle.x = it.GetFloat();
    }
  }

  if(auto it = data.find("angleY")) {
    if(it.IsNumber()) {
      angle.y = it.GetFloat();
    }
  }

  if(auto it = data.find("angleZ")) {
    if(it.IsNumber()) {
      angle.z = it.GetFloat();
    }
  }

  if(auto it = data.find("hflip")) {
    if(it.IsBool()) {
      hflip = it.GetBool();
    }
  }

  if(auto it = data.find("vflip")) {
    if(it.IsBool()) {
      vflip = it.GetBool();
    }
  }

  if(auto it = data.find("r")) {
    if(it.IsNumber()) {
      color.r = it.GetFloat();
    }
  }

  if(auto it = data.find("g")) {
    if(it.IsNumber()) {
      color.g = it.GetFloat();
    }
  }

  if(auto it = data.find("b")) {
    if(it.IsNumber()) {
      color.b = it.GetFloat();
    }
  }

  if(auto it = data.find("alpha")) {
    if(it.IsNumber()) {
      color.a = it.GetFloat();
    }
  }

  return true;
}

refptr<RefObject> ContentNode::Activate() const {
  json info;
  return Activate(info);
}

refptr<RefObject> ContentNode::Activate(const json& info) const {
  return nullptr;
}

void ContentNode::OnActivated(refptr<RefObject> object, refptr<ContentNodeInitParam> param) {

}

void ContentNode::GetWalkReferences(Stack<std::string>& paths) const {

}

void ContentNode::OnRigSetContent(refptr<ContentNodeInitParam> param) {
  if(children) {
    auto nodes = children;
    for(auto node: *nodes) {
      if(node->autoActivate) {
        refptr rigObject = new RigChild();
        if(rigObject) {
          rigObject->InitFromNode(node);
          param->obj->children.Add(rigObject);

          refptr initParam = new ContentNodeInitParam(rigObject, param->rig, param->rigContent, node, this, param->obj);

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
}
