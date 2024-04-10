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

#include <Prime/Engine.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#if defined(PrimeTargetWindows)
#include <Prime/Graphics/opengl/OpenGLGraphics.h>
#include <Prime/Input/opengl/OpenGLKeyboard.h>
#include <Prime/Input/opengl/OpenGLTouch.h>
#elif defined(PrimeTargetPS4)
#include <Prime/Graphics/ps4/PS4Graphics.h>
#elif defined(PrimeTargetPS5)
#include <Prime/Graphics/ps5/PS5Graphics.h>
#endif

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

bool Engine::initialized = false;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

namespace Prime {
extern void InitContent();
extern void ShutdownContent();
extern void ProcessContentRefs();
extern void ReleaseAllContent();
};

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

Engine& Engine::GetInstance() {
  static Engine instance;
  return instance;
}

Engine::Engine():
currentFrame(0),
running(false) {
  initialized = true;

  ogalib::Init();

#if defined(PrimeTargetWindows)
  PxOpenGLGraphics;
  PxOpenGLKeyboard;
  PxOpenGLTouch;
#elif defined(PrimeTargetPS4)
  PxPS4Graphics;
#elif defined(PrimeTargetPS5)
  PxPS5Graphics;
#endif

  InitContent();
}

Engine::~Engine() {
  WaitForNoJobs();
  ReleaseAllContent();
  ShutdownContent();

  PxTouch.Shutdown();
  PxKeyboard.Shutdown();
  PxGraphics.Shutdown();

  delete &PxTouch;
  delete &PxKeyboard;
  delete &PxGraphics;

  ogalib::Shutdown();

  initialized = false;
}

void Engine::Start() {
  running = true;

  lastFrameTime = GetSystemTime();
}

void Engine::Stop() {
  running = false;
}

f32 Engine::StartFrame() {
  f64 frameTime = GetSystemTime();
  f32 dt = (f32) (frameTime - lastFrameTime);
  lastFrameTime = frameTime;

  ogalib::Process();

  PxGraphics.StartFrame();
  PxKeyboard.StartFrame();
  PxTouch.StartFrame();

  return dt;
}

void Engine::EndFrame() {
  PxTouch.EndFrame();
  PxKeyboard.EndFrame();
  PxGraphics.EndFrame();
  
  ProcessContentRefs();

  currentFrame++;
}

void Engine::ProcessJobs() {
  if(Thread::IsMainThread()) {
    ogalib::Process();
  }
  Thread::Yield();
}

void Engine::WaitForNoJobs() {
  ogalib::WaitForNoJobs();
  Thread::Yield();
}
