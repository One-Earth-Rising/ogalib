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
#include <Prime/Content/ContentNodeInitParam.h>
#include <Prime/Types/Vec3.h>
#include <Prime/Types/Color.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Rig;
class RigContent;

class ContentNode: public RefObject {
friend class Rig;
private:

  refptr<RefArray<ContentNode>> children;

public:

  std::string content;
  std::string name;

  Vec3 pos;
  Vec3 scale;
  Vec3 angle;

  bool hflip;
  bool vflip;

  Color color;

  bool autoActivate;

public:

  ContentNode();
  ~ContentNode();

public:

  template <class T>
  bool IsInstance() const {return dynamic_cast<const T*>(this) != nullptr;}

  template <class T>
  T* GetAs() const {return dynamic_cast<const T*>(this);}

  template <class T>
  T* GetAs() {return dynamic_cast<T*>(this);}

  virtual bool Load(const json& data, const json& info);

  virtual refptr<RefObject> Activate() const;
  virtual refptr<RefObject> Activate(const json& info) const;

  virtual void OnActivated(refptr<RefObject> object, refptr<ContentNodeInitParam> param);

  virtual void GetWalkReferences(Stack<std::string>& paths) const;

  void AddJob(std::function<void(Job&)> callback, std::function<void(Job&)> response, JobType type) = delete;
  void AddJob(std::function<void(Job&)> callback, std::function<void(Job&)> response, const json& data = json(), JobType type = JobType::Default) = delete;
  void GetContent(const std::string& uri, const std::function<void (Content*)>& callback) = delete;
  void GetContent(const std::string& uri, const json& info, const std::function<void (Content*)>& callback) = delete;
  void GetContentRaw(const std::string& uri, const std::function<void (const void*, size_t)>& callback) = delete;
  void GetContentRaw(const std::string& uri, const json& info, const std::function<void (const void*, size_t)>& callback) = delete;
  void SendURL(const std::string& url, const std::function<void(const json&)>& callback) = delete;
  void SendURL(const std::string& url, const json& params, const std::function<void(const json&)>& callback) = delete;

private:

  virtual void OnRigSetContent(refptr<ContentNodeInitParam> param);

};

};
