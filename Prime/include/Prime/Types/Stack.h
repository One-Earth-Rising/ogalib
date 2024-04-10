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

#include <Prime/System/Random.h>
#include <vector>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

template <class T>
class Stack {
public:

  class ConstIterator;

  class Iterator {
  friend class ConstIterator;
  private:

    typename std::vector<T>::iterator iter;

  public:

    Iterator() {}
    Iterator(const typename std::vector<T>::iterator& stdIter): iter(stdIter) {}

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

    typename std::vector<T>::const_iterator iter;

  public:

    ConstIterator() {}
    ConstIterator(const typename std::vector<T>::const_iterator& stdIter): iter(stdIter) {}

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

  std::vector<T> stack;

public:

  Stack() {}

  Stack(const std::initializer_list<T>& v) {
    for(auto const& it: v) {
      stack.push_back(it);
    }
  }

  Iterator begin() {
    return Iterator(stack.begin());
  }

  Iterator end() {
    return Iterator(stack.end());
  }

  ConstIterator begin() const {
    return ConstIterator(stack.begin());
  }

  ConstIterator end() const {
    return ConstIterator(stack.end());
  }

  size_t GetCount() const {
    return stack.size();
  }

  bool Add(const T& t) {
    size_t count = GetCount();
    stack.push_back(t);
    return count + 1 == GetCount();
  }

  bool Remove(const T& t) {
    auto it = std::find(stack.begin(), stack.end(), t);
    if(it == stack.end()) {
      return false;
    }
    else {
      stack.erase(it);
      return true;
    }
  }

  void Clear() {
    stack.clear();
  }

  T& GetItem(size_t index) {
    return stack[index];
  }

  const T& GetItem(size_t index) const {
    return stack[index];
  }

  bool Push(const T& t) {
    return Add(t);
  }

  bool Pop(T& t) {
    if(GetCount() > 0) {
      t = stack.back();
      stack.pop_back();
      return true;
    }
    else {
      return false;
    }
  }

  bool Pop() {
    if(GetCount() > 0) {
      stack.pop_back();
      return true;
    }
    else {
      return false;
    }
  }

  ConstIterator Find(const T& t) const {
    return ConstIterator(std::find(stack.begin(), stack.end(), t));
  }

  bool HasItem(const T& item) const {
    return Find(item) != end();
  }

  T& operator[](size_t index) {
    return stack[index];
  }

  const T& operator[](size_t index) const {
    return stack[index];
  }

  void Sort() {
    std::sort(stack.begin(), stack.end());
  }

  void StableSort() {
    std::stable_sort(stack.begin(), stack.end());
  }

  void SortRange(size_t start, size_t end) {
    std::sort(stack.begin() + start, stack.begin() + end);
  }

  void StableSortRange(size_t start, size_t end) {
    std::stable_sort(stack.begin() + start, stack.begin() + end);
  }

  void Shuffle(RandomGenerator& rng) {
    size_t n = GetCount();
    if(n <= 1)
      return;
    for(size_t i = n - 1; i > 0; i--) {
      size_t r = rng() % (i + 1);
      T v = operator[](r);
      operator[](r) = operator[](i);
      operator[](i) = v;
      if(i == 0)
        break;
    }
  }

  void Shuffle2(RandomGenerator& rng) {
    std::shuffle(stack.begin(), stack.end(), rng);
  }

};

};
