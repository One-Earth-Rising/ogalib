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

#include <Prime/Imagemap/ImagemapNode.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Imagemap/Imagemap.h>
#include <Prime/Rig/Rig.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ImagemapNode::ImagemapNode():
u1(0.0f),
v1(0.0f),
u2(0.0f),
v2(0.0f) {

}

ImagemapNode::~ImagemapNode() {

}

bool ImagemapNode::Load(const json& data, const json& info) {
  if(!ContentNode::Load(data, info))
    return false;

  if(!data.IsObject())
    return false;

  if(auto it = data.find("rect"))
    rect = it.GetString();

  if(auto it = data.find("u1")) {
    if(it.IsNumber()) {
      u1 = it.GetFloat();
    }
  }

  if(auto it = data.find("v1")) {
    if(it.IsNumber()) {
      v1 = it.GetFloat();
    }
  }

  if(auto it = data.find("u2")) {
    if(it.IsNumber()) {
      u2 = it.GetFloat();
    }
  }

  if(auto it = data.find("v2")) {
    if(it.IsNumber()) {
      v2 = it.GetFloat();
    }
  }

  return true;
}

refptr<RefObject> ImagemapNode::Activate(const json& info) const {
  return new Imagemap();
}

void ImagemapNode::OnActivated(refptr<RefObject> object, refptr<ContentNodeInitParam> param) {
  ContentNode::OnActivated(object, param);

  auto imagemap = object->GetAs<Imagemap>();

  if(!content.empty()) {
    param->GetContent(content, [=](Content* content) {
      imagemap->SetContent(content);

      if(!rect.empty()) {
        imagemap->SetRect(rect);
      }
    });
  }
}
