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
  KeyNone = 0,
  KeyEscape = 256,
  KeyEnter = 257,
  KeyTab = 258,
  KeyBackspace = 259,
  KeyInsert = 260,
  KeyDelete = 261,
  KeyRight = 262,
  KeyLeft = 263,
  KeyDown = 264,
  KeyUp = 265,
  KeyPageUp = 266,
  KeyPageDown = 267,
  KeyHome = 268,
  KeyEnd = 269,
  KeyCapsLock = 270,
  KeyScrollLock = 271,
  KeyNumLock = 272,
  KeyPrintScreen = 273,
  KeyPause = 274,
  KeyF1 = 275,
  KeyF2 = 276,
  KeyF3 = 277,
  KeyF4 = 278,
  KeyF5 = 279,
  KeyF6 = 280,
  KeyF7 = 281,
  KeyF8 = 282,
  KeyF9 = 283,
  KeyF10 = 284,
  KeyF11 = 285,
  KeyF12 = 286,
  KeyF13 = 287,
  KeyF14 = 288,
  KeyF15 = 289,
  KeyF16 = 290,
  KeyF17 = 291,
  KeyF18 = 292,
  KeyF19 = 293,
  KeyF20 = 294,
  KeyF21 = 295,
  KeyF22 = 296,
  KeyF23 = 297,
  KeyF24 = 298,
  KeyF25 = 299,
  KeyNumPad0 = 300,
  KeyNumPad1 = 301,
  KeyNumPad2 = 302,
  KeyNumPad3 = 303,
  KeyNumPad4 = 304,
  KeyNumPad5 = 305,
  KeyNumPad6 = 306,
  KeyNumPad7 = 307,
  KeyNumPad8 = 308,
  KeyNumPad9 = 309,
  KeyNumPadDecimal = 310,
  KeyNumPadDivide = 311,
  KeyNumPadMultiply = 312,
  KeyNumPadSubtract = 313,
  KeyNumPadAdd = 314,
  KeyNumPadEnter = 315,
  KeyNumPadEqual = 316,
  KeyLShift = 317,
  KeyLCtrl = 318,
  KeyLAlt = 319,
  KeyLSuper = 320,
  KeyRShift = 321,
  KeyRCtrl = 322,
  KeyRAlt = 323,
  KeyRSuper = 324,
  KeyMenu = 325,
  Key_Count = 71
} Key;

#if defined(__cplusplus) && !defined(__INTELLISENSE__)
namespace std {
  template<> struct hash<Key> {
    size_t operator()(const Key& v) const noexcept {
      return hash<s32>()(v);
    }
  };
};
#endif
