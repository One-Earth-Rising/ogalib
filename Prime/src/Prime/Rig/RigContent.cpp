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

#include <Prime/Rig/RigContent.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>
#include <Prime/Imagemap/ImagemapNode.h>
#include <Prime/Skeleton/SkeletonNode.h>
#include <Prime/Model/ModelNode.h>
#include <Prime/Rig/RigNode.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

RigContent::RigContent() {

}

RigContent::~RigContent() {

}

bool RigContent::Load(const json& data, const json& info) {
  if(!Content::Load(data, info))
    return false;

  if(!data.IsObject())
    return false;

  if(auto itNodes = data.find("nodes")) {
    if(itNodes.IsArray()) {
      size_t nodeCount = itNodes.size();
      auto newChildren = new RefArray<ContentNode>(itNodes.size());
      if(newChildren) {
        size_t nodeIndex = 0;
        for(auto& node: itNodes) {
          ContentNode* contentNode = nullptr;

          if(node.IsObject()) {
            if(auto it = node.find("_className")) {
              std::string className = it.GetString();

              if(className == "ContentNode") {
                contentNode = new ContentNode();
              }
              else if(className == "ImagemapNode") {
                contentNode = new ImagemapNode();
                contentNode->Load(node, info);
              }
              else if(className == "SkeletonNode") {
                contentNode = new SkeletonNode();
                contentNode->Load(node, info);
              }
              else if(className == "ModelNode") {
                contentNode = new ModelNode();
                contentNode->Load(node, info);
              }
              else if(className == "RigNode") {
                contentNode = new RigNode();
                contentNode->Load(node, info);
              }

              if(contentNode) {
                bool loadSuccess = contentNode->Load(node, info);
                if(!loadSuccess) {
                  delete contentNode;
                  contentNode = nullptr;
                }
              }
            }
          }

          new Job(nullptr, [=](Job& job) {
            newChildren->Assign(contentNode, nodeIndex);
            if(newChildren->IsFullyAssigned()) {
              children = newChildren;
            }
          });

          nodeIndex++;
        }
      }
      else {
        return false;
      }
    }
    else {
      return false;
    }
  }

  while(!children) {
    PxEngine.ProcessJobs();
    Thread::Yield();
  }

  return true;
}
