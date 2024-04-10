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

#include <Prime/Types/Vec2.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Vec2& Vec2::operator=(const Vec2& other) {
  x = other.x;
  y = other.y;
  return *this;
}

Vec2& Vec2::operator+=(const Vec2& other) {
  x += other.x;
  y += other.y;
  return *this;
}

Vec2& Vec2::operator-=(const Vec2& other) {
  x -= other.x;
  y -= other.y;
  return *this;
}

Vec2& Vec2::operator*=(f32 scaler) {
  return *this = operator*(scaler);
}

Vec2 Vec2::operator+(const Vec2& other) const {
  return Vec2(x + other.x, y + other.y);
}

Vec2 Vec2::operator-(const Vec2& other) const {
  return Vec2(x - other.x, y - other.y);
}

Vec2 Vec2::operator-() const {
  return Vec2(-x, -y);
}

Vec2 Vec2::operator*(f32 s) const {
  return Vec2(x * s, y * s);
}

bool Vec2::operator==(const Vec2& other) const {
  return x == other.x && y == other.y;
}

bool Vec2::operator!=(const Vec2& other) const {
  return x != other.x || y != other.y;
}

f32 Vec2::GetDot(const Vec2& other) const {
  return x * other.x + y * other.y;
}

f32 Vec2::GetDotPerp(const Vec2& other) const {
  return x * other.y - y * other.x;
}

f32 Vec2::GetLengthSquared() const {
  return x * x + y * y;
}

f32 Vec2::GetLength() const {
  return sqrtf(x * x + y * y);
}

Vec2 Vec2::GetUnit() const {
  f32 len = GetLength();
  if(len > 0.0f) {
    const f32 lenInv = 1 / len;
    return Vec2(x * lenInv, y * lenInv);
  }
  else {
    return Vec2();
  }
}

Vec2& Vec2::Normalize() {
  f32 len = GetLength();
  if(len == 0.0f)
    return *this;
  const f32 lenInv = 1 / len;
  x *= lenInv;
  y *= lenInv;
  return *this;
}

Vec2 Vec2::GetLerp(const Vec2& other, f32 t) const {
  return Vec2(
    ::GetLerp(x, other.x, t),
    ::GetLerp(y, other.y, t));
}

Vec2& Vec2::Reflect(const Vec2& normal) {
  *this = *this - normal * (this->GetDot(normal) * 2.0f);
  return *this;
}

Vec2& Vec2::Rotate(f32 angle) {
  f32 angleRad = angle * PrimeDegToRadF;
  f32 ca = cosf(angleRad);
  f32 sa = sinf(angleRad);
  f32 rx = ca * x - sa * y;
  f32 ry = sa * x + ca * y;
  x = rx;
  y = ry;
  return *this;
}
