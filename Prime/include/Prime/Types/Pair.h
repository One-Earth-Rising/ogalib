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

#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

template <class A, class B>
class Pair {
private:

  std::pair<A, B> pair;

public:

  Pair() {
    pair.first = A();
    pair.second = B();
  }

  virtual ~Pair() {}

  Pair(const A& a, const B& b) {
    pair.first = a;
    pair.second = b;
  }

  Pair(const std::initializer_list<std::pair<A, B> >& v) {
    for(auto const& it: v) {
      pair = v;
      break;
    }
  }

  A& first() {return pair.first;}
  B& second() {return pair.second;}

  const A& first() const {return pair.first;}
  const B& second() const {return pair.second;}

};

template <class A, class B>
inline bool operator==(const Pair<A, B>& v1, const Pair<A, B>& v2) {
  return v1.first() == v2.first() && v1.second() == v2.second();
}

template <class A, class B>
inline bool operator<(const Pair<A, B>& v1, const Pair<A, B>& v2) {
  return std::hash<Pair<A, B> >()(v1) < std::hash<Pair<A, B> >()(v2);
}

};

namespace std {
  // https://stackoverflow.com/questions/5889238/why-is-xor-the-default-way-to-combine-hashes
  template <class T>
  inline void hash_combine(std::size_t& seed, const T& val) {
    std::hash<T> hasher;
    seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }

  template <class A, class B>
  struct hash<Prime::Pair<A, B> > {
    size_t operator()(const Prime::Pair<A, B>& v) const noexcept {
      size_t seed = 0;
      hash_combine(seed, v.first());
      hash_combine(seed, v.second());
      return seed;
    }
  };
}
