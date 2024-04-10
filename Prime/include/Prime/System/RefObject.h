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

#include <Prime/Config.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class RefObject {
private:

  u32 _refCount;

public:

  u32 GetRefCount() const {return _refCount;}
  bool HasRefs() const {return _refCount > 0;}

public:

  RefObject();
  virtual ~RefObject();

public:

  template <class T>
  bool IsInstance() const {return dynamic_cast<const T*>(this) != nullptr;}

  template <class T>
  bool IsInstance() {return dynamic_cast<T*>(this) != nullptr;}

  template <class T>
  T* GetAs() const {return dynamic_cast<const T*>(this);}

  template <class T>
  T* GetAs() {return dynamic_cast<T*>(this);}

  virtual void IncRef();
  virtual void DecRef();
  virtual void WaitForNoRefs();

  void AddJob(std::function<void(Job&)> callback, std::function<void(Job&)> response, JobType type);
  void AddJob(std::function<void(Job&)> callback, std::function<void(Job&)> response, const json& data = json(), JobType type = JobType::Default);
  void GetContent(const std::string& uri, const std::function<void (Content*)>& callback);
  void GetContent(const std::string& uri, const json& info, const std::function<void (Content*)>& callback);
  void GetContentRaw(const std::string& uri, const std::function<void (const void*, size_t)>& callback);
  void GetContentRaw(const std::string& uri, const json& info, const std::function<void (const void*, size_t)>& callback);
  void SendURL(const std::string& url, const std::function<void(const json&)>& callback);
  void SendURL(const std::string& url, const json& params, const std::function<void(const json&)>& callback);

};

template <class T>
class refptr {
private:

  T* ptr;

public:

  refptr(T* ptr = nullptr): ptr(ptr) {
    if(ptr) {
      ptr->IncRef();
    }
  }

  refptr(const refptr<T>& other): ptr(other.ptr) {
    if(ptr) {
      ptr->IncRef();
    }
  }

  ~refptr() {
    if(ptr) {
      DecRef();
    }
  }

  operator T*() const {
    return ptr;
  }

  operator bool() const {
    return ptr != nullptr;
  }

  T* operator->() const {
    return ptr;
  }

  T& operator*() const {
    return *ptr;
  }

  bool operator==(const refptr& other) const {
    return ptr == other.ptr;
  }

  bool operator!=(const refptr& other) const {
    return ptr != other.ptr;
  }

  bool operator<(const refptr& other) const {
    return ptr < other.ptr;
  }

  bool operator==(T* p) const {
    return ptr == p;
  }

  bool operator!=(T* p) const {
    return ptr != p;
  }

  bool operator<(T* p) const {
    return ptr < p;
  }

  refptr& operator=(const refptr& other)  {
    if(other.ptr) {
      other.ptr->IncRef();
    }

    if(ptr) {
      DecRef();
    }

    ptr = other.ptr;

    return *this;
  }

  refptr& operator=(T* p)  {
    if(p) {
      p->IncRef();
    }

    if(ptr) {
      DecRef();
    }

    ptr = p;

    return *this;
  }

private:

  inline void DecRef() {
    if(ptr->GetRefCount() == 1) {
      ptr->DecRef();
      ptr = nullptr;
    }
    else {
      ptr->DecRef();
    }
  }

};

template <class T>
class RefArray: public RefObject {
public:

  class ConstIterator;

  class Iterator {
  private:

    RefArray<T>& array;
    size_t index;

  public:

    Iterator(RefArray<T>& array, size_t index = 0): array(array), index(index) {}

    Iterator& operator=(const Iterator& other) {
      PrimeAssert(&array == &other.array, "Cannot copy iterator from another structure.");
      index = other.index;
      return *this;
    }

    bool operator==(const Iterator& other) const {
      return index == other.index;
    }

    bool operator==(const ConstIterator& other) const {
      return index == other.index;
    }

    bool operator!=(const Iterator& other) const {
      return index != other.index;
    }

    bool operator!=(const ConstIterator& other) const {
      return index != other.index;
    }

    Iterator operator+(ptrdiff_t offset) const {
      return Iterator(index + offset);
    }

    Iterator operator+(size_t offset) const {
      return Iterator(index + offset);
    }

    Iterator& operator++() {
      if(index < array.GetCount())
        ++index;
      return *this;
    }

    refptr<T> operator*() {
      return array[index];
    }

  };

  class ConstIterator {
  private:

    RefArray<T>& array;
    size_t index;

  public:

    ConstIterator(const RefArray<T>& array, size_t index = 0): array(array), index(index) {}

    ConstIterator& operator=(const ConstIterator& other) {
      PrimeAssert(&array == &other.array, "Cannot copy iterator from another structure.");
      index = other.index;
      return *this;
    }

    bool operator==(const ConstIterator& other) const {
      return index == other.index;
    }

    bool operator==(const Iterator& other) const {
      return index == other.index;
    }

    bool operator!=(const ConstIterator& other) const {
      return index != other.index;
    }

    bool operator!=(const Iterator& other) const {
      return index != other.index;
    }

    ConstIterator operator+(ptrdiff_t offset) const {
      return ConstIterator(index + offset);
    }

    ConstIterator operator+(size_t offset) const {
      return ConstIterator(index + offset);
    }

    ConstIterator& operator++() {
      if(index < array.GetCount())
        ++index;
      return *this;
    }

    refptr<T> operator*() const {
      return array[index];
    }

  };

private:

  refptr<T>* items;
  size_t count;
  size_t assignedCount;

public:

  size_t GetCount() const {return count;}
  size_t IsFullyAssigned() const {return assignedCount;}

public:

  RefArray(size_t count = 0): items(nullptr), count(count), assignedCount(0) {
    if(count > 0) {
      items = new refptr<T>[count];
      if(items) {
        for(size_t i = 0; i < count; i++) {
          items[i] = nullptr;
        }
      }
      else {
        count = 0;
      }
    }
  }

  ~RefArray() {
    PrimeSafeDeleteArray(items);
  }

public:

  refptr<T> operator[](size_t index) const {
    if(index < count) {
      return items[index];
    }
    else {
      return nullptr;
    }
  }

  void Assign(T* item, size_t index)  {
    if(count == 0)
      return;

    if(index < count) {
      PrimeAssert(!items[index], "Item already assigned to RefArray: index = %zu", index);

      items[index] = item;

      if(assignedCount < count) {
        assignedCount++;
      }
      else {
        PrimeAssert(false, "Assigned too many items to RefArray.");
      }
    }
    else {
      PrimeAssert(false, "Invalid assignment index into RefArray.");
    }
  }

  Iterator begin() {
    return Iterator(*this, 0);
  }

  Iterator end() {
    return Iterator(*this, count);
  }

  ConstIterator begin() const {
    return ConstIterator(*this, 0);
  }

  ConstIterator end() const {
    return ConstIterator(*this, count);
  }

};

};

#if defined(__cplusplus) && !defined(__INTELLISENSE__)
namespace std {
  template<class T> struct hash<Prime::refptr<T>> {
    size_t operator()(const Prime::refptr<T>& t) const noexcept {
      return hash<intptr_t>()((intptr_t) (T*) t);
    }
  };
};
#endif
