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
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

template <class K, class V>
class Dictionary {
public:

  class ConstIterator;

  class Iterator {
  friend class ConstIterator;
  private:

    const std::unordered_map<K, V>& map;
    typename std::unordered_map<K, V>::iterator iter;

  public:

    Iterator(const std::unordered_map<K, V>& map, const typename std::unordered_map<K, V>::iterator& stdIter): map(map), iter(stdIter) {}
    Iterator(const Iterator& other): map(other.map), iter(other.iter) {}

    operator bool() const {
      return iter != map.end();
    }

    Iterator& operator=(const Iterator& other) {
      PrimeAssert(&map == &other.map, "Cannot copy iterator from another structure.");
      iter = other.iter;
      return *this;
    }

    Iterator& operator++() {
      ++iter;
      return *this;
    }

    Iterator& operator*() {
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

    const K& key() const {return iter->first;}
    V& value() const {return iter->second;}

    template <class T>
    T GetEnumValue() const {return (T) (s64) iter->second;}

  };

  class ConstIterator {
  friend class Iterator;
  private:

    const std::unordered_map<K, V>& map;
    typename std::unordered_map<K, V>::const_iterator iter;

  public:

    ConstIterator(const std::unordered_map<K, V>& map, const typename std::unordered_map<K, V>::const_iterator& stdIter): map(map), iter(stdIter) {}
    ConstIterator(const Iterator& other): map(other.map), iter(other.iter) {}

    operator bool() const {
      return iter != map.end();
    }

    ConstIterator& operator=(const ConstIterator& other) {
      iter = other.iter;
      return *this;
    }

    ConstIterator& operator++() {
      ++iter;
      return *this;
    }

    ConstIterator& operator*() {
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

    const K& key() const {return iter->first;}
    const V& value() const {return iter->second;}

    template <class T>
    T GetEnumValue() const {return (T) (s64) iter->second;}

  };

private:

  std::unordered_map<K, V> map;

public:

  Dictionary() {}

  Dictionary(const std::initializer_list<std::pair<K, V> >& v) {
    for(auto const& it: v) {
      map[it.first] = it.second;
    }
  }

  Dictionary(const Dictionary<K, V>& other) {
    (void) operator=(other);
  }

  Dictionary& operator=(const Dictionary<K, V>& other) {
    for(auto const& it: other) {
      map[it.key()] = it.value();
    }

    return *this;
  }

  bool operator==(const Dictionary<K, V>& other) const {
    if(GetCount() != other.GetCount())
      return false;

    for(auto const& it: *this) {
      if(auto itFind = other.Find(it.key())) {
        if(!(itFind.value() == it.value()))
          return false;
      }
    }

    return true;
  }

  Iterator begin() {
    return Iterator(map, map.begin());
  }

  Iterator end() {
    return Iterator(map, map.end());
  }

  ConstIterator begin() const {
    return ConstIterator(map, map.begin());
  }

  ConstIterator end() const {
    return ConstIterator(map, map.end());
  }

  size_t GetCount() const {
    return map.size();
  }

  Dictionary& Clear() {
    map.clear();
    return *this;
  }

  V& operator[](const K& k) {
    return map[k];
  }

  const V& operator[](const K& k) const {
    return map.at(k);
  }

  Iterator Find(const K& k) {
    return Iterator(map, map.find(k));
  }

  ConstIterator Find(const K& k) const {
    return ConstIterator(map, map.find(k));
  }

  Iterator FindArrayIndex(const K& k) {
    auto result = Find(k);
    if(result)
      return result;

    return Iterator(map, map.find((f64) k));
  }

  ConstIterator FindArrayIndex(const K& k) const {
    auto result = Find(k);
    if(result)
      return result;

    return ConstIterator(map, map.find((f64) k));
  }

  bool HasKey(const K& k) const {
    return Find(k) != end();
  }

  bool Remove(const K& k) {
    return map.erase(k) > 0;
  }

  size_t GetBucket(const K& k) const {
    return map.bucket(k);
  }

};

};
