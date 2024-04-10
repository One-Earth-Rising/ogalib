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
  JoystickButtonNone = 0,
  JoystickButtonUp = 1,
  JoystickButtonDown = 2,
  JoystickButtonLeft = 3,
  JoystickButtonRight = 4,
  JoystickButtonSelect = 5,
  JoystickButtonStart = 6,
  JoystickButton1 = 7,
  JoystickButton2 = 8,
  JoystickButton3 = 9,
  JoystickButton4 = 10,
  JoystickButton5 = 11,
  JoystickButton6 = 12,
  JoystickButton7 = 13,
  JoystickButton8 = 14,
  JoystickButton_Count = 15
} JoystickButton;

#if defined(__cplusplus) && !defined(__INTELLISENSE__)
namespace std {
  template<> struct hash<JoystickButton> {
    size_t operator()(const JoystickButton& v) const noexcept {
      return hash<s32>()(v);
    }
  };
};
#endif
