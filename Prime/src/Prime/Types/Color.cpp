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

#include <Prime/Types/Color.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

Color::Color(const Color& other) {
  operator=(other);
}

Color& Color::operator=(s32 value) {
  if(value == 0) {
    r = 0.0f;
    g = 0.0f;
    b = 0.0f;
    a = 1.0f;
  }
  return *this;
}

Color& Color::operator=(const Color& other) {
  r = other.r;
  g = other.g;
  b = other.b;
  a = other.a;
  return *this;
}

Color Color::operator*(const Color& other) const {
  return Color(r * other.r, g * other.g, b * other.b, a * other.a);
}

Color Color::operator*(f32 scaler) const {
  return Color(r * scaler, g * scaler, b * scaler, a * scaler);
}

Color& Color::operator*=(const Color& other) {
  return *this = operator*(other);
}

Color Color::operator+(const Color& other) const {
  return Color(r + other.r, g + other.g, b + other.b, a + other.a);
}

Color& Color::operator+=(const Color& other) {
  return *this = operator+(other);
}

void Color::Zero() {
  r = 0.0f;
  g = 0.0f;
  b = 0.0f;
  a = 0.0f;
}

void Color::SetBlack() {
  r = 0.0f;
  g = 0.0f;
  b = 0.0f;
  a = 1.0f;
}

void Color::SetWhite() {
  r = 1.0f;
  g = 1.0f;
  b = 1.0f;
  a = 1.0f;
}

void Color::SetRed() {
  r = 1.0f;
  g = 0.0f;
  b = 0.0f;
  a = 1.0f;
}

void Color::SetGreen() {
  r = 0.0f;
  g = 1.0f;
  b = 0.0f;
  a = 1.0f;
}

void Color::SetBlue() {
  r = 0.0f;
  g = 0.0f;
  b = 1.0f;
  a = 1.0f;
}

void Color::SetYellow() {
  r = 1.0f;
  g = 1.0f;
  b = 0.0f;
  a = 1.0f;
}

void Color::SetOrange() {
  r = 1.0f;
  g = 0.5f;
  b = 0.0f;
  a = 1.0f;
}

void Color::SetCyan() {
  r = 0.0f;
  g = 1.0f;
  b = 1.0f;
  a = 1.0f;
}
