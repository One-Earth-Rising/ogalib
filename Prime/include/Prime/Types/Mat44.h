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

#include <Prime/Types/Vec3.h>
#include <Prime/Types/Vec4.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Quat;

class Mat44 {
public:

  union {
    f32 e[16];
    struct {
      // eRC -- value row, col -- e32 = row 3, col 2
      f32 e11, e21, e31, e41;
      f32 e12, e22, e32, e42;
      f32 e13, e23, e33, e43;
      f32 e14, e24, e34, e44;
    };
  };

public:

  explicit Mat44() {}

  explicit Mat44(
    f32 e11, f32 e12, f32 e13, f32 e14,
    f32 e21, f32 e22, f32 e23, f32 e24,
    f32 e31, f32 e32, f32 e33, f32 e34,
    f32 e41, f32 e42, f32 e43, f32 e44):
    e11(e11), e12(e12), e13(e13), e14(e14),
    e21(e21), e22(e22), e23(e23), e24(e24),
    e31(e31), e32(e32), e33(e33), e34(e34),
    e41(e41), e42(e42), e43(e43), e44(e44) {

  }

  explicit Mat44(const f32* p) {
    memcpy(e, p, sizeof(e));
  }

  Mat44(const Mat44& other) {
    memcpy(e, other.e, sizeof(e));
  }
  
  ~Mat44() {}

public:

  bool operator==(const Mat44& other) const;
  bool operator!=(const Mat44& other) const;

  Mat44& operator=(const Mat44& other) {
    memcpy(e, other.e, sizeof(e));
    return *this;
  }

  Mat44 operator*(const Mat44& other) const;
  Vec2 operator*(const Vec2& v) const;
  Vec3 operator*(const Vec3& v) const;
  Vec4 operator*(const Vec4& v) const;
  
  operator f32*() {return e;}
  operator const f32*() const {return e;}

  bool IsIdentity() const {
    return e11 == 1.0f &&
      e22 == 1.0f &&
      e33 == 1.0f &&
      e12 == 0.0f &&
      e13 == 0.0f &&
      e14 == 0.0f &&
      e21 == 0.0f &&
      e23 == 0.0f &&
      e24 == 0.0f &&
      e31 == 0.0f &&
      e32 == 0.0f &&
      e34 == 0.0f &&
      e41 == 0.0f &&
      e42 == 0.0f &&
      e43 == 0.0f &&
      e44 == 1.0f;
  }

  Mat44& LoadIdentity() {
    e11 = 1.0f;
    e21 = 0.0f;
    e31 = 0.0f;
    e41 = 0.0f;
    e12 = 0.0f;
    e22 = 1.0f;
    e32 = 0.0f;
    e42 = 0.0f;
    e13 = 0.0f;
    e23 = 0.0f;
    e33 = 1.0f;
    e43 = 0.0f;
    e14 = 0.0f;
    e24 = 0.0f;
    e34 = 0.0f;
    e44 = 1.0f;
    return *this;
  }

  Mat44& Load(const Mat44& other) {
    memcpy(e, other.e, sizeof(e));
    return *this;
  }

  Mat44& LoadTranslation(f32 x, f32 y, f32 z = 0.0f) {
    e11 = 1.0f;
    e21 = 0.0f;
    e31 = 0.0f;
    e41 = 0.0f;
    e12 = 0.0f;
    e22 = 1.0f;
    e32 = 0.0f;
    e42 = 0.0f;
    e13 = 0.0f;
    e23 = 0.0f;
    e33 = 1.0f;
    e43 = 0.0f;
    e14 = x;
    e24 = y;
    e34 = z;
    e44 = 1.0f;
    return *this;
  }

  Mat44& LoadTranslation(const Vec3& pos) {
    e11 = 1.0f;
    e21 = 0.0f;
    e31 = 0.0f;
    e41 = 0.0f;
    e12 = 0.0f;
    e22 = 1.0f;
    e32 = 0.0f;
    e42 = 0.0f;
    e13 = 0.0f;
    e23 = 0.0f;
    e33 = 1.0f;
    e43 = 0.0f;
    e14 = pos.x;
    e24 = pos.y;
    e34 = pos.z;
    e44 = 1.0f;
    return *this;
  }

  Mat44& LoadScaling(f32 x, f32 y, f32 z = 1.0f) {
    e11 = x;
    e21 = 0.0f;
    e31 = 0.0f;
    e41 = 0.0f;
    e12 = 0.0f;
    e22 = y;
    e32 = 0.0f;
    e42 = 0.0f;
    e13 = 0.0f;
    e23 = 0.0f;
    e33 = z;
    e43 = 0.0f;
    e14 = 0.0f;
    e24 = 0.0f;
    e34 = 0.0f;
    e44 = 1.0f;
    return *this;
  }

  Mat44& LoadScaling(const Vec3& scale) {
    e11 = scale.x;
    e21 = 0.0f;
    e31 = 0.0f;
    e41 = 0.0f;
    e12 = 0.0f;
    e22 = scale.y;
    e32 = 0.0f;
    e42 = 0.0f;
    e13 = 0.0f;
    e23 = 0.0f;
    e33 = scale.z;
    e43 = 0.0f;
    e14 = 0.0f;
    e24 = 0.0f;
    e34 = 0.0f;
    e44 = 1.0f;
    return *this;
  }

  Mat44& LoadRotation(f32 angle, f32 x = 0.0f, f32 y = 0.0f, f32 z = 1.0f);
  Mat44& LoadRotation(f32 angle, const Vec3& axis);

  Mat44& LoadOrtho(f32 x, f32 y, f32 w, f32 h, f32 nearZ, f32 farZ);
  Mat44& LoadOrtho2(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
  Mat44& LoadPerspective(f32 fov, f32 aspect, f32 nearZ, f32 farZ);
  Mat44& LoadLookAt(f32 eyeX, f32 eyeY, f32 eyeZ, f32 atX, f32 atY, f32 atZ, f32 upX, f32 upY, f32 upZ);
  Mat44& LoadLookAt(const Vec3& eye, const Vec3& at, const Vec3& up);

  Mat44& Translate(f32 x, f32 y, f32 z = 0.0f);
  Mat44& Translate(const Vec3& pos);
  Mat44& Scale(f32 x, f32 y, f32 z = 1.0f);
  Mat44& Scale(const Vec3& scale);
  Mat44& Rotate(f32 angle, f32 x = 0.0f, f32 y = 0.0f, f32 z = 1.0f);
  Mat44& Rotate(f32 angle, const Vec3& axis);

  Mat44& Multiply(const Mat44& by);
  Mat44& MultiplyPre(const Mat44& by);
  Mat44& MultiplyOrtho(f32 x, f32 y, f32 w, f32 h, f32 nearZ, f32 farZ);
  Mat44& MultiplyPerspective(f32 fov, f32 aspect, f32 nearZ, f32 farZ);
  Mat44& MultiplyLookAt(f32 eyeX, f32 eyeY, f32 eyeZ, f32 atX, f32 atY, f32 atZ, f32 upX, f32 upY, f32 upZ);
  Mat44& MultiplyLookAt(const Vec3& eye, const Vec3& at, const Vec3& up);
  Vec2 Multiply(const Vec2& v, f32 z = 0.0f, f32 w = 1.0f) const;
  Vec3 Multiply(const Vec3& v, f32 w = 1.0f) const;
  Vec4 Multiply(const Vec4& v) const;
  void Multiply(s32 x, s32 y, s32& rx, s32& ry, s32& rz) const;
  void Multiply(s32 x, s32 y, s32& rx, s32& ry) const;
  void Multiply(f32 x, f32 y, f32& rx, f32& ry, f32& rz) const;
  void Multiply(f32 x, f32 y, f32& rx, f32& ry) const;

  bool Invert();
  Mat44& Transpose();
  Vec3 Reflect(const Vec3& incident, const Vec3& normal) const;
  Quat GetQuat() const;

  void Zero() {
    e11 = 0.0f;
    e21 = 0.0f;
    e31 = 0.0f;
    e41 = 0.0f;
    e12 = 0.0f;
    e22 = 0.0f;
    e32 = 0.0f;
    e42 = 0.0f;
    e13 = 0.0f;
    e23 = 0.0f;
    e33 = 0.0f;
    e43 = 0.0f;
    e14 = 0.0f;
    e24 = 0.0f;
    e34 = 0.0f;
    e44 = 0.0f;
  }

  inline void Print() {
    dbgprintf("[%f, %f, %f, %f]\n[%f, %f, %f, %f]\n[%f, %f, %f, %f]\n[%f, %f, %f, %f]\n",
      e11, e12, e13, e14,
      e21, e22, e23, e24,
      e31, e32, e33, e34,
      e41, e42, e43, e44);
  }

public:

  static const Mat44 Identity;

};

};
