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

#include <Prime/Graphics/DeviceShader.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>
#include <Prime/Content/Content.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

RefObject::RefObject():
_refCount(0) {

}

RefObject::~RefObject() {

}

void RefObject::IncRef() {
  PxRequireInit;
  PxRequireMainThread;

  _refCount++;
}

void RefObject::DecRef() {
  PxRequireInit;
  PxRequireMainThread;

  if(_refCount > 0) {
    _refCount--;

    if(_refCount == 0) {
      delete this;
    }
  }
  else {
    PrimeAssert(false, "Released too many references.");
  }
}

void RefObject::WaitForNoRefs() {
  while(_refCount > 0) {
    PxEngine.ProcessJobs();
    Thread::Yield();
  }
}

void RefObject::AddJob(std::function<void(Job&)> callback, std::function<void(Job&)> response, JobType type) {
  IncRef();
  new Job(callback, [=](Job& job) {
    if(response) {
      response(job);
    }

    DecRef();
  }, type);
}

void RefObject::AddJob(std::function<void(Job&)> callback, std::function<void(Job&)> response, const json& data, JobType type) {
  IncRef();
  new Job(callback, [=](Job& job) {
    if(response) {
      response(job);
    }

    DecRef();
  }, data, type);
}

void RefObject::GetContent(const std::string& uri, const std::function<void (Content*)>& callback) {
  json info;
  GetContent(uri, info, callback);
}

void RefObject::GetContent(const std::string& uri, const json& info, const std::function<void (Content*)>& callback) {
  IncRef();

  refptr content = dynamic_cast<Content*>(this);
  if(content) {
    const std::string& contentURI = content->GetURI();
    Prime::GetContent(uri, info + json({{"_parentURI", contentURI}}), [=](Content* content) {
      callback(content);
      DecRef();
    });
  }
  else {
    Prime::GetContent(uri, info, [=](Content* content) {
      callback(content);
      DecRef();
    });
  }
}

void RefObject::GetContentRaw(const std::string& uri, const std::function<void (const void*, size_t)>& callback) {
  json info;
  GetContentRaw(uri, info, callback);
}

void RefObject::GetContentRaw(const std::string& uri, const json& info, const std::function<void (const void*, size_t)>& callback) {
  IncRef();

  refptr content = dynamic_cast<Content*>(this);
  if(content) {
    const std::string& contentURI = content->GetURI();
    Prime::GetContentRaw(uri, info + json({{"_parentURI", contentURI}}), [=](const void* data, size_t dataSize) {
      callback(data, dataSize);
      DecRef();
    });
  }
  else {
    Prime::GetContentRaw(uri, info, [=](const void* data, size_t dataSize) {
      callback(data, dataSize);
      DecRef();
    });
  }
}

void RefObject::SendURL(const std::string& url, const std::function<void(const json&)>& callback) {
  json params;
  SendURL(url, params, callback);
}

void RefObject::SendURL(const std::string& url, const json& params, const std::function<void(const json&)>& callback) {
  IncRef();
  Prime::SendURL(url, [=](const json& response) {
    callback(response);
    DecRef();
  });
}
