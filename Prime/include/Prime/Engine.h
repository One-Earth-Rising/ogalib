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

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PxEngine Prime::Engine::GetInstance()
#define PxRequireInit PrimeAssert(Prime::Engine::IsInitialized(), "Prime Engine is not initialized.")

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Engine {
friend class Engine;
private:

  static bool initialized;

public:

  static Engine& GetInstance();
  static inline bool IsInitialized() {return initialized;}

private:

  f64 lastFrameTime;
  size_t currentFrame;
  bool running;

public:

  size_t GetCurrentFrame() const {return currentFrame;}
  bool IsRunning() const {return running;}

protected:

  Engine();
  virtual ~Engine();

public:

  virtual void Start();
  virtual void Stop();

  virtual f32 StartFrame();
  virtual void EndFrame();

  virtual void ProcessJobs();
  virtual void WaitForNoJobs();

};

};
