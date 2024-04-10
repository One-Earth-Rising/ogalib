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

#include <Prime/Types/Quat.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Quat::operator Mat44() const {
  Mat44 result;

  const f32 xx = x * x;
  const f32 yy = y * y;
  const f32 zz = z * z;
  const f32 xw = x * w;
  const f32 yw = y * w;
  const f32 zw = z * w;
  const f32 xy = x * y;
  const f32 xz = x * z;
  const f32 yz = y * z;

  result.e11 = 1.0f - 2.0f * (yy + zz);
  result.e12 = 2.0f * (xy - zw);
  result.e13 = 2.0f * (xz + yw);
  result.e14 = 0.0f;
    
  result.e21 = 2.0f * (xy + zw);
  result.e22 = 1.0f - 2.0f * (xx + zz);
  result.e23 = 2.0f * (yz - zw);
  result.e24 = 0.0f;
    
  result.e31 = 2.0f * (xz - yw);
  result.e32 = 2.0f * (yz + xw);
  result.e33 = 1.0f - 2.0f * (xx + yy);
  result.e34 = 0.0f;
    
  result.e41 = 0.0f;
  result.e42 = 0.0f;
  result.e43 = 0.0f;
  result.e44 = 1.0f;

  return result;
}

Quat& Quat::operator=(const Quat& other) {
  x = other.x;
  y = other.y;
  z = other.z;
  w = other.w;

  return *this;
}

Quat& Quat::operator=(const Mat44& mat) {
  Quat q(
    sqrtf(max(0.0f, 1.0f + mat.e11 - mat.e22 - mat.e33)) * 0.5f,
    sqrtf(max(0.0f, 1.0f - mat.e11 + mat.e22 - mat.e33)) * 0.5f,
    sqrtf(max(0.0f, 1.0f - mat.e11 - mat.e22 + mat.e33)) * 0.5f,
    sqrtf(max(0.0f, 1.0f + mat.e11 + mat.e22 + mat.e33)) * 0.5f);
  const f32 sx = q.x * (mat.e32 - mat.e23);
  const f32 sy = q.y * (mat.e13 - mat.e31);
  const f32 sz = q.z * (mat.e21 - mat.e12);
  if(sx < 0.0f)
    q.x = -q.x;
  if(sy < 0.0f)
    q.y = -q.y;
  if(sz < 0.0f)
    q.z = -q.z;
  return operator=(q);
}

Quat Quat::operator*(const Quat& other) {
  Vec3 v1(x, y, z);
  Vec3 v2(other.x, other.y, other.z);
  f32 angle = w * other.w - v1.GetDot(v2);
  Vec3 vc = v1 * v2;
  v1 *= other.w;
  v2 *= w;
  Vec3 vs = v1 + v2 + vc;
  return Quat(vs.x, vs.y, vs.z, angle);
}

bool Quat::operator==(const Quat& other) const {
  return x == other.x
    && y == other.y
    && z == other.z
    && w == other.w;
}

Quat& Quat::Normalize() {
  f32 lengthSquared = x * x + y * y + z * z + w * w;
  if(lengthSquared == 0.0f) {
    return *this;
  }
  else {
    f32 length = sqrtf(lengthSquared);
    f32 invertedLength = 1.0f / length;
    x *= invertedLength;
    y *= invertedLength;
    z *= invertedLength;
    w *= invertedLength;
  }
  return *this;
}

Quat& Quat::Invert() {
  const f32 length = 1.0f / (x * x + y * y + z * z + w * w);
  const f32 negativeLength = -length;
  x *= negativeLength;
  y *= negativeLength;
  z *= negativeLength;
  w *= length;
  return *this;
}

Quat& Quat::ConvertFromMat44(const Mat44& mat) {
  x = sqrtf(max(0.0f, 1.0f + mat.e11 - mat.e22 - mat.e33)) * 0.5f;
  y = sqrtf(max(0.0f, 1.0f - mat.e11 + mat.e22 - mat.e33)) * 0.5f;
  z = sqrtf(max(0.0f, 1.0f - mat.e11 - mat.e22 + mat.e33)) * 0.5f;
  w = sqrtf(max(0.0f, 1.0f + mat.e11 + mat.e22 + mat.e33)) * 0.5f;
  f32 sx = x * (mat.e32 - mat.e23);
  f32 sy = y * (mat.e13 - mat.e31);
  f32 sz = z * (mat.e21 - mat.e21);
  if(sx < 0.0f)
    x = -x;
  if(sy < 0.0f)
    y = -y;
  if(sz < 0.0f)
    z = -z;
  return *this;
}

Quat& Quat::ConvertFromEulerAngles(const Vec3& euler) {
  f64 rol = euler.x * 0.5;
  f64 ptc = euler.y * 0.5;
  f64 yaw = euler.z * 0.5;
  f64 yawSin = sin(yaw);
  f64 ptcSin = sin(ptc);
  f64 rolSin = sin(rol);
  f64 yawCos = cos(yaw);
  f64 ptcCos = cos(ptc);
  f64 rolCos = cos(rol);

  x = (f32) (rolSin * ptcCos * yawCos + rolCos * ptcSin * yawSin);
  y = (f32) (rolCos * ptcSin * yawCos - rolSin * ptcCos * yawSin);
  z = (f32) (rolCos * ptcCos * yawSin - rolSin * ptcSin * yawCos);
  w = (f32) (rolCos * ptcCos * yawCos + rolSin * ptcSin * yawSin);

  return *this;
}

Quat& Quat::ConvertFromEulerAnglesDeg(const Vec3& euler) {
  return ConvertFromEulerAngles(euler * PrimeDegToRadF);
}

Quat Quat::Interpolate(const Quat& other, f32 t) const {
  f64 diff = x * other.x + y * other.y + z * other.z + w * other.w;
  f64 absDiff = abs(diff);

  f64 useX;
  f64 useY;
  f64 useZ;
  f64 useW;

  if(diff < 0.0f) {
    useX = -x;
    useY = -y;
    useZ = -z;
    useW = -w;
  }
  else {
    useX = x;
    useY = y;
    useZ = z;
    useW = w;
  }

  f64 w0;
  f64 w1;

  if(1.0 - absDiff > 0.001) {
    f64 angle = acos(absDiff);
    f64 oosa = 1.0 / sin(angle);
    w0 = sin(angle * (1.0 - t)) * oosa;
    w1 = sin(angle * t) * oosa;
  }
  else {
    w0 = 1.0 - t;
    w1 = t;
  }

  Quat result(
    (f32) ((useX * w0) + (other.x * w1)),
    (f32) ((useY * w0) + (other.y * w1)),
    (f32) ((useZ * w0) + (other.z * w1)),
    (f32) ((useW * w0) + (other.w * w1)));
  result.Normalize();
  return result;
}

Vec3 Quat::GetEulerAngles() const {
  const f32 xx = x * x;
  const f32 yy = y * y;
  const f32 zz = z * z;
  const f32 ww = w * w;
  const f32 xw = x * w;
  const f32 yw = y * w;
  const f32 zw = z * w;
  const f32 xy = x * y;
  const f32 xz = x * z;
  const f32 yz = y * z;
    
  const f32 unit = xx + yy + zz + ww;
  const f32 test = yz + xw;

  Vec3 result;
    
  if(test > 0.499999f * unit) {
    result.z = 2.0f * atan2f(y, w);
    result.x = (f32) (PrimePi * 0.5);
    result.y = 0.0;
  }
  else if(test < -0.499999f * unit) {
    result.z = -2.0f * atan2f(y, w);
    result.x = -(f32) (PrimePi * 0.5);
    result.y = 0.0;
  }
  else {
    result.z = atan2f(2.0f * (zw - xy), yy - zz - xx + ww);
    result.x = asinf(2.0f * test / unit);
    result.y = atan2f(2.0f * (yw - xz), zz - yy - xx + ww);
  };
    
  return result;
}

Vec3 Quat::GetEulerAnglesDeg() const {
  Vec3 result = GetEulerAngles();
  result.x *= PrimeRadToDegF;
  result.y *= PrimeRadToDegF;
  result.z *= PrimeRadToDegF;
  return result;
}

Mat44 Quat::GetRotationMat44() const {
  Mat44 result;

  f32 xx = x * x;
  f32 yy = y * y;
  f32 zz = z * z;
  f32 xw = x * w;
  f32 yw = y * w;
  f32 zw = z * w;
  f32 xy = x * y;
  f32 xz = x * z;
  f32 yz = y * z;

  result.e11 = 1.0f - 2.0f * (yy + zz);
  result.e12 = 2.0f * (xy - zw);
  result.e13 = 2.0f * (xz + yw);
  result.e14 = 0.0f;

  result.e21 = 2.0f * (xy + zw);
  result.e22 = 1.0f - 2.0f * (xx + zz);
  result.e23 = 2.0f * (yz - xw);
  result.e24 = 0.0f;

  result.e31 = 2.0f * (xz - yw);
  result.e32 = 2.0f * (yz + xw);
  result.e33 = 1.0f - 2.0f * (xx + yy);
  result.e34 = 0.0f;

  result.e41 = 0.0f;
  result.e42 = 0.0f;
  result.e43 = 0.0f;
  result.e44 = 1.0f;

  return result;
}
