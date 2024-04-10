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
  TouchActionNone = 0,
  TouchActionPress = 1,
  TouchActionRelease = 2,
  TouchActionDrag = 3,
  TouchActionCancel = 4,
  TouchActionHover = 5,
  TouchActionScrollUp = 6,
  TouchActionScrollDown = 7,
  TouchActionScrollLeft = 8,
  TouchActionScrollRight = 9,
  TouchAction_Count = 10
} TouchAction;

#if defined(__cplusplus) && !defined(__INTELLISENSE__)
namespace std {
  template<> struct hash<TouchAction> {
    size_t operator()(const TouchAction& v) const noexcept {
      return hash<s32>()(v);
    }
  };
};
#endif
