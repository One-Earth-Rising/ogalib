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

#include <set>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

template <class T>
class Set {
public:

  class ConstIterator;

  class Iterator {
  friend class ConstIterator;
  private:

    const std::set<T>& set;
    typename std::set<T>::iterator iter;

  public:

    Iterator(const std::set<T>& set, const typename std::set<T>::iterator& stdIter): set(set), iter(stdIter) {}
    Iterator(const Iterator& other): set(other.set), iter(other.iter) {}

    Iterator& operator=(const Iterator& other) {
      iter = other.iter;
      return *this;
    }

    operator bool() const {
      return iter != set.end();
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

    Iterator& operator++() {
      ++iter;
      return *this;
    }

    const T& operator*() const {
      return *iter;
    }

  };

  class ConstIterator {
  friend class Iterator;
  private:

    const std::set<T>& set;
    typename std::set<T>::const_iterator iter;

  public:

    ConstIterator(const std::set<T>& set, const typename std::set<T>::const_iterator& stdIter): set(set), iter(stdIter) {}
    ConstIterator(const ConstIterator& other): set(other.set), iter(other.iter) {}

    ConstIterator& operator=(const ConstIterator& other) {
      iter = other.iter;
      return *this;
    }

    operator bool() const {
      return iter != set.end();
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

    ConstIterator operator+(s64 offset) const {
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

  std::set<T> set;

public:

  Set() {}

  Set(const std::initializer_list<T>& v) {
    for(auto const& it: v) {
      set.insert(it);
    }
  }

  Iterator begin() {
    return Iterator(set, set.begin());
  }

  Iterator end() {
    return Iterator(set, set.end());
  }

  ConstIterator begin() const {
    return ConstIterator(set, set.begin());
  }

  ConstIterator end() const {
    return ConstIterator(set, set.end());
  }

  size_t GetCount() const {
    return set.size();
  }

  bool Add(const T& t) {
    size_t count = GetCount();
    set.insert(t);
    return count + 1 == GetCount();
  }

  bool Remove(const T& t) {
    size_t count = GetCount();
    set.erase(t);
    return count == GetCount() + 1;
  }

  void Clear() {
    set.clear();
  }

  Set<T>& Append(const Set<T>& other) {
    for(auto const& t: other) {
      Add(t);
    }
    return *this;
  }

  Iterator Find(T& t) {
    return Iterator(set, set.find(t));
  }

  ConstIterator Find(const T& t) const {
    return ConstIterator(set, set.find(t));
  }

  bool HasItem(const T& item) const {
    return Find(item) != end();
  }

};

};
