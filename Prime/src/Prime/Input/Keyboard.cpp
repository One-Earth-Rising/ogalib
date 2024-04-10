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

#include <Prime/Input/Keyboard.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

Keyboard* Keyboard::instance = nullptr;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Keyboard& Keyboard::GetInstance() {
  PxRequireInit;

  if(instance)
    return *instance;

  PrimeAssert(false, "Target did not create a Keyboard instance.");
  instance = new Keyboard();
  PrimeAssert(instance, "Could not create Keyboard instance.");
  instance->Init();
  return *instance;
}

Keyboard::Keyboard() {

}

Keyboard::~Keyboard() {

}

void Keyboard::Init() {

}

void Keyboard::Shutdown() {

}

void Keyboard::StartFrame() {

}

void Keyboard::EndFrame() {
  keyPressed.Clear();
  keyReleased.Clear();
}

bool Keyboard::IsKeyPressed(Key key, KeyFlag keyFlag) const {
  PrimeAssert(keyFlag == KeyFlagNone, "Key flag query is currently unsupported.");
  if(auto it = keyPressed.Find(key)) {
    return it.value();
  }
  return false;
}

bool Keyboard::IsKeyReleased(Key key, KeyFlag keyFlag) const {
  PrimeAssert(keyFlag == KeyFlagNone, "Key flag query is currently unsupported.");
  if(auto it = keyReleased.Find(key)) {
    return it.value();
  }
  return false;
}

bool Keyboard::IsKeyHeld(Key key, KeyFlag keyFlag) const {
  PrimeAssert(keyFlag == KeyFlagNone, "Key flag query is currently unsupported.");
  if(auto it = keyHeld.Find(key)) {
    return it.value();
  }
  return false;
}

bool Keyboard::IsKeyPressed(char key, KeyFlag keyFlag) const {
  return IsKeyPressed((Key) key, keyFlag);
}

bool Keyboard::IsKeyReleased(char key, KeyFlag keyFlag) const {
  return IsKeyReleased((Key) key, keyFlag);
}

bool Keyboard::IsKeyHeld(char key, KeyFlag keyFlag) const {
  return IsKeyHeld((Key) key, keyFlag);
}

void Keyboard::AddKeyboardPressState(s32 key) {
  bool alreadyHeld = false;

  if(auto it = keyHeld.Find(key)) {
    alreadyHeld = it.value();
  }

  if(!alreadyHeld) {
    keyPressed[key] = true;
  }

  keyHeld[key] = true;
}

void Keyboard::RemoveKeyboardPressState(s32 key) {
  keyPressed.Remove(key);
  keyHeld.Remove(key);
  keyReleased[key] = true;
}
