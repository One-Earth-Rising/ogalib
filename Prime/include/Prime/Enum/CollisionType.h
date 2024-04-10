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

#include <Prime/Config.h>

typedef enum {
  CollisionTypeNone = 0,
  CollisionTypeLevel = 1,
  CollisionTypeBoundary = 2,
  CollisionTypePlatform = 3,
  CollisionTypeLadder = 4,
  CollisionTypeForce = 5,
  CollisionTypeObject = 6,
  CollisionTypeBlock = 7,
  CollisionTypeGhost = 8,
  CollisionTypeDamage = 9,
  CollisionTypeHazard = 10,
  CollisionTypeDebris = 11,
  CollisionTypePickup = 12,
  CollisionTypeCamera = 13,
  CollisionTypeInteraction = 14,
  CollisionTypeGrapple = 15,
  CollisionTypeDoor = 16,
  CollisionTypeInvisibleWall = 17,
  CollisionType_Count = 18
} CollisionType;

#if defined(__cplusplus) && !defined(__INTELLISENSE__)
namespace std {
  template<> struct hash<CollisionType> {
    size_t operator()(const CollisionType& v) const noexcept {
      return hash<s32>()(v);
    }
  };
};
#endif

extern CollisionType GetEnumCollisionTypeFromString(const std::string& s);
extern const char* GetEnumCollisionTypeAsString(CollisionType v);

#define GetEnumCollisionTypeCount() CollisionType_Count
#define GetEnumCollisionTypeCountAsInt() 18
