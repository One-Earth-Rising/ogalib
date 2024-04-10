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

#include <Prime/Types/Mat44.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Types/Quat.h>
#include <math.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

static int glhInvertMatrixf2(float *m, float *out);

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

const Mat44 Mat44::Identity = Mat44(
  1.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 1.0f);

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

bool Mat44::operator==(const Mat44& other) const {
  return memcmp(e, other.e, sizeof(e)) == 0;
}

bool Mat44::operator!=(const Mat44& other) const {
  return memcmp(e, other.e, sizeof(e)) != 0;
}

Mat44 Mat44::operator*(const Mat44& other) const {
  Mat44 result = *this;
  return result.Multiply(other);
}

Vec2 Mat44::operator*(const Vec2& v) const {
  return Multiply(v);
}

Vec3 Mat44::operator*(const Vec3& v) const {
  return Multiply(v);
}

Vec4 Mat44::operator*(const Vec4& v) const {
  return Multiply(v);
}

Mat44& Mat44::LoadRotation(f32 angle, f32 x, f32 y, f32 z) {
  f32 angleRad = angle * PrimeDegToRadF;
  f32 c = cosf(angleRad);
  f32 s = sinf(angleRad);
  f32 omc = 1.0f - c;
  f32 xx = x * x;
  f32 yy = y * y;
  f32 zz = z * z;
  f32 xy = x * y;
  f32 xz = x * z;
  f32 yz = y * z;
  f32 xs = x * s;
  f32 ys = y * s;
  f32 zs = z * s;

  e11 = xx * omc + c;
  e21 = xy * omc + zs;
  e31 = xz * omc - ys;
  e41 = 0.0f;

  e12 = xy * omc - zs;
  e22 = yy * omc + c;
  e32 = yz * omc + xs;
  e42 = 0.0f;

  e13 = xz * omc + ys;
  e23 = yz * omc - xs;
  e33 = zz * omc + c;
  e43 = 0.0f;

  e14 = 0.0f;
  e24 = 0.0f;
  e34 = 0.0f;
  e44 = 1.0f;

  return *this;
}

Mat44& Mat44::LoadRotation(f32 angle, const Vec3& axis) {
  return LoadRotation(angle, axis.x, axis.y, axis.z);
}

Mat44& Mat44::LoadOrtho(f32 x, f32 y, f32 w, f32 h, f32 nearZ, f32 farZ) {
  f32 vl = x;
  f32 vr = x + w;
  f32 vt = y + h;
  f32 vb = y;
  f32 vfmn = farZ - nearZ;

  LoadIdentity();

  e11 = 2.0f / w;
  e22 = 2.0f / h;
  e33 = -2.0f / vfmn;
  e14 = -((vr + vl) / w);
  e24 = -((vt + vb) / h);
  e34 = -((farZ + nearZ) / vfmn);

  return *this;
}

Mat44& Mat44::LoadOrtho2(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
  LoadIdentity();

  f32 w = r - l;
  f32 h = t - b;
  f32 d = f - n;

  e11 = 2.0f / w;
  e22 = 2.0f / h;
  e33 = -2.0f / d;
  e14 = -((r + l) / w);
  e24 = -((t + b) / h);
  e34 = -((f + n) / d);

  return *this;
}

Mat44& Mat44::LoadPerspective(f32 fov, f32 aspect, f32 nearZ, f32 farZ) {
  f32 maxY = (f32) (((f64) nearZ) * tan(fov * PrimePi / 360.0));
  f32 minY = -maxY;
  f32 maxX = maxY * aspect;
  f32 minX = minY * aspect;

  f32 w = maxX - minX;
  f32 h = maxY - minY;

  f32 nearZ2 = 2.0f * nearZ;
  f32 w2 = nearZ2 / w;
  f32 h2 = nearZ2 / h;

  f32 depth = farZ - nearZ;
  f32 q = -(farZ + nearZ) / depth;
  f32 qn = -2.0f * (farZ * nearZ) / depth;

  e11 = w2;
  e21 = 0.0f;
  e31 = 0.0f;
  e41 = 0.0f;

  e12 = 0.0f;
  e22 = h2;
  e32 = 0.0f;
  e42 = 0.0f;

  e13 = 0.0f;
  e23 = 0.0f;
  e33 = q;
  e43 = -1.0f;

  e14 = 0.0f;
  e24 = 0.0f;
  e34 = qn;
  e44 = 0.0f;

  return *this;
}

Mat44& Mat44::LoadLookAt(f32 eyeX, f32 eyeY, f32 eyeZ, f32 atX, f32 atY, f32 atZ, f32 upX, f32 upY, f32 upZ) {
  f32 fX = atX - eyeX;
  f32 fY = atY - eyeY;
  f32 fZ = atZ - eyeZ;

  f32 len = sqrtf(fX * fX + fY * fY + fZ * fZ);
  f32 fnX = fX / len;
  f32 fnY = fY / len;
  f32 fnZ = fZ / len;

  f32 sideX = fnY * upZ - fnZ * upY;
  f32 sideY = fnZ * upX - fnX * upZ;
  f32 sideZ = fnX * upY - fnY * upX;

  f32 u2X, u2Y, u2Z;

  len = sqrtf(sideX * sideX + sideY * sideY + sideZ * sideZ);
  sideX = sideX / len;
  sideY = sideY / len;
  sideZ = sideZ / len;

  u2X = sideY * fnZ - sideZ * fnY;
  u2Y = sideZ * fnX - sideX * fnZ;
  u2Z = sideX * fnY - sideY * fnZ;

  e11 = sideX;
  e12 = sideY;
  e13 = sideZ;
  e14 = 0.0f;

  e21 = u2X;
  e22 = u2Y;
  e23 = u2Z;
  e24 = 0.0f;

  e31 = -fnX;
  e32 = -fnY;
  e33 = -fnZ;
  e34 = 0.0f;

  e41 = 0.0f;
  e42 = 0.0f;
  e43 = 0.0f;
  e44 = 1.0f;

  return Translate(-eyeX, -eyeY, -eyeZ);
}

Mat44& Mat44::LoadLookAt(const Vec3& eye, const Vec3& at, const Vec3& up) {
  return LoadLookAt(eye.x, eye.y, eye.z, at.x, at.y, at.z, up.x, up.y, up.z);
}

Mat44& Mat44::Translate(f32 x, f32 y, f32 z) {
  if(x != 0.0f || y != 0.0f || z != 0.0f)
    return Multiply(Mat44().LoadTranslation(x, y, z));
  else
    return *this;
}

Mat44& Mat44::Translate(const Vec3& pos) {
  if(pos.IsNotZero())
    return Multiply(Mat44().LoadTranslation(pos));
  else
    return *this;
}

Mat44& Mat44::Scale(f32 x, f32 y, f32 z) {
  if(x != 1.0f || y != 1.0f || z != 1.0f)
    return Multiply(Mat44().LoadScaling(x, y, z));
  else
    return *this;
}

Mat44& Mat44::Scale(const Vec3& scale) {
  if(scale.IsNotOne())
    return Multiply(Mat44().LoadScaling(scale));
  else
    return *this;
}

Mat44& Mat44::Rotate(f32 angle, f32 x, f32 y, f32 z) {
  if(angle != 0.0f)
    return Multiply(Mat44().LoadRotation(angle, x, y, z));
  else
    return *this;
}

Mat44& Mat44::Rotate(f32 angle, const Vec3& axis) {
  if(angle != 0.0f)
    return Multiply(Mat44().LoadRotation(angle, axis));
  else
    return *this;
}

Mat44& Mat44::Multiply(const Mat44& by) {
  Mat44 mat;

  if(by.IsIdentity()) {
    return *this;
  }

  if(IsIdentity()) {
    return *this = by;
  }

  mat.e11 = e[ 0] * by.e[ 0] + e[ 4] * by.e[ 1] + e[ 8] * by.e[ 2] + e[12] * by.e[ 3];
  mat.e12 = e[ 0] * by.e[ 4] + e[ 4] * by.e[ 5] + e[ 8] * by.e[ 6] + e[12] * by.e[ 7];
  mat.e13 = e[ 0] * by.e[ 8] + e[ 4] * by.e[ 9] + e[ 8] * by.e[10] + e[12] * by.e[11];
  mat.e14 = e[ 0] * by.e[12] + e[ 4] * by.e[13] + e[ 8] * by.e[14] + e[12] * by.e[15];

  mat.e21 = e[ 1] * by.e[ 0] + e[ 5] * by.e[ 1] + e[ 9] * by.e[ 2] + e[13] * by.e[ 3];
  mat.e22 = e[ 1] * by.e[ 4] + e[ 5] * by.e[ 5] + e[ 9] * by.e[ 6] + e[13] * by.e[ 7];
  mat.e23 = e[ 1] * by.e[ 8] + e[ 5] * by.e[ 9] + e[ 9] * by.e[10] + e[13] * by.e[11];
  mat.e24 = e[ 1] * by.e[12] + e[ 5] * by.e[13] + e[ 9] * by.e[14] + e[13] * by.e[15];

  mat.e31 = e[ 2] * by.e[ 0] + e[ 6] * by.e[ 1] + e[10] * by.e[ 2] + e[14] * by.e[ 3];
  mat.e32 = e[ 2] * by.e[ 4] + e[ 6] * by.e[ 5] + e[10] * by.e[ 6] + e[14] * by.e[ 7];
  mat.e33 = e[ 2] * by.e[ 8] + e[ 6] * by.e[ 9] + e[10] * by.e[10] + e[14] * by.e[11];
  mat.e34 = e[ 2] * by.e[12] + e[ 6] * by.e[13] + e[10] * by.e[14] + e[14] * by.e[15];

  if(e[3] == 0.0f && e[7] == 0.0f && e[11] == 0.0f && e[15] == 1.0f) {
    mat.e41 = by.e[3];
    mat.e42 = by.e[7];
    mat.e43 = by.e[11];
    mat.e44 = by.e[15];
  }
  else {
    mat.e41 = e[ 3] * by.e[ 0] + e[ 7] * by.e[ 1] + e[11] * by.e[ 2] + e[15] * by.e[ 3];
    mat.e42 = e[ 3] * by.e[ 4] + e[ 7] * by.e[ 5] + e[11] * by.e[ 6] + e[15] * by.e[ 7];
    mat.e43 = e[ 3] * by.e[ 8] + e[ 7] * by.e[ 9] + e[11] * by.e[10] + e[15] * by.e[11];
    mat.e44 = e[ 3] * by.e[12] + e[ 7] * by.e[13] + e[11] * by.e[14] + e[15] * by.e[15];
  }

  return *this = mat;
}

Mat44& Mat44::MultiplyPre(const Mat44& by) {
  Mat44 mat;

  if(by.IsIdentity()) {
    return *this;
  }

  if(IsIdentity()) {
    return *this = by;
  }

  mat.e11 = by.e[ 0] * e[ 0] + by.e[ 4] * e[ 1] + by.e[ 8] * e[ 2] + by.e[12] * e[ 3];
  mat.e12 = by.e[ 0] * e[ 4] + by.e[ 4] * e[ 5] + by.e[ 8] * e[ 6] + by.e[12] * e[ 7];
  mat.e13 = by.e[ 0] * e[ 8] + by.e[ 4] * e[ 9] + by.e[ 8] * e[10] + by.e[12] * e[11];
  mat.e14 = by.e[ 0] * e[12] + by.e[ 4] * e[13] + by.e[ 8] * e[14] + by.e[12] * e[15];

  mat.e21 = by.e[ 1] * e[ 0] + by.e[ 5] * e[ 1] + by.e[ 9] * e[ 2] + by.e[13] * e[ 3];
  mat.e22 = by.e[ 1] * e[ 4] + by.e[ 5] * e[ 5] + by.e[ 9] * e[ 6] + by.e[13] * e[ 7];
  mat.e23 = by.e[ 1] * e[ 8] + by.e[ 5] * e[ 9] + by.e[ 9] * e[10] + by.e[13] * e[11];
  mat.e24 = by.e[ 1] * e[12] + by.e[ 5] * e[13] + by.e[ 9] * e[14] + by.e[13] * e[15];

  mat.e31 = by.e[ 2] * e[ 0] + by.e[ 6] * e[ 1] + by.e[10] * e[ 2] + by.e[14] * e[ 3];
  mat.e32 = by.e[ 2] * e[ 4] + by.e[ 6] * e[ 5] + by.e[10] * e[ 6] + by.e[14] * e[ 7];
  mat.e33 = by.e[ 2] * e[ 8] + by.e[ 6] * e[ 9] + by.e[10] * e[10] + by.e[14] * e[11];
  mat.e34 = by.e[ 2] * e[12] + by.e[ 6] * e[13] + by.e[10] * e[14] + by.e[14] * e[15];

  if(by.e[3] == 0.0f && by.e[7] == 0.0f && by.e[11] == 0.0f && by.e[15] == 1.0f) {
    mat.e41 = e[3];
    mat.e42 = e[7];
    mat.e43 = e[11];
    mat.e44 = e[15];
  }
  else {
    mat.e41 = by.e[ 3] * e[ 0] + by.e[ 7] * e[ 1] + by.e[11] * e[ 2] + by.e[15] * e[ 3];
    mat.e42 = by.e[ 3] * e[ 4] + by.e[ 7] * e[ 5] + by.e[11] * e[ 6] + by.e[15] * e[ 7];
    mat.e43 = by.e[ 3] * e[ 8] + by.e[ 7] * e[ 9] + by.e[11] * e[10] + by.e[15] * e[11];
    mat.e44 = by.e[ 3] * e[12] + by.e[ 7] * e[13] + by.e[11] * e[14] + by.e[15] * e[15];
  }

  return *this = mat;
}

Mat44& Mat44::MultiplyOrtho(f32 x, f32 y, f32 w, f32 h, f32 nearZ, f32 farZ) {
  return Multiply(Mat44().LoadOrtho(x, y, w, h, nearZ, farZ));
}

Mat44& Mat44::MultiplyPerspective(f32 fov, f32 aspect, f32 nearZ, f32 farZ) {
  return Multiply(Mat44().LoadPerspective(fov, aspect, nearZ, farZ));
}

Mat44& Mat44::MultiplyLookAt(f32 eyeX, f32 eyeY, f32 eyeZ, f32 atX, f32 atY, f32 atZ, f32 upX, f32 upY, f32 upZ) {
  return Multiply(Mat44().LoadLookAt(eyeX, eyeY, eyeZ, atX, atY, atZ, upX, upY, upZ));
}

Mat44& Mat44::MultiplyLookAt(const Vec3& eye, const Vec3& at, const Vec3& up) {
  return Multiply(Mat44().LoadLookAt(eye, at, up));
}

Vec2 Mat44::Multiply(const Vec2& v, f32 z, f32 w) const {
  return Vec2(
    e11 * v.x + e12 * v.y + e13 * z + e14 * w,
    e21 * v.x + e22 * v.y + e23 * z + e24 * w);
}

Vec3 Mat44::Multiply(const Vec3& v, f32 w) const {
  return Vec3(
    e11 * v.x + e12 * v.y + e13 * v.z + e14 * w,
    e21 * v.x + e22 * v.y + e23 * v.z + e24 * w,
    e31 * v.x + e32 * v.y + e33 * v.z + e34 * w);
}

Vec4 Mat44::Multiply(const Vec4& v) const {
  return Vec4(
    e11 * v.x + e12 * v.y + e13 * v.z + e14 * v.w,
    e21 * v.x + e22 * v.y + e23 * v.z + e24 * v.w,
    e31 * v.x + e32 * v.y + e33 * v.z + e34 * v.w,
    e41 * v.x + e42 * v.y + e43 * v.z + e44 * v.w);
}

void Mat44::Multiply(s32 x, s32 y, s32& rx, s32& ry, s32& rz) const {
  s32 ox = (s32) (e11 * x + e12 * y + e14);
  s32 oy = (s32) (e21 * x + e22 * y + e24);
  s32 oz = (s32) (e31 * x + e32 * y + e34);
  rx = ox;
  ry = oy;
  rz = oz;
}

void Mat44::Multiply(s32 x, s32 y, s32& rx, s32& ry) const {
  s32 ox = (s32) (e11 * x + e12 * y + e14);
  s32 oy = (s32) (e21 * x + e22 * y + e24);
  rx = ox;
  ry = oy;
}

void Mat44::Multiply(f32 x, f32 y, f32& rx, f32& ry, f32& rz) const {
  f32 ox = e11 * x + e12 * y + e14;
  f32 oy = e21 * x + e22 * y + e24;
  f32 oz = e31 * x + e32 * y + e34;
  rx = ox;
  ry = oy;
  rz = oz;
}

void Mat44::Multiply(f32 x, f32 y, f32& rx, f32& ry) const {
  f32 ox = e11 * x + e12 * y + e14;
  f32 oy = e21 * x + e22 * y + e24;
  rx = ox;
  ry = oy;
}

bool Mat44::Invert() {
  Mat44 temp;
  
  if(!glhInvertMatrixf2(e, temp.e))
    return false;

  *this = temp;
  return true;
}

Mat44& Mat44::Transpose() {
  Mat44 temp;

  temp.e11 = e11;
  temp.e12 = e21;
  temp.e13 = e31;
  temp.e14 = e41;

  temp.e21 = e12;
  temp.e22 = e22;
  temp.e23 = e32;
  temp.e24 = e42;

  temp.e31 = e13;
  temp.e33 = e33;
  temp.e32 = e23;
  temp.e34 = e43;

  temp.e41 = e14;
  temp.e44 = e44;
  temp.e42 = e24;
  temp.e43 = e34;

  *this = temp;
  return *this;
}

Vec3 Mat44::Reflect(const Vec3& incident, const Vec3& normal) const {
  return incident - normal * (2.0f * incident.GetDot(normal));
}

Quat Mat44::GetQuat() const {
  Quat result;

  f32 tr = e11 + e22 + e33;
  if(tr > 0.0f) {
    f32 s = sqrtf(tr + 1.0f) * 2.0f;
    result.w = 0.25f * s;
    result.x = (e32 - e23) / s;
    result.y = (e13 - e31) / s;
    result.z = (e21 - e12) / s;
  }
  else if((e11 > e22) && (e11 > e33)) {
    f32 s = sqrtf(1.0f + e11 - e22 - e33) * 2.0f;
    result.w = (e32 - e23) / s;
    result.x = 0.25f * s;
    result.y = (e12 + e21) / s;
    result.z = (e13 + e31) / s;
  }
  else if(e22 > e33) {
    f32 s = sqrtf(1.0f + e22 - e11 - e33) * 2.0f;
    result.w = (e13 - e31) / s;
    result.x = (e12 + e21) / s;
    result.y = 0.25f * s;
    result.z = (e23 + e32) / s;
  }
  else {
    f32 s = sqrtf(1.0f + e33 - e11 - e22) * 2.0f;
    result.w = (e21 - e12) / s;
    result.x = (e13 + e31) / s;
    result.y = (e23 + e32) / s;
    result.z = 0.25f * s;
  }

  return result;
}

#define SWAP_ROWS_DOUBLE(a, b) { double *_tmp = a; (a)=(b); (b)=_tmp; }
#define SWAP_ROWS_FLOAT(a, b) { float *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

//This code comes directly from GLU except that it is for float
int glhInvertMatrixf2(float *m, float *out) {
  float wtmp[4][8];
  float m0, m1, m2, m3, s;
  float *r0, *r1, *r2, *r3;
  r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];
  r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
  r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
  r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,
  r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
  r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
  r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,
  r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
  r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
  r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,
  r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
  r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
  r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;
  /* choose pivot - or die */
  if (fabsf(r3[0]) > fabsf(r2[0]))
    SWAP_ROWS_FLOAT(r3, r2);
  if (fabsf(r2[0]) > fabsf(r1[0]))
    SWAP_ROWS_FLOAT(r2, r1);
  if (fabsf(r1[0]) > fabsf(r0[0]))
    SWAP_ROWS_FLOAT(r1, r0);
  if (0.0 == r0[0])
    return 0;
  /* eliminate first variable     */
  m1 = r1[0] / r0[0];
  m2 = r2[0] / r0[0];
  m3 = r3[0] / r0[0];
  s = r0[1];
  r1[1] -= m1 * s;
  r2[1] -= m2 * s;
  r3[1] -= m3 * s;
  s = r0[2];
  r1[2] -= m1 * s;
  r2[2] -= m2 * s;
  r3[2] -= m3 * s;
  s = r0[3];
  r1[3] -= m1 * s;
  r2[3] -= m2 * s;
  r3[3] -= m3 * s;
  s = r0[4];
  if (s != 0.0) {
    r1[4] -= m1 * s;
    r2[4] -= m2 * s;
    r3[4] -= m3 * s;
  }
  s = r0[5];
  if (s != 0.0) {
    r1[5] -= m1 * s;
    r2[5] -= m2 * s;
    r3[5] -= m3 * s;
  }
  s = r0[6];
  if (s != 0.0) {
    r1[6] -= m1 * s;
    r2[6] -= m2 * s;
    r3[6] -= m3 * s;
  }
  s = r0[7];
  if (s != 0.0) {
    r1[7] -= m1 * s;
    r2[7] -= m2 * s;
    r3[7] -= m3 * s;
  }
  /* choose pivot - or die */
  if (fabsf(r3[1]) > fabsf(r2[1]))
    SWAP_ROWS_FLOAT(r3, r2);
  if (fabsf(r2[1]) > fabsf(r1[1]))
    SWAP_ROWS_FLOAT(r2, r1);
  if (0.0 == r1[1])
    return 0;
  /* eliminate second variable */
  m2 = r2[1] / r1[1];
  m3 = r3[1] / r1[1];
  r2[2] -= m2 * r1[2];
  r3[2] -= m3 * r1[2];
  r2[3] -= m2 * r1[3];
  r3[3] -= m3 * r1[3];
  s = r1[4];
  if (0.0 != s) {
    r2[4] -= m2 * s;
    r3[4] -= m3 * s;
  }
  s = r1[5];
  if (0.0 != s) {
    r2[5] -= m2 * s;
    r3[5] -= m3 * s;
  }
  s = r1[6];
  if (0.0 != s) {
    r2[6] -= m2 * s;
    r3[6] -= m3 * s;
  }
  s = r1[7];
  if (0.0 != s) {
    r2[7] -= m2 * s;
    r3[7] -= m3 * s;
  }
  /* choose pivot - or die */
  if (fabsf(r3[2]) > fabsf(r2[2]))
    SWAP_ROWS_FLOAT(r3, r2);
  if (0.0 == r2[2])
    return 0;
  /* eliminate third variable */
  m3 = r3[2] / r2[2];
  r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
    r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];
  /* last check */
  if (0.0 == r3[3])
    return 0;
  s = 1.0f / r3[3];    /* now back substitute row 3 */
  r3[4] *= s;
  r3[5] *= s;
  r3[6] *= s;
  r3[7] *= s;
  m2 = r2[3];     /* now back substitute row 2 */
  s = 1.0f / r2[2];
  r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
    r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
  m1 = r1[3];
  r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
    r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
  m0 = r0[3];
  r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
    r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;
  m1 = r1[2];     /* now back substitute row 1 */
  s = 1.0f / r1[1];
  r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
    r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
  m0 = r0[2];
  r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
    r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;
  m0 = r0[1];     /* now back substitute row 0 */
  s = 1.0f / r0[0];
  r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
    r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);
  MAT(out, 0, 0) = r0[4];
  MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
  MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
  MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
  MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
  MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
  MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
  MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
  MAT(out, 3, 3) = r3[7];
  return 1;
}
