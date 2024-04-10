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

#if defined(_WIN32) || defined(_WIN64)

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <ogalib/ogalib.h>

void ogalib::Thread::Yield() {
  std::this_thread::yield();
}

static __inline void YieldThread() {
  ogalib::Thread::Yield();
}

#include <mutex>
#include <thread>
#include <Windows.h>
#include <type_traits>

using namespace ogalib;

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {

typedef struct {
  std::mutex* mutex;
  std::recursive_mutex* recursiveMutex;
} ThreadMutexWindows;

typedef struct {
  std::thread* thread;
} ThreadWindows;

};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

long long Thread::mainThreadId = 0;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {
void* ThreadEntryFunction(void* param);
};

const DWORD MS_VC_EXCEPTION = 0x406D1388;  
#pragma pack(push,8)  
typedef struct tagTHREADNAME_INFO  
{  
  DWORD dwType; // Must be 0x1000.  
  LPCSTR szName; // Pointer to name (in user addr space).  
  DWORD dwThreadID; // Thread ID (-1=caller thread).  
  DWORD dwFlags; // Reserved for future use, must be zero.  
} THREADNAME_INFO;  
#pragma pack(pop)  
static void SetOGALibThreadName(DWORD dwThreadID, const char* threadName) {  
  THREADNAME_INFO info;  
  info.dwType = 0x1000;  
  info.szName = threadName;  
  info.dwThreadID = dwThreadID;  
  info.dwFlags = 0;  
#pragma warning(push)  
#pragma warning(disable: 6320 6322)  
  __try{  
    RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);  
  }  
  __except (EXCEPTION_EXECUTE_HANDLER){  
  }  
#pragma warning(pop)  
}

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ThreadMutex::ThreadMutex(const char* name, bool recursive):
native(nullptr) {
  this->name = name ? name : "";

  ThreadMutexWindows* nativeWindows = new ThreadMutexWindows;
  native = nativeWindows;

  if(native) {
    nativeWindows->mutex = nullptr;
    nativeWindows->recursiveMutex = nullptr;

    if(recursive) {
      nativeWindows->recursiveMutex = new std::recursive_mutex();
    }
    else {
      nativeWindows->mutex = new std::mutex();
    }
  }
  else {
    ogalibAssert(false, "Could not allocate native data for thread mutex.");
  }
}

ThreadMutex::~ThreadMutex() {
  if(native) {
    ThreadMutexWindows* nativeWindows = static_cast<ThreadMutexWindows*>(native);

    if(nativeWindows->mutex) {
      delete nativeWindows->mutex;
      nativeWindows->mutex = nullptr;
    }

    if(nativeWindows->recursiveMutex) {
      delete nativeWindows->recursiveMutex;
      nativeWindows->recursiveMutex = nullptr;
    }

    delete nativeWindows;
    native = nullptr;
  }
}

bool ThreadMutex::Lock() {
  if(native) {
    ThreadMutexWindows* nativeWindows = static_cast<ThreadMutexWindows*>(native);

    if(nativeWindows->mutex) {
      nativeWindows->mutex->lock();
      return true;
    }
    else if(nativeWindows->recursiveMutex) {
      nativeWindows->recursiveMutex->lock();
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

bool ThreadMutex::TryLock() {
  if(native) {
    ThreadMutexWindows* nativeWindows = static_cast<ThreadMutexWindows*>(native);

    if(nativeWindows->mutex) {
      return nativeWindows->mutex->try_lock();
    }
    else if(nativeWindows->recursiveMutex) {
      return nativeWindows->recursiveMutex->try_lock();
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

bool ThreadMutex::Unlock() {
  if(native) {
    ThreadMutexWindows* nativeWindows = static_cast<ThreadMutexWindows*>(native);

    if(nativeWindows->mutex) {
      nativeWindows->mutex->unlock();
      return true;
    }
    else if(nativeWindows->recursiveMutex) {
      nativeWindows->recursiveMutex->unlock();
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

Thread::Thread(std::function<void*(void*)> entry, void* param, const char* name):
entry(entry),
param(param),
result(nullptr),
native(nullptr),
threadId(0),
priority(0.0f),
preferredCore(-1),
started(false) {
  this->name = name ? name : "";

  ThreadWindows* nativeWindows = new ThreadWindows;
  native = nativeWindows;

  if(native) {
    nativeWindows->thread = nullptr;
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

  if(native) {
    ThreadWindows* nativeWindows = static_cast<ThreadWindows*>(native);

    if(nativeWindows->thread) {
      nativeWindows->thread->join();
      delete nativeWindows->thread;
      nativeWindows->thread = nullptr;
    }

    delete nativeWindows;
    native = nullptr;
  }
}

bool Thread::Start() {
  if(started)
    return false;

  if(native) {
    ThreadWindows* nativeWindows = static_cast<ThreadWindows*>(native);

    nativeWindows->thread = new std::thread(ThreadEntryFunction, this);
    if(nativeWindows->thread) {
      HANDLE hThread = nativeWindows->thread->native_handle();
      if(priority == -1.0f) {
        SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
      }
      else if(priority >= 0.9f) {
        SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
      }
      else if(priority == 0.0f) {
        SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
      }
      else {
        SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
      }
    }

    started = nativeWindows->thread != nullptr;
    return started;
  }
  else {
    return false;
  }
}

bool Thread::Join() {
  if(native) {
    ThreadWindows* nativeWindows = static_cast<ThreadWindows*>(native);

    if(nativeWindows->thread) {
      nativeWindows->thread->join();
      delete nativeWindows->thread;
      nativeWindows->thread = nullptr;
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

void Thread::SetPriority(float priority) {
  if(native) {
    ThreadWindows* nativeWindows = static_cast<ThreadWindows*>(native);

    if(nativeWindows->thread) {
      HANDLE hThread = nativeWindows->thread->native_handle();
      if(priority == -1.0f) {
        SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
      }
      else if(priority >= 0.9f) {
        SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
      }
      else if(priority == 0.0f) {
        SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
      }
      else {
        SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
      }
    }
  }
}

void Thread::SetPreferredCore(size_t core) {

}

bool Thread::IsMainThread() {
  return mainThreadId == ::GetCurrentThreadId();
}

void Thread::Sleep(double duration) {
  std::this_thread::sleep_for(std::chrono::nanoseconds((long long) (1000000000 * duration)));
}

size_t Thread::GetDeviceThreadCount() {
  size_t count = std::thread::hardware_concurrency();

  if(count == 0)
    count = 1;

  return count;
}

void Thread::InitGlobal() {
  mainThreadId = ::GetCurrentThreadId();
}

void Thread::ShutdownGlobal() {

}

ThreadConditionLock::ThreadConditionLock(ThreadCondition& condition): std::unique_lock<std::mutex>(*static_cast<ThreadCondition*>(&condition)->mutex),
condition(condition) {
  ThreadCondition* threadConditionPC = static_cast<ThreadCondition*>(&this->condition);
  threadConditionPC->SetCurrentLock(this);
}

ThreadConditionLock::ThreadConditionLock(ThreadCondition* condition): std::unique_lock<std::mutex>(*static_cast<ThreadCondition*>(condition)->mutex),
condition(*condition) {
  ThreadCondition* threadConditionPC = static_cast<ThreadCondition*>(&this->condition);
  threadConditionPC->SetCurrentLock(this);
}

ThreadConditionLock::~ThreadConditionLock() {
  ThreadCondition* threadConditionPC = static_cast<ThreadCondition*>(&this->condition);
  threadConditionPC->SetCurrentLock(NULL);
}

ThreadCondition::ThreadCondition(const char* name):
currentLock(NULL) {
  this->name = name ? name : "";

  mutex = new std::mutex();
  condition = new std::condition_variable();
}

ThreadCondition::~ThreadCondition() {
  if(condition) {
    delete condition;
    condition = nullptr;
  }

  if(mutex) {
    delete mutex;
    mutex = nullptr;
  }
}

bool ThreadCondition::LockMutex() {
  mutex->lock();
  return true;
}

bool ThreadCondition::TryLockMutex() {
  return mutex->try_lock();
}

bool ThreadCondition::UnlockMutex() {
  mutex->unlock();
  return true;
}

bool ThreadCondition::Signal() {
  condition->notify_one();
  return true;
}

bool ThreadCondition::SignalAll() {
  condition->notify_all();
  return true;
}

bool ThreadCondition::Wait() {
  if(currentLock) {
    condition->wait(*currentLock);
    return true;
  }
  else {
    std::unique_lock<std::mutex> lock(*mutex);
    condition->wait(lock);
    return true;
  }
}

bool ThreadCondition::Wait(double duration) {
  if(currentLock) {
    std::cv_status status = condition->wait_for(*currentLock, std::chrono::nanoseconds((long long) (1000000000.0 * duration)));
    return true;
  }
  else {
    std::unique_lock<std::mutex> lock(*mutex);
    std::cv_status status = condition->wait_for(lock, std::chrono::nanoseconds((long long) (1000000000.0 * duration)));
    return status == std::cv_status::timeout;
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
      YieldThread();
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
      YieldThread();
    }
  }

  if(thread) {
    delete thread;
    thread = nullptr;
  }
}

void ThreadCondition::SetCurrentLock(ThreadConditionLock* currentLock) {
  this->currentLock = currentLock;
}

void* ogalib::ThreadEntryFunction(void* param) {
  Thread* thread = (Thread*) param;
  thread->threadId = ::GetCurrentThreadId();
  SetOGALibThreadName((DWORD) (-1), thread->name.c_str());

  if(thread->entry) {
    thread->result = thread->entry(thread->param);
  }

  thread->started = false;

  return nullptr;
}

#endif
