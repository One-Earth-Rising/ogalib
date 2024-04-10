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

#include <Prime/Input/opengl/OpenGLKeyboard.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>
#include <Prime/Graphics/Graphics.h>
#include <Prime/Enum/Key.h>
#include <Prime/Enum/KeyFlag.h>
#include <Prime/Enum/KeyboardAction.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

const s32 OpenGLKeyboardNativeKeyMap[] = {
  KeyEscape,
  KeyEnter,
  KeyTab,
  KeyBackspace,
  KeyInsert,
  KeyDelete,
  KeyRight,
  KeyLeft,
  KeyDown,
  KeyUp,
  KeyPageUp,
  KeyPageDown,
  KeyHome,
  KeyEnd,
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyCapsLock,
  KeyScrollLock,
  KeyNumLock,
  KeyPrintScreen,
  KeyPause,
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyF1,
  KeyF2,
  KeyF3,
  KeyF4,
  KeyF5,
  KeyF6,
  KeyF7,
  KeyF8,
  KeyF9,
  KeyF10,
  KeyF11,
  KeyF12,
  KeyF13,
  KeyF14,
  KeyF15,
  KeyF16,
  KeyF17,
  KeyF18,
  KeyF19,
  KeyF20,
  KeyF21,
  KeyF22,
  KeyF23,
  KeyF24,
  KeyF25,
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNumPad0,
  KeyNumPad1,
  KeyNumPad2,
  KeyNumPad3,
  KeyNumPad4,
  KeyNumPad5,
  KeyNumPad6,
  KeyNumPad7,
  KeyNumPad8,
  KeyNumPad9,
  KeyNumPadDecimal,
  KeyNumPadDivide,
  KeyNumPadMultiply,
  KeyNumPadSubtract,
  KeyNumPadAdd,
  KeyNumPadEnter,
  KeyNumPadEqual,
  KeyNone,   // empty
  KeyNone,   // empty
  KeyNone,   // empty
  KeyLShift,
  KeyLCtrl,
  KeyLAlt,
  KeyLSuper,
  KeyRShift,
  KeyRCtrl,
  KeyRAlt,
  KeyRSuper,
  KeyMenu,
};

static const s32 OpenGLKeyboardNativeKeyMapCount = sizeof(OpenGLKeyboardNativeKeyMap) / sizeof(OpenGLKeyboardNativeKeyMap[0]);

#define OPENGL_KEYBOARD_NATIVE_KEY_MAP_START GLFW_KEY_ESCAPE

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

OpenGLKeyboard& OpenGLKeyboard::GetInstance() {
  PxRequireInit;

  if(instance) {
    PrimeAssert(dynamic_cast<OpenGLKeyboard*>(instance), "Keyboard instance is not a OpenGLKeyboard instance.");
    return *static_cast<OpenGLKeyboard*>(instance);
  }

  OpenGLKeyboard* inst = new OpenGLKeyboard();
  PrimeAssert(inst, "Could not create OpenGLKeyboard instance.");
  instance = inst;
  inst->Init();
  return *static_cast<OpenGLKeyboard*>(instance);
}

OpenGLKeyboard::OpenGLKeyboard(): Keyboard() {

}

OpenGLKeyboard::~OpenGLKeyboard() {

}

void OpenGLKeyboard::Init() {

}

void OpenGLKeyboard::Shutdown() {

}

void OpenGLKeyboard::OnKey(GLFWwindow* window, int sysKey, int sysScancode, int sysAction, int sysMods) {
  s32 key;
  KeyboardAction action;
  u32 flags = 0;

  switch(sysAction) {
  case GLFW_PRESS:
  case GLFW_REPEAT:
    action = KeyboardActionPress;
    break;

  case GLFW_RELEASE:
    action = KeyboardActionRelease;
    break;
  }

  if(sysKey >= OPENGL_KEYBOARD_NATIVE_KEY_MAP_START && sysKey < OPENGL_KEYBOARD_NATIVE_KEY_MAP_START + OpenGLKeyboardNativeKeyMapCount) {
    key = OpenGLKeyboardNativeKeyMap[sysKey - OPENGL_KEYBOARD_NATIVE_KEY_MAP_START];
  }
  else {
    key = sysKey;
  }

  if(key < GLFW_KEY_LEFT_SHIFT || key >= GLFW_KEY_MENU) {
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT))
      flags |= KeyFlagShift;

    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL))
      flags |= KeyFlagCtrl;

    if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) || glfwGetKey(window, GLFW_KEY_RIGHT_ALT))
      flags |= KeyFlagAlt;

    if(glfwGetKey(window, GLFW_KEY_LEFT_SUPER) || glfwGetKey(window, GLFW_KEY_RIGHT_SUPER))
      flags |= KeyFlagCmd;
  }

  if(GetKeyState(VK_CAPITAL) != 0)
    flags |= KeyFlagCapsLock;

  if(GetKeyState(VK_NUMLOCK) != 0)
    flags |= KeyFlagNumLock;

  bool shift = (flags & KeyFlagShift) != 0;
  bool capslock = (flags & KeyFlagCapsLock) != 0;

  if(shift != capslock)
    flags |= KeyFlagShifted;

  bool keyComboPressedQuit = false;
  bool keyComboPressedFullscreenToggle = false;

  if(key == GLFW_KEY_F4 && (flags & KeyFlagAlt) != 0) {
    keyComboPressedQuit = true;
  }
  else if(key == GLFW_KEY_ENTER && (flags & KeyFlagAlt) != 0) {
    keyComboPressedFullscreenToggle = true;
  }

  if(keyComboPressedQuit) {
    PxEngine.Stop();
  }
  else if(keyComboPressedFullscreenToggle) {
    PxGraphics.RequestFullscreenToggle();
  }
  else {
    bool pressed = action == KeyboardActionPress;
    if(pressed) {
      AddKeyboardPressState(key);
    }
    else {
      RemoveKeyboardPressState(key);
    }
  }
}

#endif
