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
#include <Prime/Types/Vec2.h>
#include <Prime/Types/TypeStack.h>
#include <Prime/Types/Dictionary.h>
#include <Prime/Enum/TouchAction.h>
#include <Prime/Enum/TouchButton.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PxTouch Touch::GetInstance()

#define PRIME_TOUCH_DRAG_INFO_ITEM_COUNT 5

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _TouchParam {
  f32 x, y, x0, y0;
  TouchAction action;
  TouchButton button;

  static inline struct _TouchParam Make(f32 x, f32 y, f32 x0, f32 y0, TouchAction action, TouchButton button) {
    struct _TouchParam result;
    result.x = x;
    result.y = y;
    result.x0 = x0;
    result.y0 = y0;
    result.action = action;
    result.button = button;
    return result;
  }

} TouchParam;

typedef struct _TouchDragInfo {
  Vec2 items[PRIME_TOUCH_DRAG_INFO_ITEM_COUNT];
  u32 itemStart;
  u32 itemCount;
  f32 averageX;
  f32 averageY;
} TouchDragInfo;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Touch {
friend class Engine;
protected:

  static Touch* instance;

public:

  static Touch& GetInstance();

protected:

  TouchParam* queue;
  u32 queueCapacity;
  u32 queueStart;
  u32 queueSize;

  TouchDragInfo dragInfo;

  bool lastButtonHeld[GetEnumTouchButtonCount()];
  bool buttonHeld[GetEnumTouchButtonCount()];

  Dictionary<u32, bool> actionPressed;
  Dictionary<u32, bool> actionHeld;
  Dictionary<u32, bool> actionReleased;

protected:

  Touch();

public:

  virtual ~Touch();

protected:

  virtual void Init();
  virtual void Shutdown();

public:

  virtual void StartFrame();
  virtual void EndFrame();

  virtual void ClearInput();

  virtual bool GetMainCursorPos(f32& x, f32& y) const;
  virtual bool IsButtonHeld(TouchButton button) const;
  virtual bool IsButtonPressed(TouchButton button) const;
  virtual bool IsButtonReleased(TouchButton button) const;

  virtual void ResetDragInfo();
  virtual void AppendDragInfo(f32 x, f32 y, f32 x0, f32 y0);
  virtual void UpdateTouchDragAverage();
  virtual void GetAverageDrag(f32& x, f32& y);

  virtual u32 GetInputQueueSize() const;
  virtual bool GetFromInputQueue(TouchParam& param);

  virtual bool IsActionPressed(TouchAction action) const;
  virtual bool IsActionReleased(TouchAction action) const;
  virtual bool IsActionHeld(TouchAction action) const;

public:

  virtual void InitInputQueue(u32 capacity);
  virtual void DestroyInputQueue();
  virtual bool AddToInputQueue(const TouchParam& param);

  virtual void AddActionPressState(TouchAction action);
  virtual void RemoveActionPressState(TouchAction action);

};

};
