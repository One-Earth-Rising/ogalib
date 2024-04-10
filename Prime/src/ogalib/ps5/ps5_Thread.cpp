/*
ogalib

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

#if defined(__PROSPERO__)

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <ogalib/ogalib.h>
#include <kernel.h>
#include <sceerror.h>

using namespace ogalib;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define OGALIB_SCE_THREAD_NAME_LENGTH 15
#define OGALIB_SCE_THREAD_COUNT 13

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {

typedef struct {
  ScePthreadMutex mutex;
} ThreadMutexPS5;

typedef struct {
  ScePthread thread;
} ThreadPS5;

typedef struct {
  ScePthreadMutex mutex;
  ScePthreadCond condition;
} ThreadConditionPS5;

};

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {
void* ThreadEntryFunction(void* param);
};

static std::string GetPS5ThreadName(const char* name);
static std::string GetPS5ThreadName(const std::string& name);
static int GetPS5ThreadPriority(float priority);

static __inline float GetLerp(float a, float b, float t) {
  if(t == 0.0f)
    return a;
  else if(t == 1.0f)
    return b;
  else
    return a + (b - a) * t;
}

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

// Should be 13 cores, matching the PS5ThreadSystem::GetDeviceThreadCount() result.
static const SceKernelCpumask PS5ThreadPreferredCoreMap[] = {
  (1ull << 0),
  (1ull << 2),
  (1ull << 4),
  (1ull << 6),
  (1ull << 8),
  (1ull << 10),
  (1ull << 12),
  (1ull << 1),
  (1ull << 3),
  (1ull << 5),
  (1ull << 7),
  (1ull << 9),
  (1ull << 11),
};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ThreadMutex::ThreadMutex(const char* name, bool recursive):
native(NULL) {
  this->name = name ? name : "";

  ThreadMutexPS5* nativePS5 = new ThreadMutexPS5;
  native = nativePS5;

  if(native) {
    int err;
    ScePthreadMutexattr attr;
    scePthreadMutexattrInit(&attr);

    std::string useName = GetPS5ThreadName(this->name);

    if(recursive) {
      scePthreadMutexattrSettype(&attr, SCE_PTHREAD_MUTEX_RECURSIVE);
      err = scePthreadMutexInit(&nativePS5->mutex, &attr, useName.c_str());
      ogalibAssert(err >= SCE_OK, "Error creating thread mutex: scePthreadMutexInit, 0x%08X", err);
    }
    else {
      scePthreadMutexattrSettype(&attr, SCE_PTHREAD_MUTEX_NORMAL);
      err = scePthreadMutexInit(&nativePS5->mutex, &attr, useName.c_str());
      ogalibAssert(err >= SCE_OK, "Error creating thread mutex: scePthreadMutexInit, 0x%08X", err);
    }

    scePthreadMutexattrDestroy(&attr);
  }
  else {
    ogalibAssert(false, "Could not allocate native data for thread mutex.");
  }
}

ThreadMutex::~ThreadMutex() {
  if(native) {
    ThreadMutexPS5* nativePS5 = static_cast<ThreadMutexPS5*>(native);

    int err = scePthreadMutexDestroy(&nativePS5->mutex);
    ogalibAssert(err >= SCE_OK, "Error deleting thread mutex: scePthreadMutexDestroy, 0x%08X", err);

    delete nativePS5;
  }
}

bool ThreadMutex::Lock() {
  if(native) {
    ThreadMutexPS5* nativePS5 = static_cast<ThreadMutexPS5*>(native);

    int result = scePthreadMutexLock(&nativePS5->mutex);
    return result == SCE_OK;
  }
  else {
    return false;
  }
}

bool ThreadMutex::TryLock() {
  if(native) {
    ThreadMutexPS5* nativePS5 = static_cast<ThreadMutexPS5*>(native);

    int result = scePthreadMutexTrylock(&nativePS5->mutex);
    return result == SCE_OK;
  }
  else {
    return false;
  }
}

bool ThreadMutex::Unlock() {
  if(native) {
    ThreadMutexPS5* nativePS5 = static_cast<ThreadMutexPS5*>(native);

    int result = scePthreadMutexUnlock(&nativePS5->mutex);
    return result == SCE_OK;
  }
  else {
    return false;
  }
}

Thread::Thread(std::function<void*(void*)> entry, void* param, const char* name):
entry(entry),
param(param),
result(NULL),
native(NULL),
threadId(0),
priority(0.0f),
preferredCore(-1),
started(false) {
  this->name = name ? name : "";

  ThreadPS5* nativePS5 = new ThreadPS5;
  native = nativePS5;

  if(native) {

  }
  else {
    ogalibAssert(false, "Could not allocate native data for thread.");
  }
}

Thread::~Thread() {
  if(started) {
    Join();
    started = false;
  }
}

bool Thread::Start() {
  if(started)
    return false;

  if(native) {
    ThreadPS5* nativePS5 = static_cast<ThreadPS5*>(native);

    std::string useName = GetPS5ThreadName(this->name);

    int callResult = scePthreadCreate(&nativePS5->thread, NULL, ThreadEntryFunction, this, useName.c_str());
    if(callResult == 0) {
      SceKernelCpumask cpuMask;
      if(preferredCore < 0) {
        cpuMask = SCE_KERNEL_CPUMASK_13CPU;
      }
      else {
        int usePreferredCore = preferredCore % OGALIB_SCE_THREAD_COUNT;
        cpuMask = PS5ThreadPreferredCoreMap[usePreferredCore];
      }

      scePthreadSetaffinity(nativePS5->thread, cpuMask);

      int ps5Priority = GetPS5ThreadPriority(priority);
      scePthreadSetprio(nativePS5->thread, ps5Priority);
    }

    started = callResult == 0;
    return started;
  }
  else {
    return false;
  }
}

bool Thread::Join() {
  if(native) {
    ThreadPS5* nativePS5 = static_cast<ThreadPS5*>(native);
    
    int callResult = scePthreadJoin(nativePS5->thread, NULL);
    return callResult == 0;
  }
  else {
    return false;
  }
}

void Thread::SetPriority(float priority) {
  if(native) {
    ThreadPS5* nativePS5 = static_cast<ThreadPS5*>(native);

    if(started) {
      int ps5Priority = GetPS5ThreadPriority(priority);
      scePthreadSetprio(nativePS5->thread, ps5Priority);
    }
  }
}

void Thread::SetPreferredCore(size_t core) {
  if(native) {
    ThreadPS5* nativePS5 = static_cast<ThreadPS5*>(native);

    if(started) {
      SceKernelCpumask cpuMask;
      if(preferredCore < 0) {
        cpuMask = SCE_KERNEL_CPUMASK_13CPU;
      }
      else {
        size_t usePreferredCore = preferredCore % Thread::GetDeviceThreadCount();
        cpuMask = PS5ThreadPreferredCoreMap[usePreferredCore];
      }

      scePthreadSetaffinity(nativePS5->thread, cpuMask);
    }
  }
}

void Thread::Sleep(double duration) {
  SceKernelUseconds durationValue = duration * (1000 * 1000);
  sceKernelUsleep(durationValue);
}

void Thread::Yield() {
  scePthreadYield();
}

size_t Thread::GetDeviceThreadCount() {
  return 13;
}

ThreadCondition::ThreadCondition(const char* name):
native(nullptr) {
  this->name = name ? name : "";

  ThreadConditionPS5* nativePS5 = new ThreadConditionPS5;
  native = nativePS5;

  if(native) {
    int err;
    ScePthreadMutexattr attr;
    scePthreadMutexattrInit(&attr);

    std::string useMutexName = GetPS5ThreadName(string_printf("%s (mutex)", this->name.c_str()));
    scePthreadMutexattrSettype(&attr, SCE_PTHREAD_MUTEX_NORMAL);
    err = scePthreadMutexInit(&nativePS5->mutex, &attr, useMutexName.c_str());
    ogalibAssert(err >= SCE_OK, "Error creating thread mutex: scePthreadMutexInit, 0x%08X", err);

    std::string useName = GetPS5ThreadName(this->name);
    scePthreadCondInit(&nativePS5->condition, NULL, useName.c_str());

    scePthreadMutexattrDestroy(&attr);
  }
}

ThreadCondition::~ThreadCondition() {
  int err;

  if(native) {
    ThreadConditionPS5* nativePS5 = static_cast<ThreadConditionPS5*>(native);

    err = scePthreadCondDestroy(&nativePS5->condition);
    ogalibAssert(err >= SCE_OK, "Error deleting thread condition: scePthreadCondDestroy, 0x%08X", err);

    err = scePthreadMutexDestroy(&nativePS5->mutex);
    ogalibAssert(err >= SCE_OK, "Error deleting thread mutex: scePthreadMutexDestroy, 0x%08X", err);

    delete nativePS5;
    native = nullptr;
  }
}

bool ThreadCondition::LockMutex() {
  if(native) {
    ThreadConditionPS5* nativePS5 = static_cast<ThreadConditionPS5*>(native);

    int result = scePthreadMutexLock(&nativePS5->mutex);
    return result == SCE_OK;
  }
  else {
    return false;
  }
}

bool ThreadCondition::TryLockMutex() {
  if(native) {
    ThreadConditionPS5* nativePS5 = static_cast<ThreadConditionPS5*>(native);

    int result = scePthreadMutexTrylock(&nativePS5->mutex);
    return result == SCE_OK;
  }
  else {
    return false;
  }
}

bool ThreadCondition::UnlockMutex() {
  if(native) {
    ThreadConditionPS5* nativePS5 = static_cast<ThreadConditionPS5*>(native);

    int result = scePthreadMutexUnlock(&nativePS5->mutex);
    return result == SCE_OK;
  }
  else {
    return false;
  }
}

bool ThreadCondition::Signal() {
  if(native) {
    ThreadConditionPS5* nativePS5 = static_cast<ThreadConditionPS5*>(native);

    return scePthreadCondSignal(&nativePS5->condition) == 0;;
  }
  else {
    return false;
  }
}

bool ThreadCondition::SignalAll() {
  if(native) {
    ThreadConditionPS5* nativePS5 = static_cast<ThreadConditionPS5*>(native);

    return scePthreadCondBroadcast(&nativePS5->condition) == 0;;
  }
  else {
    return false;
  }
}

bool ThreadCondition::Wait() {
  if(native) {
    ThreadConditionPS5* nativePS5 = static_cast<ThreadConditionPS5*>(native);

    bool result = true;

    int err = scePthreadCondWait(&nativePS5->condition, &nativePS5->mutex);
    if(err != SCE_OK && err != SCE_KERNEL_ERROR_ETIMEDOUT) {
      result = false;
    }

    return result;
  }
  else {
    return false;
  }
}

bool ThreadCondition::Wait(double duration) {
  if(native) {
    ThreadConditionPS5* nativePS5 = static_cast<ThreadConditionPS5*>(native);

    bool result = true;

    SceKernelUseconds durationValue = duration * (1000 * 1000);
    int err = scePthreadCondTimedwait(&nativePS5->condition, &nativePS5->mutex, durationValue);
    if(err != SCE_OK && err != SCE_KERNEL_ERROR_ETIMEDOUT) {
      result = false;
    }

    return result;
  }
  else {
    return false;
  }
}

void ThreadCondition::Signal(bool& wait) {
  ThreadConditionLock lock(*this);
  wait = false;
  Signal();
}

void ThreadCondition::Wait(bool& wait) {
  wait = true;

  ThreadConditionLock lock(*this);
  while(wait)
    Wait();
}

void ThreadCondition::Wait(double duration, bool& wait) {
  wait = true;

  ThreadConditionLock lock(*this);
  Wait(duration);
  wait = false;
}

void ThreadCondition::ShutdownThread(Thread*& thread) {
  if(!thread)
    return;

  while(thread->started) {
    Signal();
    if(thread->started) {
      Thread::Yield();
    }
  }

  if(thread) {
    delete thread;
    thread = nullptr;
  }
}

void ThreadCondition::ShutdownThread(Thread*& thread, bool& wait) {
  if(!thread)
    return;

  while(thread->started) {
    Signal(wait);
    if(thread->started) {
      Thread::Yield();
    }
  }

  if(thread) {
    delete thread;
    thread = nullptr;
  }
}

void* ogalib::ThreadEntryFunction(void* param) {
  Thread* thread = (Thread*) param;
  thread->threadId = scePthreadGetthreadid();

  if(thread->entry) {
    thread->result = thread->entry(thread->param);
  }

  thread->started = false;

  return NULL;
}

std::string GetPS5ThreadName(const char* name) {
  std::string nameStr = name;
  size_t len = strlen(nameStr.c_str());
  if(len > OGALIB_SCE_THREAD_NAME_LENGTH) {
    char resultC[OGALIB_SCE_THREAD_NAME_LENGTH + 1];
    memcpy(resultC, name, OGALIB_SCE_THREAD_NAME_LENGTH);
    resultC[OGALIB_SCE_THREAD_NAME_LENGTH] = 0;
    std::string result = resultC;
    return result;
  }
  else {
    return std::string(name);
  }
}

std::string GetPS5ThreadName(const std::string& name) {
  return GetPS5ThreadName(name.c_str());
}

int GetPS5ThreadPriority(float priority) {
  if(priority == -1.0f)
    return SCE_KERNEL_PRIO_FIFO_DEFAULT;

  float value = GetLerp(SCE_KERNEL_PRIO_FIFO_LOWEST, SCE_KERNEL_PRIO_FIFO_HIGHEST, priority);
  return roundf(value);
}

#endif
