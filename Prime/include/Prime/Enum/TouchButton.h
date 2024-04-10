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

typedef enum {
  TouchButtonNone = 0,
  TouchButton1 = 1,
  TouchButton2 = 2,
  TouchButton3 = 3,
  TouchButton4 = 4,
  TouchButton5 = 5,
  TouchButton6 = 6,
  TouchButton7 = 7,
  TouchButton8 = 8,
  TouchButton9 = 9,
  TouchButton10 = 10,
  TouchButton11 = 11,
  TouchButton12 = 12,
  TouchButton13 = 13,
  TouchButton14 = 14,
  TouchButton15 = 15,
  TouchButton16 = 16,
  TouchButton_Count = 17
} TouchButton;

#if defined(__cplusplus) && !defined(__INTELLISENSE__)
namespace std {
  template<> struct hash<TouchButton> {
    size_t operator()(const TouchButton& v) const noexcept {
      return hash<s32>()(v);
    }
  };
};
#endif

extern TouchButton GetEnumTouchButtonFromString(const std::string& s);
extern const char* GetEnumTouchButtonAsString(TouchButton v);

#define GetEnumTouchButtonCount() TouchButton_Count
#define GetEnumTouchButtonCountAsInt() 3
