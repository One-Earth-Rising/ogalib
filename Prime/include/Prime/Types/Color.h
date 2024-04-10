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

#include <Prime/Types/Vec4.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Color {
public:

  f32 r;    //!< The red hue.
  f32 g;    //!< The green hue.
  f32 b;    //!< The blue hue.
  f32 a;    //!< The alpha value.

public:

  explicit Color() {}
  explicit Color(f32 r, f32 g, f32 b, f32 a = 1.0f): r(r), g(g), b(b), a(a) {}
  Color(const Color& other);
  ~Color() {}

public:

  operator Vec4() const {return Vec4(r, g, b, a);}

  Color& operator=(s32 value);

  Color& operator=(const Color& other);
  Color operator*(const Color& other) const;
  Color operator*(f32 scaler) const;
  Color& operator*=(const Color& other);
  Color operator+(const Color& other) const;
  Color& operator+=(const Color& other);

  bool operator==(const Color& other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
  }

  bool operator!=(const Color& other) const {
    return r != other.r || g != other.g || b != other.b || a != other.a;
  }

  bool operator<(const Color& other) const {
    return r + g + b + a < other.r + other.g + other.b + other.a;
  }

  bool IsWhite() const {return a == 1.0f && r == 1.0f && g == 1.0f && b == 1.0f;}

  void Zero();

  void SetBlack();
  void SetWhite();
  void SetRed();
  void SetGreen();
  void SetBlue();
  void SetYellow();
  void SetOrange();
  void SetCyan();

};

};
