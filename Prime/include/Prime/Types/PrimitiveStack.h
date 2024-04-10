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

#include <Prime/Config.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PRIME_PRIMITIVE_STACK_DEFAULT_CAPACITY 16

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

template <class T>
class PrimitiveStack {
private:

  T value;
  T* stack;
  size_t count;
  size_t capacity;

public:

  PrimitiveStack():
  value(T()),
  stack(NULL),
  count(0),
  capacity(0) {

  }

  virtual ~PrimitiveStack() {
    if(stack)
      delete[] stack;
  }

public:

  operator T() const {return value;}

  virtual PrimitiveStack& operator=(const PrimitiveStack& t) {
    value = t.value;
    return *this;
  }

  virtual PrimitiveStack& operator=(const T& t) {
    value = t;
    return *this;
  }

  T& operator*() const {return *value;}

  void Allocate(size_t count) {
    PrimeSafeDeleteArray(stack);

    this->count = 0;

    stack = new T[count];
    capacity = stack ? count : 0;
  }

  size_t GetItemCount() const {
    return count;
  }

  const T& GetItem(size_t index) const {
    PrimeAssert(count > 0, "Stack is empty.");
    return count > 0 ? stack[index % count] : stack[0];
  }
  
  T& GetItem(size_t index) {
    PrimeAssert(count > 0, "Stack is empty.");
    return count > 0 ? stack[index % count] : stack[0];
  }

  const T& GetTopItem() const {
    PrimeAssert(count > 0, "Stack is empty.");
    return count > 0 ? GetItem(count - 1) : stack[0];
  }

  T& GetTopItem() {
    PrimeAssert(count > 0, "Stack is empty.");
    return count > 0 ? GetItem(count - 1) : stack[0];
  }

  void ClearAllItems() {
    count = 0;
  }
  
  virtual PrimitiveStack& Push() {
    if(capacity == 0)
      Allocate(PRIME_PRIMITIVE_STACK_DEFAULT_CAPACITY);

    if(count < capacity) {
      stack[count++] = *this;
    }
#ifdef _DEBUG
    else {
      PrimeAssert(false, "Pushed a full stack.");
    }
#endif
    return *this;
  }

  virtual PrimitiveStack& Pop() {
    if(count > 0) {
      *this = stack[--count];
    }
#ifdef _DEBUG
    else {
      PrimeAssert(false, "Popped an empty stack.");
    }
#endif
    return *this;
  }

};

typedef PrimitiveStack<bool> BoolStack;

};
