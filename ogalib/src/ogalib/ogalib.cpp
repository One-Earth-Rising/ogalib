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

#include <ogalib/ogalib.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#if defined(OGALIB_USING_STEAM)
#include <ogalib/steam/steam_ogalib.h>
#elif defined(__PROSPERO__)
#include <ogalib/ps5/ps5_ogalib.h>
#endif
#include <ogalib/md5/md5.h>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdarg.h>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

#ifdef Yield
#undef Yield
#endif

using namespace ogalib;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define StringBufferSizeStack (8 * 1024)
#define StringBufferSize (64 * 1024)

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

ogalib::Data ogalibData;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

Data::Data():
initialized(false),
encodeURLRequests(false),
loginInProgress(false),
userId(0),
token(0) {

}

void ogalib::Init(const json& params) {
  if(ogalibData.initialized) {
    ogalibAssert(!ogalibData.initialized, "ogalib is already initialized.");
    return;
  }

  ogalibData.initialized = true;

  ogalibData.initParams = params;

  ogalibData.baseAPI = OGALIB_API_ROOT;
  ogalibData.globalSendURLParams = json::object();

  Thread::InitGlobal();
  Job::InitGlobal();

  ogalibData.assetCacheMutex = new ThreadMutex();

#if defined(OGALIB_USING_STEAM)
  InitSteam();
#elif defined(__PROSPERO__)
  InitPS5();
#endif
}

void ogalib::Shutdown() {
  if(!ogalibData.initialized) {
    ogalibAssert(ogalibData.initialized, "ogalib is not initialized.");
    return;
  }

  while(Job::HasJobs()) {
    ogalib::Process();
    Thread::Yield();
  }

#if defined(OGALIB_USING_STEAM)
  FinalizeSteam();
#elif defined(__PROSPERO__)
  FinalizePS5();
#endif

  if(ogalibData.assetCacheMutex) {
    delete ogalibData.assetCacheMutex;
    ogalibData.assetCacheMutex = NULL;
  }

  Job::ShutdownGlobal();

  ogalibData.initialized = false;
}

void ogalib::Process() {
  ogalibRequireInit;

  if(Thread::IsMainThread()) {
    Job::ProcessGlobal();

#if defined(OGALIB_USING_STEAM)
    ProcessSteam();
#endif
  }
}

bool ogalib::IsInitialized() {
  return ogalibData.initialized;
}

void ogalib::SetBaseAPI(const std::string& baseAPI, bool encodeURLRequests) {
  ogalibRequireInit;

  ogalibData.baseAPI = baseAPI;
  ogalibData.encodeURLRequests = std::string(ogalibData.baseAPI).find("?") != std::string::npos;
}

void ogalib::SetAPIKey(const std::string& apiKey) {
  ogalibRequireInit;

  ogalibData.apiKey = apiKey;
}

void ogalib::WaitForNoJobs() {
  while(Job::HasJobs()) {
    ogalib::Process();
    Thread::Yield();
  }
}

void ogalib::SetGlobalSendURLParams(const json& params) {
  ogalibData.globalSendURLParams = params;
}

void ogalib::SendURL(const std::string& url) {
  SendURL(url, {}, [](const json&) {});
}

void ogalib::SendURL(const std::string& url, const json& params) {
  SendURL(url, {}, [](const json&) {});
}

void ogalib::SendURL(const std::string& url, const std::function<void(const json&)>& callback) {
  SendURL(url, {}, callback);
}

void ogalib::SendURL(const std::string& url, const json& params, const std::function<void(const json&)>& callback) {
  ogalibRequireInit;

  new Job([=](Job& job) {
    json useParams = ogalibData.globalSendURLParams;
    useParams += params;

    if(auto it = useParams.find("usesAPIKey")) {
      auto& value = it.value();
      if(value.IsBool() && value.GetBool()) {
        if(ogalibData.apiKey.length() > 0) {
          useParams["authorizationBearerToken"] = ogalibData.apiKey;
        }
      }
    }

    job.data["sendURLResult"] = SendURL(url.c_str(), useParams, job.data);
  }, [=](Job& job) {
    if(callback) {
      callback(job.data);
    }
  });
}

void ogalib::Login(std::function<void(const json&)> callback) {
  ogalibRequireInit;

  if(ogalibData.loginInProgress) {
    if(callback) {
      callback({
        {"error", "ogalib login is already in progress."},
        });
    }
  }
  else {
    ogalibData.loginInProgress = true;

    new Job(nullptr, [=](Job& cb) {
      ogalibAssert(ogalibData.initialized, "ogalib is not initialized.");
      if(!ogalibData.initialized) {
        if(callback) {
          callback({
            {"error", "ogalib is not initialized."},
            });
        }
      }

      ogalibData.userId = 0;
      ogalibData.token = 0;

#if defined(OGALIB_USING_STEAM)
      LoginUsingSteam(callback);
#elif defined(__PROSPERO__)
      LoginUsingPS5(callback);
#endif
    });
  }
}

bool ogalib::IsLoginInProgress() {
  ogalibRequireInit;

  return ogalibData.loginInProgress;
}

void ogalib::WaitForLogin(std::function<void()> callback) {
  ogalibRequireInit;

  if(IsLoginInProgress()) {
    new Job([=](Job& cb) {
      while(IsLoginInProgress()) {
        Thread::Sleep(0.1);
        Thread::Yield();
      }
    }, [=](Job& cb) {
      if(callback) {
        callback();
      }
    });
  }
  else {
    if(callback) {
      callback();
    }
  }
}

void ogalib::GetAssetByURL(const std::string& url, std::function<void(const json&)> callback) {
  ogalibRequireInit;

  if(url.empty()) {
    if(callback) {
      callback({
        {"error", "Bad URL."},
        });
    }

    return;
  }
  else {
    std::string useURL(url);

    if(auto it = ogalibData.assetCache.find(useURL)) {
      if(callback) {
        callback(it);
      }

      return;
    }
    else {
      ogalibData.assetCacheMutex->Lock();
      bool inProgress = ogalibData.assetCacheInProgress.find(useURL);
      ogalibData.assetCacheMutex->Unlock();

      if(inProgress) {
        new Job([=](Job& job) {
          while(true) {
            ogalibData.assetCacheMutex->Lock();
            bool cacheInProgress = ogalibData.assetCacheInProgress.find(useURL);
            ogalibData.assetCacheMutex->Unlock();

            if(!cacheInProgress) {
              break;
            }

            Thread::Yield();
          }
        }, [=](Job& job) {
          if(auto it = ogalibData.assetCache.find(useURL)) {
            if(callback) {
              callback(it);
            }
          }
          else {
            if(callback) {
              callback({
                {"error", "Asset not found."},
                });
            }
          }
        });

        return;
      }
    }

    ogalibData.assetCacheMutex->Lock();
    ogalibData.assetCacheInProgress[useURL] = true;

    json sendURLParams;
    sendURLParams["ignoreSSLErrors"] = true;

    SendURL(url, sendURLParams, [=](const json& data) {
      ogalibData.assetCacheMutex->Lock();
      ogalibData.assetCacheInProgress.erase(useURL);
      ogalibData.assetCacheMutex->Unlock();

      json asset;
      asset["url"] = std::string(useURL);

      uint8_t md5[16];
      char md5Str[32 + 1];

      MD5_CTX ctx;
      MD5_Init(&ctx);
      MD5_Update(&ctx, useURL.c_str(), (uint32_t) useURL.length());
      MD5_Final(md5, &ctx);

      for(uint32_t i = 0; i < 16; i++) {
        sprintf(&md5Str[i * 2], "%02x", md5[i]);
      }
      md5Str[32] = 0;

      asset["md5_url"] = std::string(md5Str);

      if(auto it = data.find("error")) {
        if(callback) {
          callback({
            {"error", it.c_str()},
            });
        }
      }
      else {
        if(auto it = data.find("response")) {
          std::string response = it.GetString();
          size_t fileDataSize = response.size();
          bool foundAsset = false;

          // Check for GLB.
          if(!foundAsset) {
            uint8_t header[12];
            if(fileDataSize >= sizeof(header)) {
              memcpy(header, &response[0], sizeof(header));

              if(memcmp(&header, "glTF", 4) == 0) {
                //uint32_t readVersion = header[4] | (header[5] << 8) | (header[6] << 16) | (header[7] << 24);
                uint32_t readSize = header[8] | (header[9] << 8) | (header[10] << 16) | (header[11] << 24);

                if(readSize == fileDataSize) {
                  asset["gltf"] = response;
                  foundAsset = true;
                }
              }
            }
          }

          // Check for PNG.
          if(!foundAsset) {
            uint8_t header[8];
            if(fileDataSize >= sizeof(header)) {
              memcpy(header, &response[0], sizeof(header));

              static unsigned char pngSignature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
              if(!memcmp(header, pngSignature, 8)) {
                asset["png"] = response;
                foundAsset = true;
              }
            }
          }

          if(foundAsset) {
            ogalibData.assetCache[useURL] = asset;

            if(callback) {
              callback(asset);
            }
          }
          else {
            if(callback) {
              callback({
                {"error", "Did not find an asset."},
                });
            }
          }
        }
        else {
          if(callback) {
            callback({
              {"error", "Did not receive a response."},
              });
          }
        }
      }
    });
  }

  ogalibData.assetCacheMutex->Unlock();
}

void ogalib::GetActiveBattlepass(std::function<void(const json&)> callback) {
  ogalibRequireInit;

  WaitForLogin([=]() {
    json sendURLParams;
    sendURLParams["usesAPIKey"] = true;
    sendURLParams["ignoreSSLErrors"] = true;

    SendURL(string_printf("%s/GetActiveBattlepass/v1/", ogalibData.baseAPI.c_str()).c_str(), sendURLParams, [=](const json& response) {
      if(auto it = response.find("error")) {
        if(callback) {
          callback({
            {"error", it.c_str()},
            });
        }
      }
      else if(auto it = response.find("response")) {
        json js;
        if(js.parse(it.c_str())) {
          if(!js.find("error")) {
            if(js.size() > 0) {
              auto battlepass = js.at(0);
              if(auto itTiers = battlepass.find("tiers")) {
                for(auto& tier: itTiers) {
                  if(auto itURL = tier.find("rewardAssetURL")) {
                    auto url = itURL.GetString();
                    if(!url.empty()) {
                      // Get the asset without a callback which caches the asset in the process.
                      GetAssetByURL(url.c_str());
                    }
                  }
                }
              }
            }
          }

          if(callback) {
            callback(js);
          }
        }
        else {
          callback({
            {"error", js.error()},
            });
        }
      }
      else {
        if(callback) {
          callback({
            {"error", "Response not found."},
            });
        }
      }
    });
  });
}

void ogalib::GetBattlepassProgress(size_t battlepassId, std::function<void(const json&)> callback) {
  ogalibRequireInit;

  if(battlepassId == 0) {
    if(callback) {
      callback({
        {"error", "Invalid battlepass id."},
        });
    }

    return;
  }

  WaitForLogin([=]() {
    std::string params = string_printf("?playerAccountId=%zu&token=%zu&battlepassId=%zu", ogalibData.userId, ogalibData.token, battlepassId);
    if(ogalibData.encodeURLRequests)
      params = EncodeURL(params.c_str());

    json sendURLParams;
    sendURLParams["usesAPIKey"] = true;
    sendURLParams["ignoreSSLErrors"] = true;

    SendURL(string_printf("%s/GetBattlepassProgress/v1/%s", ogalibData.baseAPI.c_str(), params.c_str()).c_str(), sendURLParams, [=](const json& response) {
      if(auto it = response.find("error")) {
        if(callback) {
          callback({
            {"error", it.c_str()},
            });
        }
      }
      else if(auto it = response.find("response")) {
        if(callback) {
          json js;
          if(js.parse(it.c_str())) {
            callback(js);
          }
          else {
            callback({
              {"error", js.error()},
              });
          }
        }
      }
      else {
        if(callback) {
          callback({
            {"error", "Response not found."},
            });
        }
      }
    });
  });
}

void ogalib::IncBattlepassProgress(size_t battlepassId, size_t amount, std::function<void(const json&)> callback) {
  ogalibRequireInit;

  if(battlepassId == 0) {
    if(callback) {
      callback({
        {"error", "Invalid battlepass id."},
        });
    }

    return;
  }

  WaitForLogin([=]() {
    std::string params = string_printf("?playerAccountId=%zu&token=%zu&battlepassId=%zu&amount=%zu", ogalibData.userId, ogalibData.token, battlepassId, amount);
    if(ogalibData.encodeURLRequests)
      params = EncodeURL(params.c_str());

    json sendURLParams;
    sendURLParams["usesAPIKey"] = true;
    sendURLParams["ignoreSSLErrors"] = true;

    SendURL(string_printf("%s/IncBattlepassProgress/v1/%s", ogalibData.baseAPI.c_str(), params.c_str()).c_str(), sendURLParams, [=](const json& response) {
      if(auto it = response.find("error")) {
        if(callback) {
          callback({
            {"error", it.c_str()},
            });
        }
      }
      else if(auto it = response.find("response")) {
        if(callback) {
          json js;
          if(js.parse(it.c_str())) {
            callback(js);
          }
          else {
            callback({
              {"error", js.error()},
              });
          }
        }
      }
      else {
        if(callback) {
          callback({
            {"error", "Response not found."},
            });
        }
      }
    });
  });
}

void ogalib::ResetBattlepassProgress(size_t battlepassId, std::function<void(const json&)> callback) {
  ogalibRequireInit;

  if(battlepassId == 0) {
    if(callback) {
      callback({
        {"error", "Invalid battlepass id."},
        });
    }

    return;
  }

  WaitForLogin([=]() {
    std::string params = string_printf("?playerAccountId=%zu&token=%zu&battlepassId=%zu", ogalibData.userId, ogalibData.token, battlepassId);
    if(ogalibData.encodeURLRequests)
      params = EncodeURL(params.c_str());

    json sendURLParams;
    sendURLParams["usesAPIKey"] = true;
    sendURLParams["ignoreSSLErrors"] = true;

    SendURL(string_printf("%s/ResetBattlepassProgress/v1/%s", ogalibData.baseAPI.c_str(), params.c_str()).c_str(), sendURLParams, [=](const json& response) {
      if(auto it = response.find("error")) {
        if(callback) {
          callback({
            {"error", it.c_str()},
            });
        }
      }
      else if(auto it = response.find("response")) {
        if(callback) {
          json js;
          if(js.parse(it.c_str())) {
            callback(js);
          }
          else {
            callback({
              {"error", js.error()},
              });
          }
        }
      }
      else {
        if(callback) {
          callback({
            {"error", "Response not found."},
            });
        }
      }
    });
  });
}

std::string ogalib::string_printf(const char* f, ...) {
  std::string result;

  va_list ap;
  va_start(ap, f);
  char buffer[StringBufferSizeStack + 1];
  int writeLen = vsnprintf(buffer, sizeof(buffer), f, ap);
  if(writeLen >= StringBufferSizeStack) {
    char* buffer = (char*) malloc(StringBufferSize + 1);
    if(buffer) {
      buffer[0] = 0;
      buffer[StringBufferSize] = 0;
      writeLen = vsnprintf(buffer, StringBufferSize, f, ap);
      buffer[writeLen < StringBufferSize ? writeLen : StringBufferSize] = 0;
      result = buffer;
      free(buffer);
    }
  }
  else {
    buffer[sizeof(buffer) - 1] = 0;
    result = buffer;
  }
  va_end(ap);

  return result;
}

std::string ogalib::string_vprintf(const char* f, va_list ap, va_list ap2) {
  char stackBuffer[StringBufferSizeStack + 1];
  int writeLen = vsnprintf(stackBuffer, StringBufferSizeStack, f, ap2);
  if(writeLen >= StringBufferSizeStack) {
    char* buffer = (char*) malloc(StringBufferSize + 1);
    if(buffer) {
      buffer[0] = 0;
      buffer[StringBufferSize] = 0;
      writeLen = vsnprintf(buffer, StringBufferSize, f, ap2);
      buffer[writeLen < StringBufferSize ? writeLen : StringBufferSize] = 0;
      std::string result = buffer;
      free(buffer);
      return result;
    }
    else {
      return std::string(f);
    }
  }
  else {
    return std::string(stackBuffer);
  }
}

// From: https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
std::string ogalib::EncodeURL(const char* f, ...) {
  va_list ap;
  va_start(ap, f);
  char buffer[8 * 1024 + 1];
  vsnprintf(buffer, sizeof(buffer), f, ap);
  buffer[sizeof(buffer) - 1] = 0;
  va_end(ap);

  std::ostringstream result;
  result.fill('0');
  result << std::hex;

  std::string b = buffer;
  for(std::string::const_iterator i = b.begin(), n = b.end(); i != n; ++i) {
    std::string::value_type c = (*i);

    if(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      result << c;
      continue;
    }

    result << std::uppercase;
    result << '%' << std::setw(2) << int((unsigned char) c);;
    result << std::nouppercase;
  }

  return result.str();
}

static __inline char FromHex(char c) {
  return isdigit(c) ? c - '0' : tolower(c) - 'a' + 10;
}

std::string ogalib::DecodeURL(const char* f) {
  std::string b(f ? f : "");
  char h;
  std::ostringstream escaped;
  escaped.fill('0');

  for (auto i = b.begin(), n = b.end(); i != n; ++i) {
    std::string::value_type c = (*i);

    if (c == '%') {
      if (i[1] && i[2]) {
        h = FromHex(i[1]) << 4 | FromHex(i[2]);
        escaped << h;
        i += 2;
      }
    } else if (c == '+') {
      escaped << ' ';
    } else {
      escaped << c;
    }
  }

  return escaped.str();
}

void ogalib::AssertCore(const char* file, uint32_t line, const char* f, ...) {
  va_list ap, ap2;
  volatile int wait = 1;
  std::string buffer;

  buffer.append("A failed assertion has occurred.\n");
  buffer.append(string_printf("File: %s\n", file));
  buffer.append(string_printf("Line: %d\n\n", line));

  va_start(ap, f);
  va_start(ap2, f);
  buffer.append(string_vprintf(f, ap, ap2));
  ogalib_dbgprintf("%s", buffer.c_str());
  ogalib_dbgprintf("\n");

#if defined(_WIN32) || defined(_WIN64)
  MessageBoxA(NULL, buffer.c_str(), "Assertion", MB_OK);
#endif

#if defined(_DEBUG)
  while(wait) {
    volatile int d;
    d = 1;
    (void) d;
  }
#else
  exit(0);
#endif

  va_end(ap2);
  va_end(ap);
}
