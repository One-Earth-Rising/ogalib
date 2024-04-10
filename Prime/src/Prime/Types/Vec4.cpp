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

#include <Prime/Types/Vec4.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Vec4& Vec4::operator=(const Vec4& other) {
  x = other.x;
  y = other.y;
  z = other.z;
  w = other.w;
  return *this;
}

Vec4& Vec4::operator=(const Vec3& other) {
  x = other.x;
  y = other.y;
  z = other.z;
  w = 0.0f;
  return *this;
}

Vec4& Vec4::operator=(const Vec2& other) {
  x = other.x;
  y = other.y;
  z = 0.0f;
  w = 0.0f;
  return *this;
}

Vec4& Vec4::operator+=(const Vec4& other) {
  x += other.x;
  y += other.y;
  z += other.z;
  w += other.w;
  return *this;
}

Vec4& Vec4::operator-=(const Vec4& other) {
  x -= other.x;
  y -= other.y;
  z -= other.z;
  w -= other.w;
  return *this;
}

Vec4& Vec4::operator*=(f32 scale) {
  return *this = operator*(scale);
}

Vec4& Vec4::operator*=(const Vec4& other) {
  return *this = operator*(other);
}

Vec4 Vec4::operator+(const Vec4& other) const {
  return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
}

Vec4 Vec4::operator-(const Vec4& other) const {
  return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
}

Vec4 Vec4::operator-() const {
  return Vec4(-x, -y, -z, -w);
}

Vec4 Vec4::operator*(f32 scaler) const {
  return Vec4(x * scaler, y * scaler, z * scaler, w * scaler);
}

Vec4 Vec4::operator*(const Vec4& other) const {
  Vec3 v1 = Vec3(x, y, z);
  Vec3 v2 = Vec3(other.x, other.y, other.z);
  f32 angle = w * other.w - v1.GetDot(v2);
  Vec3 vc = v1 * v2;
  v1 *= other.w;
  v2 *= w;
  Vec3 vs = v1 + v2 + vc;
  return Vec4(vs.x, vs.y, vs.z, angle);
}

bool Vec4::operator==(const Vec4& other) const {
  return x == other.x && y == other.y && z == other.z;
}

bool Vec4::operator!=(const Vec4& other) const {
  return x != other.x || y != other.y || z != other.z;
}

f32 Vec4::GetDot(const Vec4& other) const {
  return x * other.x + y * other.y + z * other.z + w * other.w;
}

f32 Vec4::GetLengthSquared() const {
  return x * x + y * y + z * z;
}

f32 Vec4::GetLength() const {
  return sqrtf(x * x + y * y + z * z);
}

Vec4 Vec4::GetUnit() const {
  f32 len = GetLength();
  if(len > 0.0f) {
    const f32 lenInv = 1 / len;
    return Vec4(x * lenInv, y * lenInv, z * lenInv, w * lenInv);
  }
  else {
    return Vec4();
  }
}

Vec4& Vec4::Normalize() {
  f32 len = GetLength();
  if(len == 0.0f)
    return *this;
  const f32 lenInv = 1 / len;
  x *= lenInv;
  y *= lenInv;
  z *= lenInv;
  w *= lenInv;
  return *this;
}
