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

#include <ogalib/Job.h>
#include <functional>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define ogalibRequireInit ogalibAssert(ogalib::IsInitialized(), "ogalib is not initialized.");

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {

class Data {
public:

  bool initialized;
  json initParams;
  
  std::string baseAPI;
  std::string apiKey;
  json globalSendURLParams;
  bool encodeURLRequests;

  bool loginInProgress;
  size_t userId;
  size_t token;

  json assetCache;
  json assetCacheInProgress;
  ThreadMutex* assetCacheMutex;

public:

  Data();

};

};

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {

void Init(const json& params = json());
void Shutdown();
void Process();

bool IsInitialized();

void SetBaseAPI(const std::string& baseAPI, bool encodeURLRequests = false);
void SetAPIKey(const std::string& apiKey);

ThreadMutex* GetJobMutex();
void WaitForNoJobs();

void SetGlobalSendURLParams(const json& params);

void SendURL(const std::string& url);
void SendURL(const std::string& url, const json& params);
void SendURL(const std::string& url, const std::function<void(const json&)>& callback);
void SendURL(const std::string& url, const json& params, const std::function<void(const json&)>& callback);
bool SendURL(const std::string& url, const json& params, json& result);

void Login(std::function<void(const json&)> callback = nullptr);
bool IsLoginInProgress();
void WaitForLogin(std::function<void()> callback = nullptr);

void GetAssetByURL(const std::string& url, std::function<void(const json&)> callback = nullptr);

void GetActiveBattlepass(std::function<void(const json&)> callback);

void GetBattlepassProgress(size_t battlepassId, std::function<void(const json&)> callback = nullptr);
void IncBattlepassProgress(size_t battlepassId, size_t amount, std::function<void(const json&)> callback = nullptr);
void ResetBattlepassProgress(size_t battlepassId, std::function<void(const json&)> callback = nullptr);

std::string string_printf(const char* f, ...);
std::string string_vprintf(const char* f, va_list ap, va_list ap2);
std::string EncodeURL(const char* f, ...);
std::string DecodeURL(const char* f);

};
