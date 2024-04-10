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

#include <Prime/System/RefObject.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Rig;
class RigContent;
class RigChild;
class ContentNode;

class ContentNodeInitParam: public RefObject {
public:

  refptr<RigChild> obj;
  refptr<Rig> rig;
  refptr<RigContent> rigContent;
  refptr<ContentNode> node;
  refptr<ContentNode> parentNode;
  refptr<RigChild> parent;

public:

  ContentNodeInitParam(refptr<RigChild> obj, refptr<Rig> rig, refptr<RigContent> rigContent, refptr<ContentNode> node, refptr<ContentNode> parentNode = nullptr, refptr<RigChild> parent = nullptr);
  ContentNodeInitParam(const ContentNodeInitParam& other);
  ~ContentNodeInitParam();

  ContentNodeInitParam& operator=(const ContentNodeInitParam& other);

public:

  void GetContent(const std::string& uri, const std::function<void (Content*)>& callback);
  void GetContent(const std::string& uri, const json& info, const std::function<void (Content*)>& callback);
  void GetContentRaw(const std::string& uri, const std::function<void (const void*, size_t)>& callback);
  void GetContentRaw(const std::string& uri, const json& info, const std::function<void (const void*, size_t)>& callback);

};

};
