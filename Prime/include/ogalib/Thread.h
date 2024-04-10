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

#pragma once

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <ogalib/Types.h>
#include <functional>
#if defined(_WIN32) || defined(_WIN64)
#include <mutex>
#ifdef Yield
#undef Yield
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {

class json;

class ThreadMutex {
private:

  std::string name;
  void* native;

public:

  ThreadMutex(const char* name = nullptr, bool recursive = false);
  virtual ~ThreadMutex();

public:

  bool Lock();
  bool TryLock();
  bool Unlock();

};

class Thread {
friend void Init(const json& params);
friend void* ThreadEntryFunction(void*);
friend class ThreadCondition;
private:

  std::string name;
  std::function<void*(void*)> entry;
  void* param;
  void* result;
  void* native;
  long long threadId;
  float priority;
  int preferredCore;
  bool started;

  static long long mainThreadId;

public:

  Thread(std::function<void*(void*)> entry, void* param, const char* name = nullptr);
  virtual ~Thread();

public:

  bool Start();
  bool Join();
  void SetPriority(float priority);
  void SetPreferredCore(size_t core);

public:

  static bool IsMainThread();
  static void Yield();
  static void Sleep(double duration);
  static size_t GetDeviceThreadCount();

private:

  static void InitGlobal();
  static void ShutdownGlobal();

};

class ThreadCondition {
friend class ThreadConditionLock;
protected:

  std::string name;

#if defined(_WIN32) || defined(_WIN64)
  std::mutex* mutex;
  std::condition_variable* condition;
  ThreadConditionLock* currentLock;
#else
  void* native;
#endif

public:

  ThreadCondition(const char* name = nullptr);
  virtual ~ThreadCondition();

public:

  bool LockMutex();
  bool TryLockMutex();
  bool UnlockMutex();

  bool Signal();
  bool SignalAll();
  bool Wait();
  bool Wait(double duration);

  void Signal(bool& wait);
  void Wait(bool& wait);
  void Wait(double duration, bool& wait);

  void ShutdownThread(Thread*& thread);
  void ShutdownThread(Thread*& thread, bool& wait);

#if defined(_WIN32) || defined(_WIN64)
  void SetCurrentLock(ThreadConditionLock* currentLock);
#endif

};

#if defined(_WIN32) || defined(_WIN64)
class ThreadConditionLock: public std::unique_lock<std::mutex> {
private:

  ThreadCondition& condition;

public:

  ThreadConditionLock(ThreadCondition& condition);
  ThreadConditionLock(ThreadCondition* condition);
  ~ThreadConditionLock();

};
#else
class ThreadConditionLock {
private:

  ThreadCondition& condition;

public:

  ThreadConditionLock(ThreadCondition& condition): condition(condition) {
    condition.LockMutex();
  }

  ThreadConditionLock(ThreadCondition* condition): condition(*condition) {
    this->condition.LockMutex();
  }

  ~ThreadConditionLock() {
    condition.UnlockMutex();
  }

};
#endif

};
