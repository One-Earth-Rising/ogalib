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

#include <Prime/Enum/CollisionType.h>
#include <Prime/Types/Dictionary.h>

static const char* CollisionTypeAsStringTable[] = {
  "CollisionTypeNone",
  "CollisionTypeLevel",
  "CollisionTypeBoundary",
  "CollisionTypePlatform",
  "CollisionTypeLadder",
  "CollisionTypeForce",
  "CollisionTypeObject",
  "CollisionTypeBlock",
  "CollisionTypeGhost",
  "CollisionTypeDamage",
  "CollisionTypeHazard",
  "CollisionTypeDebris",
  "CollisionTypePickup",
  "CollisionTypeCamera",
  "CollisionTypeInteraction",
  "CollisionTypeGrapple",
  "CollisionTypeDoor",
  "CollisionTypeInvisibleWall",
};

static const Prime::Dictionary<std::string, CollisionType> CollisionTypeFromStringLookup = {
  {"None", CollisionTypeNone},
  {"Level", CollisionTypeLevel},
  {"Boundary", CollisionTypeBoundary},
  {"Platform", CollisionTypePlatform},
  {"Ladder", CollisionTypeLadder},
  {"Force", CollisionTypeForce},
  {"Object", CollisionTypeObject},
  {"Block", CollisionTypeBlock},
  {"Ghost", CollisionTypeGhost},
  {"Damage", CollisionTypeDamage},
  {"Hazard", CollisionTypeHazard},
  {"Debris", CollisionTypeDebris},
  {"Pickup", CollisionTypePickup},
  {"Camera", CollisionTypeCamera},
  {"Interaction", CollisionTypeInteraction},
  {"Grapple", CollisionTypeGrapple},
  {"Door", CollisionTypeDoor},
  {"InvisibleWall", CollisionTypeInvisibleWall},
};

CollisionType GetEnumCollisionTypeFromString(const std::string& s) {
  if(auto it = CollisionTypeFromStringLookup.Find(s))
    return it.value();
  else
    return (CollisionType) 0;
}

const char* GetEnumCollisionTypeAsString(CollisionType v) {
  return CollisionTypeAsStringTable[v];
}
