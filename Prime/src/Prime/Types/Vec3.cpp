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

#include <Prime/Types/Vec3.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Vec3& Vec3::operator=(const Vec3& other) {
  x = other.x;
  y = other.y;
  z = other.z;
  return *this;
}

Vec3& Vec3::operator=(const Vec2& other) {
  x = other.x;
  y = other.y;
  z = 0.0f;
  return *this;
}

Vec3& Vec3::operator+=(const Vec3& other) {
  x += other.x;
  y += other.y;
  z += other.z;
  return *this;
}

Vec3& Vec3::operator-=(const Vec3& other) {
  x -= other.x;
  y -= other.y;
  z -= other.z;
  return *this;
}

Vec3& Vec3::operator*=(f32 scale) {
  return *this = operator*(scale);
}

Vec3& Vec3::operator*=(const Vec3& other) {
  return *this = operator*(other);
}

Vec3 Vec3::operator+(const Vec3& other) const {
  return Vec3(x + other.x, y + other.y, z + other.z);
}

Vec3 Vec3::operator-(const Vec3& other) const {
  return Vec3(x - other.x, y - other.y, z - other.z);
}

Vec3 Vec3::operator-() const {
  return Vec3(-x, -y, -z);
}

Vec3 Vec3::operator*(f32 scaler) const {
  return Vec3(x * scaler, y * scaler, z * scaler);
}

Vec3 Vec3::operator*(const Vec3& other) const {
  return Vec3(
    y * other.z - z * other.y,
    z * other.x - x * other.z,
    x * other.y - y * other.x);
}

bool Vec3::operator==(const Vec3& other) const {
  return x == other.x && y == other.y && z == other.z;
}

bool Vec3::operator!=(const Vec3& other) const {
  return x != other.x || y != other.y || z != other.z;
}

bool Vec3::operator<(const Vec3& other) const {
  if(x < other.x)
    return true;
  else if(x > other.x)
    return false;

  if(y < other.y)
    return true;
  else if(y > other.y)
    return false;

  return z < other.z;
}

bool Vec3::operator>(const Vec3& other) const {
  if(x > other.x)
    return true;
  else if(x < other.x)
    return false;

  if(y > other.y)
    return true;
  else if(y < other.y)
    return false;

  return z > other.z;
}

f32 Vec3::GetDot(const Vec3& other) const {
  return x * other.x + y * other.y + z * other.z;
}

f32 Vec3::GetLengthSquared() const {
  return x * x + y * y + z * z;
}

f32 Vec3::GetLength() const {
  return sqrtf(x * x + y * y + z * z);
}

Vec3 Vec3::GetUnit() const {
  f32 len = GetLength();
  if(len > 0.0f) {
    const f32 lenInv = 1 / len;
    return Vec3(x * lenInv, y * lenInv, z * lenInv);
  }
  else {
    return Vec3();
  }
}

Vec3 Vec3::GetLerp(const Vec3& other, f32 t) const {
  return Vec3(
    ::GetLerp(x, other.x, t),
    ::GetLerp(y, other.y, t),
    ::GetLerp(z, other.z, t));
}

Vec3& Vec3::Normalize() {
  f32 len = GetLength();
  if(len == 0.0f)
    return *this;
  const f32 lenInv = 1 / len;
  x *= lenInv;
  y *= lenInv;
  z *= lenInv;
  return *this;
}

Vec3& Vec3::Reflect(const Vec3& normal) {
  f32 dot = normal.GetDot(*this);
  *this = *this - normal * (dot * 2.0f);
  return *this;
}
