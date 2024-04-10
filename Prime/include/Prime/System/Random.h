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

#include <random>

PrimeExternCBegin
#include <tinymt/tinymt.h>
PrimeExternCEnd

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

struct uniform_u32_distribution {
  const u32 A, B;

  struct param_type {
    const u32 A, B;

    param_type(u32 aa, u32 bb): A(aa), B(bb) {}
  };

  explicit uniform_u32_distribution(const u32 a = 0, const u32 b = 0xFFFFFFFF): A(a), B(b) {
    PrimeAssert(B >= A, "Invalid random range.");
  }

  explicit uniform_u32_distribution(const param_type& params): A(params.A), B(params.B) {
    PrimeAssert(B >= A, "Invalid random range.");
  }

  template <class Generator>
  u32 operator()(Generator& g) const {
    return rnd(g, A, B);
  }

  template <class Generator>
  u32 operator()(Generator& g, const param_type& params) const {
    return rnd(g, params.A, params.B);
  }

  u32 a() const {
    return A;
  }

  u32 b() const {
    return B;
  }

  private:
    template <class Generator>
    u32 rnd(Generator& g, const u32 a, const u32 b) const {
    if(a == 0 && b == 0xFFFFFFFF) {
      return g();
    }
    else {
      const u32 range = b - a + 1;
      return (g() % range) + a;
    }
  }
};

struct uniform_s32_distribution {
const s32 A, B;

  struct param_type {
    const s32 A, B;

    param_type(s32 aa, s32 bb): A(aa), B(bb) {}
  };

  explicit uniform_s32_distribution(const s32 a = 0, const s32 b = 0xFFFFFFFF): A(a), B(b) {
    PrimeAssert(B >= A, "Invalid random range.");
  }

  explicit uniform_s32_distribution(const param_type& params): A(params.A), B(params.B) {
    PrimeAssert(B >= A, "Invalid random range.");
  }

  template <class Generator>
  s32 operator()(Generator& g) const {
    return rnd(g, A, B);
  }

  template <class Generator>
  s32 operator()(Generator& g, const param_type& params) const {
    return rnd(g, params.A, params.B);
  }

  s32 a() const {
    return A;
  }

  s32 b() const {
    return B;
  }

  private:
  template <class Generator>
  s32 rnd(Generator& g, const s32 a, const s32 b) const {
    if(a == 0x80000000 && b == 0x7FFFFFFF) {
      return g();
    }
    else {
      const s32 range = b - a + 1;
      return (g() % range) + a;
    }
  }
};

class Random {
private:
  tinymt_t mt;
#ifdef _DEBUG
  bool seeded;
#endif

public:

  static Random instance;

public:

  Random();
  Random(const Random& other);
  ~Random();

public:

  Random& operator=(const Random& other);

  void Seed(s32 seed);
  u32 GetValue();
  u32 GetValueMax() const;
  f32 GetValueF();
  u32 GetRange(u32 low, u32 high);
  s32 GetRange(s32 low, s32 high);
  f32 GetRange(f32 low, f32 high);
  void FillArrayS32(s32* a, u32 size, s32 start, u32 step);
  void FillArrayU32(u32* a, u32 size, s32 start, u32 step);
  u32 GetWeightedChoice(u32* a, u32 size);

};

class RandomGenerator {
public:

  typedef u32 result_type;

public:

  Random& rng;
  
public:

  static constexpr u32 min() {return 0;}
  static constexpr u32 max() {return 0xFFFFFFFF;}
  u32 operator()() {
    return rng.GetValue();
  }

public:

  RandomGenerator(Random& rng): rng(rng) {}

};

};
