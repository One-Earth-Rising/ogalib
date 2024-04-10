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

#include <list>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

template <class T>
class Queue {
private:

  class ConstIterator;

  class Iterator {
  friend class ConstIterator;
  private:

    typename std::list<T>::iterator iter;

  public:

    Iterator() {}
    Iterator(const typename std::list<T>::iterator& stdIter): iter(stdIter) {}

    Iterator& operator=(const Iterator& other) {
      iter = other.iter;
      return *this;
    }

    bool operator==(const Iterator& other) const {
      return iter == other.iter;
    }

    bool operator==(const ConstIterator& other) const {
      return iter == other.iter;
    }

    bool operator!=(const Iterator& other) const {
      return iter != other.iter;
    }

    bool operator!=(const ConstIterator& other) const {
      return iter != other.iter;
    }

    Iterator operator+(ptrdiff_t offset) const {
      return Iterator(iter + offset);
    }

    Iterator operator+(size_t offset) const {
      return Iterator(iter + offset);
    }

    Iterator& operator++() {
      ++iter;
      return *this;
    }

    T& operator*() {
      return *iter;
    }

  };

  class ConstIterator {
  friend class Iterator;
  private:

    typename std::list<T>::const_iterator iter;

  public:

    ConstIterator() {}
    ConstIterator(const typename std::list<T>::const_iterator& stdIter): iter(stdIter) {}

    ConstIterator& operator=(const ConstIterator& other) {
      iter = other.iter;
      return *this;
    }

    bool operator==(const ConstIterator& other) const {
      return iter == other.iter;
    }

    bool operator==(const Iterator& other) const {
      return iter == other.iter;
    }

    bool operator!=(const ConstIterator& other) const {
      return iter != other.iter;
    }

    bool operator!=(const Iterator& other) const {
      return iter != other.iter;
    }

    ConstIterator operator+(ptrdiff_t offset) const {
      return ConstIterator(iter + offset);
    }

    ConstIterator operator+(size_t offset) const {
      return ConstIterator(iter + offset);
    }

    ConstIterator& operator++() {
      ++iter;
      return *this;
    }

    const T& operator*() const {
      return *iter;
    }

  };

private:

  std::list<T> queue;

public:

  Queue() {}

  Queue(const std::initializer_list<T>& v) {
    for(auto const& it: v) {
      queue.push_back(it);
    }
  }

  Iterator begin() {
    return Iterator(queue.begin());
  }

  Iterator end() {
    return Iterator(queue.end());
  }

  ConstIterator begin() const {
    return ConstIterator(queue.begin());
  }

  ConstIterator end() const {
    return ConstIterator(queue.end());
  }

  size_t GetCount() const {
    return queue.size();
  }

  bool Add(const T& t) {
    size_t count = GetCount();
    queue.push_back(t);
    return count + 1 == GetCount();
  }

  bool Remove(const T& t) {
    size_t count = GetCount();
    queue.remove(t);
    return count == GetCount() + 1;
  }

  Queue& Clear() {
    queue.clear();
    return *this;
  }

  bool Enqueue(const T& t) {
    return Add(t);
  }

  bool Dequeue(T& t) {
    if(GetCount() > 0) {
      t = queue.front();
      queue.pop_front();
      return true;
    }
    else {
      return false;
    }
  }

  bool Dequeue() {
    if(GetCount() > 0) {
      queue.pop_front();
      return true;
    }
    else {
      return false;
    }
  }

  T& GetFirst() {
    return queue.front();
  }

  bool PeekFirst(T& t) {
    if(GetCount() > 0) {
      t = queue.front();
      return true;
    }
    else {
      return false;
    }
  }

};

typedef Queue<PrimeId> IdQueue;
typedef Queue<void*> VoidPtrQueue;
typedef Queue<String> StringQueue;

};
