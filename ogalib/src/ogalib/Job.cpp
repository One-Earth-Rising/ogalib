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

#include <ogalib/Job.h>

using namespace ogalib;

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <ogalib/ogalib.h>
#include <set>
#include <list>

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ThreadMutex* jobMutex = nullptr;
static std::set<Job*> jobs;
static std::list<Job*> jobQueue;
static std::vector<Job*> jobCompletedStack;
static size_t jobsInProgress = 0;

static bool* workerThreadActive = nullptr;
static bool* workerThreadWait = nullptr;
static ThreadMutex* workerThreadMutex = nullptr;
static ThreadCondition** workerThreadCondition = nullptr;
static Thread** workerThread = nullptr;
static uint32_t* workerThreadNumbers = nullptr;
static uint32_t workerThreadCount = 0;
static uint32_t workerThreadIndex = 0;

static const std::string CanceledErrorStr("canceled");

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {
void* JobThread(void* param);
void* JobWorkerThread(void* param);
};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Job::Job(std::function<void(Job&)> callback, std::function<void(Job&)> response, JobType type):
callback(callback),
response(response),
thread(nullptr),
completed(false),
canceled(false),
type(type) {
  InitCommon();
}

Job::Job(std::function<void(Job&)> callback, std::function<void(Job&)> response, const json& data, JobType type):
callback(callback),
response(response),
thread(nullptr),
completed(false),
canceled(false),
type(type),
data(data) {
  InitCommon();
}

Job::~Job() {
  if(thread)
    delete thread;
}

void Job::InitCommon() {
  if(type == JobType::Independent) {
    jobMutex->Lock();
    jobs.insert(this);
    jobMutex->Unlock();

    if(callback) {
      thread = new Thread(JobThread, this);
      thread->Start();
    }
    else {
      completed = true;
    }
  }
  else {
    if(callback) {
      workerThreadMutex->Lock();
      jobQueue.push_back(this);
      workerThreadMutex->Unlock();

      workerThreadMutex->Lock();
      workerThreadCondition[workerThreadIndex]->Signal(workerThreadWait[workerThreadIndex]);
      if(++workerThreadIndex >= workerThreadCount)
        workerThreadIndex = 0;
      workerThreadMutex->Unlock();
    }
    else {
      completed = true;

      workerThreadMutex->Lock();
      jobCompletedStack.push_back(this);
      workerThreadMutex->Unlock();
    }
  }
}

void Job::Call(void* param, const std::string& error) {
  this->param = param;
  this->error = error;
  if(response) {
    response(*this);
  }
}

void Job::Cancel() {
  canceled = true;
}

void Job::Shutdown() {

}

void* ogalib::JobThread(void* param) {
  Job* job = static_cast<Job*>(param);
  if(job->callback) {
    job->callback(*job);
  }
  job->completed = true;
  return nullptr;
}

void Job::InitGlobal() {
  jobMutex = new ThreadMutex("Job Callback Mutex");

  InitWorkerThread();
}

void Job::ShutdownGlobal() {
  ShutdownWorkerThread();

  std::vector<Job*> removeCallbacks;
  for(auto jc: jobs) {
    removeCallbacks.push_back(jc);
  }

  for(auto jc: removeCallbacks) {
    delete jc;
  }

  jobs.clear();

  if(jobMutex) {
    delete jobMutex;
    jobMutex = nullptr;
  }
}

void Job::ProcessGlobal() {
  if(jobQueue.size() > 0) {
    workerThreadMutex->Lock();
    workerThreadCondition[workerThreadIndex]->Signal(workerThreadWait[workerThreadIndex]);
    if(++workerThreadIndex >= workerThreadCount)
      workerThreadIndex = 0;
    workerThreadMutex->Unlock();
  }

  if(jobs.size() == 0 && jobCompletedStack.size() == 0)
    return;

  std::vector<Job*> processJobs;
  std::vector<Job*> removeJobs;

  workerThreadMutex->Lock();
  for(auto jc: jobCompletedStack) {
    processJobs.push_back(jc);
  }  
  jobCompletedStack.clear();
  workerThreadMutex->Unlock();

  for(auto jc: processJobs) {
    if(jc->canceled) {
      jc->Call(&jc->data, CanceledErrorStr);
      removeJobs.push_back(jc);
    }
    else if(jc->completed) {
      jc->Call(&jc->data);
      removeJobs.push_back(jc);
    }
  }

  for(auto jc: removeJobs) {
    delete jc;
  }

  processJobs.clear();
  removeJobs.clear();

  jobMutex->Lock();
  for(auto jc: jobs) {
    processJobs.push_back(jc);
  }
  jobMutex->Unlock();

  for(auto jc: processJobs) {
    if(jc->canceled) {
      jc->Call(&jc->data, CanceledErrorStr);
      removeJobs.push_back(jc);
    }
    else if(jc->completed) {
      jc->Call(&jc->data);
      removeJobs.push_back(jc);
    }
  }

  jobMutex->Lock();
  for(auto jc: removeJobs) {
    jobs.erase(jc);
  }
  jobMutex->Unlock();

  for(auto jc: removeJobs) {
    delete jc;
  }
}

bool Job::HasJobs() {
  size_t count;

  jobMutex->Lock();
  count = jobsInProgress + jobs.size() + jobQueue.size() + jobCompletedStack.size();
  jobMutex->Unlock();

  return count > 0;
}

void Job::InitWorkerThread() {
  workerThreadMutex = new ThreadMutex("ogalib::Job worker thread mutex", true);

  uint32_t maxWorkerThreadCount = (uint32_t) Thread::GetDeviceThreadCount();

  workerThreadCount = std::min(maxWorkerThreadCount, (uint32_t) OGALIB_JOB_CALLBACK_WORKER_COUNT);
  if(workerThreadCount < 1)
    workerThreadCount = 1;
  workerThreadCondition = new ThreadCondition*[workerThreadCount];
  workerThread = new Thread*[workerThreadCount];
  workerThreadNumbers = new uint32_t[workerThreadCount];
  workerThreadActive = new bool[workerThreadCount];
  workerThreadWait = new bool[workerThreadCount];

  for(uint32_t i = 0; i < workerThreadCount; i++) {
    workerThreadActive[i] = true;
    workerThreadWait[i] = true;
    workerThreadNumbers[i] = i;
    workerThreadCondition[i] = new ThreadCondition(string_printf("ogalib::Job worker thread condition (%d)", i).c_str());
    workerThread[i] = new Thread(JobWorkerThread, &workerThreadNumbers[i], string_printf("ogalib::Job worker thread (%d)", i).c_str());
    workerThread[i]->SetPriority(OGALIB_JOB_CALLBACK_WORKER_THREAD_PRIORITY);
    if(!workerThread[i]->Start()) {
      ogalibAssert(false, "Could not start ogalib::Job worker thread.");
    }
  }
}

void Job::ShutdownWorkerThread() {
  for(uint32_t i = 0; i < workerThreadCount; i++) {
    workerThreadActive[i] = false;
  }

  for(uint32_t i = 0; i < workerThreadCount; i++) {
    if(workerThreadCondition[i]) {
      workerThreadCondition[i]->ShutdownThread(workerThread[i], workerThreadWait[i]);
      delete workerThreadCondition[i];
      workerThreadCondition[i] = nullptr;
    }
  }

  if(workerThreadMutex) {
    delete workerThreadMutex;
    workerThreadMutex = nullptr;
  }

  if(workerThreadCondition) {
    delete[] workerThreadCondition;
    workerThreadCondition = nullptr;
  }

  if(workerThread) {
    delete[] workerThread;
    workerThread = nullptr;
  }

  if(workerThreadNumbers) {
    delete[] workerThreadNumbers;
    workerThreadNumbers = nullptr;
  }

  if(workerThreadActive) {
    delete[] workerThreadActive;
    workerThreadActive = nullptr;
  }

  if(workerThreadWait) {
    delete[] workerThreadWait;
    workerThreadWait = nullptr;
  }
}

void* ogalib::JobWorkerThread(void* param) {
  uint32_t* workerThreadNumbers = (uint32_t*) param;
  uint32_t workerThreadNumber = *workerThreadNumbers;

  while(workerThreadActive[workerThreadNumber]) {
    Job* job = nullptr;

    size_t jobCount = jobQueue.size();        
    if(jobCount > 0) {
      workerThreadMutex->Lock();

      if(workerThreadNumber == 0 && workerThreadCount > 1) {
        // This thread is designated as an "express lane" for jobs that perform quickly.
        for(auto& currJob: jobQueue) {
          if(currJob->type == JobType::Express) {
            job = currJob;
            jobQueue.remove(currJob);
            jobsInProgress++;
            break;
          }
        }
      }
      else {
        if(jobQueue.size() > 0) {
          job = jobQueue.front();
          jobQueue.pop_front();
          jobsInProgress++;
        }
      }

      workerThreadMutex->Unlock();
    }

    if(job) {
      if(job->callback) {
        job->callback(*job);
      }
      job->completed = true;

      workerThreadMutex->Lock();
      jobCompletedStack.push_back(job);
      if(jobsInProgress > 0) {
        jobsInProgress--;
      }
      workerThreadMutex->Unlock();

      Thread::Yield();
    }

    workerThreadCondition[workerThreadNumber]->Wait(workerThreadWait[workerThreadNumber]);
  }

  return nullptr;
}
