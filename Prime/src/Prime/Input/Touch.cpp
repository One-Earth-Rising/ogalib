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

#include <Prime/Input/Touch.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PRIME_TOUCH_INPUT_QUEUE_CAPACITY 128

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

Touch* Touch::instance = nullptr;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Touch& Touch::GetInstance() {
  PxRequireInit;

  if(instance)
    return *instance;

  PrimeAssert(false, "Target did not create a Touch instance.");
  instance = new Touch();
  PrimeAssert(instance, "Could not create Touch instance.");
  instance->Init();
  return *instance;
}

Touch::Touch():
queue(nullptr) {
  memset(lastButtonHeld, 0, sizeof(lastButtonHeld));
}

Touch::~Touch() {

}

void Touch::Init() {
  ResetDragInfo();
  InitInputQueue(PRIME_TOUCH_INPUT_QUEUE_CAPACITY);
}

void Touch::Shutdown() {
  DestroyInputQueue();
}

void Touch::StartFrame() {
  for(size_t i = 0; i < GetEnumTouchButtonCount(); i++) {
    TouchButton touchButton = (TouchButton) i;
    lastButtonHeld[i] = buttonHeld[i];
    buttonHeld[i] = IsButtonHeld(touchButton);
  }
}

void Touch::EndFrame() {
  actionPressed.Clear();
  actionReleased.Clear();
  actionHeld.Clear();
}

void Touch::ClearInput() {
  queueStart = 0;
  queueSize = 0;
  ResetDragInfo();
  actionPressed.Clear();
  actionReleased.Clear();
  actionHeld.Clear();
}

bool Touch::GetMainCursorPos(f32& x, f32& y) const {
  return false;
}

bool Touch::IsButtonHeld(TouchButton button) const {
  return buttonHeld[button];
}

bool Touch::IsButtonPressed(TouchButton button) const {
  return !lastButtonHeld[button] && buttonHeld[button];
}

bool Touch::IsButtonReleased(TouchButton button) const {
  return lastButtonHeld[button] && !buttonHeld[button];
}

void Touch::ResetDragInfo() {
  dragInfo.itemStart = 0;
  dragInfo.itemCount = 0;
  UpdateTouchDragAverage();
}

void Touch::AppendDragInfo(f32 x, f32 y, f32 x0, f32 y0) {
  u32 index;
  Vec2* item;

  if(dragInfo.itemCount < PRIME_TOUCH_DRAG_INFO_ITEM_COUNT) {
    index = dragInfo.itemCount++;
  }
  else {
    dragInfo.itemStart++;
    if(dragInfo.itemStart >= PRIME_TOUCH_DRAG_INFO_ITEM_COUNT)
      dragInfo.itemStart = 0;
    index = dragInfo.itemStart;
  }

  item = &dragInfo.items[index];
  item->x = x - x0;
  item->y = y - y0;
}

void Touch::UpdateTouchDragAverage() {
  if(dragInfo.itemCount == 0) {
    dragInfo.averageX = 0.0f;
    dragInfo.averageY = 0.0f;
    return;
  }

  f32 averageX = 0.0f;
  f32 averageY = 0.0f;

  u32 index = (dragInfo.itemStart + dragInfo.itemCount) % PRIME_TOUCH_DRAG_INFO_ITEM_COUNT;
  for(u32 i = 0; i < dragInfo.itemCount; i++) {
    Vec2* item = &dragInfo.items[index++];
    if(index >= PRIME_TOUCH_DRAG_INFO_ITEM_COUNT)
      index = 0;
    averageX += item->x;
    averageY += item->y;
  }

  dragInfo.averageX = averageX / (f32) dragInfo.itemCount;
  dragInfo.averageY = averageY / (f32) dragInfo.itemCount;
}

void Touch::GetAverageDrag(f32& x, f32& y) {
  if(dragInfo.itemCount) {
    UpdateTouchDragAverage();
    x = dragInfo.averageX;
    y = dragInfo.averageY;
  }
  else {
    x = 0.0f;
    y = 0.0f;
  }
}

u32 Touch::GetInputQueueSize() const {
  return queueSize;
}

bool Touch::GetFromInputQueue(TouchParam& param) {
  if(queueSize > 0) {
    param = queue[queueStart++];
    if(queueStart >= queueCapacity)
      queueStart = 0;
    queueSize--;

    if(param.action == TouchActionPress) {
      ResetDragInfo();
    }
    else if(param.action == TouchActionDrag || param.action == TouchActionRelease) {
      AppendDragInfo(param.x, param.y, param.x0, param.y0);
    }

    return true;
  }

  return false;
}

bool Touch::IsActionPressed(TouchAction action) const {
  if(auto it = actionPressed.Find(action)) {
    return it.value();
  }
  return false;
}

bool Touch::IsActionReleased(TouchAction action) const {
  if(auto it = actionReleased.Find(action)) {
    return it.value();
  }
  return false;
}

bool Touch::IsActionHeld(TouchAction action) const {
  if(auto it = actionHeld.Find(action)) {
    return it.value();
  }
  return false;
}

void Touch::InitInputQueue(u32 capacity) {
  DestroyInputQueue();

  queue = new TouchParam[capacity];
  queueStart = 0;
  queueSize = 0;
  if(queue) {
    queueCapacity = capacity;
  }
}

void Touch::DestroyInputQueue() {
  if(queue) {
    delete queue;
    queueCapacity = 0;
    queueSize = 0;
    queueStart = 0;
  }
}

bool Touch::AddToInputQueue(const TouchParam& param) {
  if(queueSize < queueCapacity) {
    u32 index = queueStart + queueSize;
    if(index >= queueCapacity)
      index -= queueCapacity;
    queue[index] = param;
    queueSize++;
    return true;
  }

  return false;
}

void Touch::AddActionPressState(TouchAction action) {
  actionPressed[action] = true;
  actionHeld[action] = true;
}

void Touch::RemoveActionPressState(TouchAction action) {
  actionPressed.Remove(action);
  actionHeld.Remove(action);
  actionReleased[action] = true;
}
