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

class Vec2 {
public:

  f32 x;
  f32 y;

public:

  explicit Vec2() {}
  explicit Vec2(f32 x, f32 y): x(x), y(y) {}
  Vec2(const Vec2& other) {(void) operator=(other);}
  ~Vec2() {}

public:

  Vec2& operator=(const Vec2& other);
  Vec2& operator+=(const Vec2& other);
  Vec2& operator-=(const Vec2& other);
  Vec2& operator*=(f32 scaler);

  Vec2 operator+(const Vec2& other) const;
  Vec2 operator-(const Vec2& other) const;
  Vec2 operator-() const;
  Vec2 operator*(f32 scaler) const;

  bool operator==(const Vec2& other) const;
  bool operator!=(const Vec2& other) const;

  bool IsZero() const {return x == 0.0f && y == 0.0f;}
  bool IsNotZero() const {return x != 0.0f || y != 0.0f;}

  bool IsOne() const {return x == 1.0f && y == 1.0f;}
  bool IsNotOne() const {return x != 1.0f || y != 1.0f;}

  bool IsNegativeOne() const {return x == -1.0f && y == -1.0f;}
  bool IsNotNegativeOne() const {return x != -1.0f || y != -1.0f;}

  f32 GetDot(const Vec2& other) const;
  f32 GetDotPerp(const Vec2& other) const;

  f32 GetLengthSquared() const;
  f32 GetLength() const;

  Vec2 GetUnit() const;

  Vec2 GetLerp(const Vec2& other, f32 t) const;
  
  Vec2& Normalize();
  Vec2& Reflect(const Vec2& normal);
  Vec2& Rotate(f32 angle);

};

};
