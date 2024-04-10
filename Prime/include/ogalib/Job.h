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

#include <ogalib/Thread.h>
#include <functional>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {

enum class JobType {
  Default = 0,
  Independent = 1,
  Express = 2,
};

class Job {
friend void Init(const json& params);
friend void WaitForNoJobs();
friend void Shutdown();
friend void Process();
friend void* JobThread(void*);
friend void* JobWorkerThread(void*);
private:

  std::function<void(Job&)> callback;
  std::function<void(Job&)> response;
  ogalib::Thread* thread;
  bool completed;
  bool canceled;
  JobType type;

public:

  json data;
  void* param;
  std::string error;

public:

  Job(std::function<void(Job&)> callback, std::function<void(Job&)> response, JobType type);
  Job(std::function<void(Job&)> callback, std::function<void(Job&)> response, const json& data = json(), JobType type = JobType::Default);
  ~Job();

private:

  void InitCommon();

public:

  void Call(void* param = nullptr, const std::string& error = std::string());
  void Cancel();
  void Shutdown();

private:

  static void InitGlobal();
  static void ShutdownGlobal();
  static void ProcessGlobal();
  static bool HasJobs();

  static void InitWorkerThread();
  static void ShutdownWorkerThread();

};

};
