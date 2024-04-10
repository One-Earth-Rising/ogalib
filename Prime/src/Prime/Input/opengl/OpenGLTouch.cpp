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

#include <Prime/Config.h>
#if defined(PrimeTargetOpenGL)

#include <Prime/Input/opengl/OpenGLTouch.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>
#include <Prime/Graphics/opengl/OpenGLGraphics.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

OpenGLTouch& OpenGLTouch::GetInstance() {
  PxRequireInit;

  if(instance) {
    PrimeAssert(dynamic_cast<OpenGLTouch*>(instance), "Touch instance is not a OpenGLTouch instance.");
    return *static_cast<OpenGLTouch*>(instance);
  }

  OpenGLTouch* inst = new OpenGLTouch();
  PrimeAssert(inst, "Could not create OpenGLTouch instance.");
  instance = inst;
  inst->Init();
  return *static_cast<OpenGLTouch*>(instance);
}

OpenGLTouch::OpenGLTouch(): Touch() {

}

OpenGLTouch::~OpenGLTouch() {

}

void OpenGLTouch::Init() {
  oldTouchInfoKnown = false;
}

void OpenGLTouch::Shutdown() {

}

void OpenGLTouch::StartFrame() {
  Touch::StartFrame();

  // Get current touch information.
  OpenGLTouchInfo touchInfo = GetOpenGLTouchInfo();

  if(oldTouchInfoKnown) {
    if(touchInfo.button2 && !oldTouchInfo.button2) {
      if(oldTouchInfo.button1) {
        AddToInputQueue(TouchParam::Make(-1, -1, oldTouchInfo.x, oldTouchInfo.y, TouchActionCancel, TouchButton1));
        oldTouchInfo.button1 = false;
      }
      AddToInputQueue(TouchParam::Make(touchInfo.x, touchInfo.y, -1, -1, TouchActionPress, TouchButton2));
    }
    else if(touchInfo.button1 && !oldTouchInfo.button1) {
      if(oldTouchInfo.button2) {
        AddToInputQueue(TouchParam::Make(-1, -1, oldTouchInfo.x, oldTouchInfo.y, TouchActionCancel, TouchButton2));
        oldTouchInfo.button2 = false;
      }
      AddToInputQueue(TouchParam::Make(touchInfo.x, touchInfo.y, -1, -1, TouchActionPress, TouchButton1));
    }
    else {
      bool held = touchInfo.button1 || touchInfo.button2;
      if(held) {
        TouchButton touchButton = touchInfo.button1 ? TouchButton1 : TouchButton2;
        if(touchInfo.x != oldTouchInfo.x || touchInfo.y != oldTouchInfo.y) {
          AddToInputQueue(TouchParam::Make(touchInfo.x, touchInfo.y, oldTouchInfo.x, oldTouchInfo.y, TouchActionDrag, touchButton));
        }
      }
      else {
        if(!touchInfo.button2 && oldTouchInfo.button2) {
          AddToInputQueue(TouchParam::Make(touchInfo.x, touchInfo.y, oldTouchInfo.x, oldTouchInfo.y, TouchActionRelease, TouchButton2));
        }
        else if(!touchInfo.button1 && oldTouchInfo.button1) {
          AddToInputQueue(TouchParam::Make(touchInfo.x, touchInfo.y, oldTouchInfo.x, oldTouchInfo.y, TouchActionRelease, TouchButton1));
        }
      }
    }
  }

  oldTouchInfo = touchInfo;
  oldTouchInfoKnown = true;
}

bool OpenGLTouch::GetMainCursorPos(f32& x, f32& y) const {
  OpenGLGraphics& g = PxOpenGLGraphics;
  GLFWwindow* window = g.GetOpenGLGLFWScreenWindow();
  double nativePosX, nativePosY;

  glfwGetCursorPos(window, &nativePosX, &nativePosY);

  x = g.MapWindowToScreenX((f32) nativePosX);
  y = g.MapWindowToScreenX((f32) nativePosY);

  return true;
}

bool OpenGLTouch::IsButtonHeld(TouchButton button) const {
  if(button >= TouchButton1 && button <= TouchButton8) {
    OpenGLGraphics& g = PxOpenGLGraphics;
    GLFWwindow* window = g.GetOpenGLGLFWScreenWindow();
    s32 nativeButton = button - TouchButton1 + GLFW_MOUSE_BUTTON_1;
    return glfwGetMouseButton(window, nativeButton) == GLFW_PRESS;
  }

  return false;
}

void OpenGLTouch::OnScroll(GLFWwindow* window, double x, double y) {
  // Get current touch information.
  OpenGLTouchInfo touchInfo = GetOpenGLTouchInfo();

  if(y != 0.0) {
    AddActionPressState(y > 0.0 ? TouchActionScrollUp : TouchActionScrollDown);
  }
  else if(x != 0.0) {
    AddActionPressState(x > 0.0 ? TouchActionScrollLeft : TouchActionScrollRight);
  }
}

OpenGLTouchInfo OpenGLTouch::GetOpenGLTouchInfo() const {
  OpenGLGraphics& g = PxOpenGLGraphics;
  GLFWwindow* window = g.GetOpenGLGLFWScreenWindow();
  OpenGLTouchInfo result;
  if(!window) {
    memset(&result, 0, sizeof(result));
    return result;
  }

  result.button1 = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
  result.button2 = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS;

  double nativePosX, nativePosY;
  glfwGetCursorPos(window, &nativePosX, &nativePosY);

  result.x = g.MapWindowToScreenX((f32) nativePosX);
  result.y = g.MapWindowToScreenY((f32) nativePosY);

  return result;
}

#endif
