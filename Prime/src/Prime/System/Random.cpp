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

#include <Prime/System/Random.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

Random Random::instance;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Random::Random() {
#ifdef _DEBUG
  seeded = false;
#endif
  tinymt_init(&mt);
  Seed(1);
}

Random::Random(const Random& other) {
  tinymt_init(&mt);
  tinymt_copy(&mt, &other.mt);
}

Random::~Random() {
  tinymt_destroy(&mt);
}

Random& Random::operator=(const Random& other) {
  tinymt_copy(&mt, &other.mt);
  return *this;
}

void Random::Seed(s32 seed) {
#ifdef _DEBUG
  seeded = true;
#endif
  tinymt_seed(&mt, seed);
}

u32 Random::GetValue() {
#ifdef _DEBUG
  if(!seeded) {
    dbgprintf("[Warning] Random not seeded.\n");
  }
#endif
  return tinymt_rand(&mt);
}

u32 Random::GetValueMax() const {
  return tinymt_rand_max(&mt);
}

f32 Random::GetValueF() {
#ifdef _DEBUG
  if(!seeded) {
    dbgprintf("[Warning] Random not seeded.\n");
  }
#endif
  return (f32) (tinymt_rand(&mt) / (f64) tinymt_rand_max(&mt));
}

u32 Random::GetRange(u32 low, u32 high) {
  if(low == high)
    return low;
  else if(low == 0 && high == tinymt_rand_max(&mt))
    return GetValue();

  u32 range = high - low + 1;
  return low + (GetValue() % range);
}

s32 Random::GetRange(s32 low, s32 high) {
  if(low == high)
    return low;
  else if(low == 0x80000000 && high == 0x7FFFFFFF)
    return GetValue();

  u32 range = high - low + 1;
  return low + (GetValue() % range);
}

f32 Random::GetRange(f32 low, f32 high) {
#ifdef _DEBUG
  if(!seeded) {
    dbgprintf("[Warning] Random not seeded.\n");
  }
#endif
  if(low == high)
    return low;

  f64 range = high - low;
  return (f32) (low + ((tinymt_rand(&mt) / (f64) tinymt_rand_max(&mt)) * range));
}

void Random::FillArrayS32(s32* a, u32 size, s32 start, u32 step) {
  s32 stepValue = start;
  u32 i;
  
  if(size == 0)
    return;

  for(i = 0; i < size; i++) {
    a[i] = stepValue;
    stepValue += step;
  }
  
  for(i = 0; i < size; i++) {
    u32 r = i + (GetValue() % size);
    s32 t = a[i];
    a[i] = a[r];
    a[r] = t;
  }
}

void Random::FillArrayU32(u32* a, u32 size, s32 start, u32 step) {
  u32 stepValue = start;
  u32 i;
  
  if(size == 0)
    return;

  for(i = 0; i < size; i++) {
    a[i] = stepValue;
    stepValue += step;
  }
  
  for(i = 0; i < size; i++) {
    u32 r = i + (GetValue() % size);
    u32 t = a[i];
    a[i] = a[r];
    a[r] = t;
  }
}

u32 Random::GetWeightedChoice(u32* a, u32 size) {
  u32 sum = 0;
  u32 i;
  u32 r;
  
  if(size == 0)
    return 0;
  
  for(i = 0; i < size; i++)
    sum += a[i];

  if(sum == 0)
    return GetValue() % size;
  else
    r = GetValue() % sum;
  
  sum = 0;
  for(i = 0; i < size; i++) {
    sum += a[i];
    if(sum > r)
      break;
  }
  
  return min(i, size - 1);
}
