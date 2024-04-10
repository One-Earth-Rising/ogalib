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

#include <Prime/Skeleton/SkeletonNode.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Skeleton/Skeleton.h>
#include <Prime/Rig/Rig.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

SkeletonNode::SkeletonNode() {

}

SkeletonNode::~SkeletonNode() {

}

bool SkeletonNode::Load(const json& data, const json& info) {
  if(!ContentNode::Load(data, info))
    return false;

  if(!data.IsObject())
    return false;

  if(auto it = data.find("skinset"))
    skinset = it.GetString();

  if(auto it = data.find("action"))
    action = it.GetString();

  return true;
}

refptr<RefObject> SkeletonNode::Activate(const json& info) const {
  return new Skeleton();
}

void SkeletonNode::OnActivated(refptr<RefObject> object, refptr<ContentNodeInitParam> param) {
  ContentNode::OnActivated(object, param);

  auto skeleton = object->GetAs<Skeleton>();

  if(!content.empty()) {
    param->GetContent(content, [=](Content* content) {
      skeleton->SetContent(content);

      if(!skinset.empty()) {
        param->GetContent(skinset, [=](Content* content) {
          if(content) {
            refptr newSkinset = new Skinset();
            newSkinset->SetContent(content);
            skeleton->SetSkinset(newSkinset);
          }
        });
      }

      if(!action.empty()) {
        skeleton->SetAction(action);
      }
    });
  }
}
