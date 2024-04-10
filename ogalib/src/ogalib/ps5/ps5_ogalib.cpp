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
#include <ogalib/ps5/ps5_ogalib.h>
#include <libsysmodule.h>
#include <libhttp.h>
#include <libssl.h>
#include <libhttp2.h>
#include <net.h>

using namespace ogalib;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define OGALIB_PS5_PSN_CLIENT_ID "65c86568-da8d-4ad9-bca0-a6eff270f945"

#define OGALIB_PS5_URL_SSL_HEAP_SIZE          (304 * 1024)
#define OGALIB_PS5_URL_HTTP_HEAP_SIZE         (256 * 1024)
#define OGALIB_PS5_URL_NET_HEAP_SIZE          (16 * 1024)
#define OGALIB_PS5_URL_RESOLVE_TIMEOUT        30
#define OGALIB_PS5_URL_CONNECT_TIMEOUT        30
#define OGALIB_PS5_URL_REQUEST_TIMEOUT        30
#define OGALIB_PS5_URL_RECEIVE_TIMEOUT        30
#define OGALIB_PS5_URL_HTTP_USER_AGENT        ""
#define OGALIB_PS5_URL_STACK_RECV_BUFFER_SIZE (8 * 1024)
#define OGALIB_PS5_URL_RECV_BUFFER_SIZE       (256 * 1024)

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

extern ogalib::Data ogalibData;
ogalib::DataPS5 ogalibDataPS5;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

DataPS5::DataPS5():
initialUserId(SCE_USER_SERVICE_USER_ID_INVALID),
npStateCallbackId(-1),
netPoolId(-1),
sslContextId(-1),
httpContextId(-1),
http2ContextId(-1) {

}

void ogalib::InitPS5() {
  int err;

  err = sceSysmoduleLoadModule(SCE_SYSMODULE_NP_AUTH);
  ogalibAssert(err >= SCE_OK, "Error in call to sceSysmoduleLoadModule(SCE_SYSMODULE_NP_AUTH): 0x%08X", err);

  const int HTTP2_MAX_CONCURRENT_REQUEST = 128;
  const int HTTP2_HEAP_SIZE_REQUIRED = (((HTTP2_MAX_CONCURRENT_REQUEST - 1) / 3) + 1) * 256 * 1024;
  const int NET_HEAP_SIZE = (16 * 1024);
  const int SSL_HEAP_SIZE = HTTP2_HEAP_SIZE_REQUIRED;
  const int HTTP2_HEAP_SIZE = HTTP2_HEAP_SIZE_REQUIRED;

  err = sceNetPoolCreate("simple", NET_HEAP_SIZE, 0);
  ogalibAssert(err >= SCE_OK, "Error in call to sceNetPoolCreate: 0x%08X", err);
  ogalibDataPS5.netPoolId = err;

  err = sceSslInit(SSL_HEAP_SIZE);
  ogalibAssert(err >= SCE_OK, "Error in call to sceSslInit: 0x%08X", err);
  ogalibDataPS5.sslContextId = err;

  err = sceHttpInit(ogalibDataPS5.netPoolId, ogalibDataPS5.sslContextId, OGALIB_PS5_URL_HTTP_HEAP_SIZE);
  ogalibAssert(err >= SCE_OK, "Error in call to sceHttpInit: 0x%08X", err);
  ogalibDataPS5.httpContextId = err;

  err = sceHttp2Init(ogalibDataPS5.netPoolId, ogalibDataPS5.sslContextId, HTTP2_HEAP_SIZE, 16);
  ogalibAssert(err >= SCE_OK, "Error in call to sceHttp2Init: 0x%08X", err);
  ogalibDataPS5.http2ContextId = err;
}

void ogalib::FinalizePS5() {
  int err;

  if(ogalibDataPS5.http2ContextId >= 0) {
    err = sceHttp2Term(ogalibDataPS5.http2ContextId);
    ogalibAssert(err >= SCE_OK, "Error in call to sceHttp2Term: 0x%08X", err);
    ogalibDataPS5.http2ContextId = -1;
  }

  if(ogalibDataPS5.httpContextId >= 0) {
    err = sceHttpTerm(ogalibDataPS5.httpContextId);
    ogalibAssert(err >= SCE_OK, "Error in call to sceHttpTerm: 0x%08X", err);
    ogalibDataPS5.httpContextId = -1;
  }

  if(ogalibDataPS5.sslContextId >= 0) {
    err = sceSslTerm(ogalibDataPS5.sslContextId);
    ogalibAssert(err >= SCE_OK, "Error in call to sceSslTerm: 0x%08X", err);
    ogalibDataPS5.sslContextId = -1;
  }

  if(ogalibDataPS5.netPoolId >= 0) {
    err = sceNetPoolDestroy(ogalibDataPS5.netPoolId);
    ogalibAssert(err >= SCE_OK, "Error in call to sceNetPoolDestroy: 0x%08X", err);
    ogalibDataPS5.netPoolId = -1;
  }

  err = sceSysmoduleUnloadModule(SCE_SYSMODULE_NP_AUTH);
  ogalibAssert(err >= SCE_OK, "Error in call to sceSysmoduleUnloadModule(SCE_SYSMODULE_NP_AUTH): 0x%08X", err);
}

void ogalib::LoginUsingPS5(std::function<void(const json&)> callback) {
  if(ogalibDataPS5.initialUserId == SCE_USER_SERVICE_USER_ID_INVALID) {
    int err;

    err = sceUserServiceGetInitialUser(&ogalibDataPS5.initialUserId);

    if(err == SCE_USER_SERVICE_ERROR_NOT_INITIALIZED) {
      err = sceUserServiceInitialize(NULL);
      ogalibAssert(err >= SCE_OK, "Error in call to sceUserServiceInitialize: 0x%08X", err);

      err = sceUserServiceGetInitialUser(&ogalibDataPS5.initialUserId);
      ogalibAssert(err >= SCE_OK, "Error in call to sceUserServiceGetInitialUser: 0x%08X", err);

      err = sceUserServiceTerminate();
      ogalibAssert(err >= SCE_OK, "Error in call to sceUserServiceTerminate: 0x%08X", err);
    }
  }

  if(ogalibDataPS5.initialUserId == SCE_USER_SERVICE_USER_ID_INVALID) {
    if(callback) {
      callback({
        {"error", "Unknown initial system user."},
      });
    }

    return;
  }

  new Job([=](Job& job) {
    bool success = false;

    int err;
    int reqId = 0;
    SceNpClientId clientId;
    SceNpAuthGetAuthorizationCodeParameterV3 authParam;

    SceNpAuthorizationCode psnAuthorizationCodeData;
    int psnAuthorizationCodeIssuerId;

    memset(&psnAuthorizationCodeData, 0, sizeof(psnAuthorizationCodeData));
    psnAuthorizationCodeIssuerId = 0;

    memset(&clientId, 0, sizeof(clientId));
    memset(&authParam, 0, sizeof(authParam));

    SceNpAccountId psnAccountId;

    do {
      err = sceNpGetAccountIdA(ogalibDataPS5.initialUserId, &psnAccountId);
      if(err < SCE_OK) {
        ogalib_dbgprintf("Error in call to sceNpGetAccountIdA: 0x%08X\n", err);
        break;
      }

      err = sceNpAuthCreateRequest();
      if(err < SCE_OK) {
        ogalib_dbgprintf("Error in call to sceNpAuthCreateRequest: 0x%08X\n", err);
        break;
      }
      reqId = err;

      strncpy(clientId.id, OGALIB_PS5_PSN_CLIENT_ID, SCE_NP_CLIENT_ID_MAX_LEN);
      authParam.size = sizeof(authParam);
      authParam.userId = ogalibDataPS5.initialUserId;
      authParam.clientId = &clientId;
      authParam.scope = "psn:s2s";

      err = sceNpAuthGetAuthorizationCodeV3(reqId, &authParam, &psnAuthorizationCodeData, &psnAuthorizationCodeIssuerId);
      if(err < SCE_OK) {
        ogalib_dbgprintf("Error in call to sceNpAuthGetAuthorizationCode: 0x%08X\n", err);

        if(err == SCE_NP_ERROR_LATEST_PATCH_PKG_EXIST) {

        }

        break;
      }

      success = true;
    } while(0);

    if(reqId > 0) {
      err = sceNpAuthDeleteRequest(reqId);
      if(err < SCE_OK) {
        ogalib_dbgprintf("Error in call to sceNpAuthDeleteRequest: 0x%08X\n", err);
      }
    }

    job.data["success"] = success;
    job.data["accountId"] = string_printf("%llu", psnAccountId);
    job.data["authorizationCode"] = std::string(psnAuthorizationCodeData.code);
    job.data["issuerId"] = psnAuthorizationCodeIssuerId;
  }, [=](Job& job) {
    bool success = job.data["success"];
    if(success) {
      json param;

      std::string accountId = job.data["accountId"].GetString();
      std::string authorizationCode = job.data["authorizationCode"].GetString();
      int64_t issuerId = job.data["issuerId"].GetInt64();

      std::string params = string_printf("?network=psn&psnAccountId=%s&psnAuthorizationCode=%s&psnAuthorizationCodeIssuerId=%d", EncodeURL(accountId.c_str()).c_str(), EncodeURL(authorizationCode.c_str()).c_str(), issuerId);
      if(ogalibData.encodeURLRequests)
        params = EncodeURL(params.c_str());

      json sendURLParams;
      sendURLParams["ignoreSSLErrors"] = true;

      std::string url = string_printf("%s/Login/v1/%s", ogalibData.baseAPI.c_str(), params.c_str()).c_str();
      SendURL(url.c_str(), sendURLParams, [=](const json& response) {
        ogalibData.loginInProgress = false;

        if(auto it = response.find("error")) {
          if(callback) {
            callback({
              {"error", it.c_str()},
            });
          }
        }
        else if(auto it = response.find("response")) {
          json loginResponse;
          if(loginResponse.parse(it.GetString())) {
            if(auto itError = loginResponse.find("error")) {
              if(callback) {
                callback({
                  {"error", itError.c_str()},
                });
              }
            }
            else if(auto itResp = loginResponse.find("resp")) {
              auto resp = itResp.GetString();
              if(resp == "ok") {
                if(auto itId = loginResponse.find("id")) {
                  auto& id = itId.value();
                  if(id.IsNumber()) {
                    ogalibData.userId = itId.value().GetUint64();
                  }
                  else {
                    ogalibData.userId = 0;
                  }
                }
                else {
                  ogalibData.userId = 0;
                }

                if(auto itToken = loginResponse.find("token")) {
                  auto& token = itToken.value();
                  if(token.IsNumber()) {
                    ogalibData.token = token.GetUint64();
                  }
                  else {
                    ogalibData.token = 0;
                  }
                }
                else {
                  ogalibData.token = 0;
                }
              }

              if(ogalibData.userId && ogalibData.token) {
                if(callback) {
                  json result;
                  result["success"] = true;
                  callback(result);
                }
              }
              else {
                if(callback) {
                  callback({
                    {"error", "Invalid user."},
                  });
                }
              }
            }
            else {
              if(callback) {
                callback({
                  {"error", "Unknown response."},
                });
              }
            }
          }
          else {
            if(callback) {
              callback({
                {"error", loginResponse.error()},
              });
            }
          }
        }
        else {
          if(callback) {
            callback({
              {"error", "Could not find response."},
            });
          }
        }
      });
    }
    else {
      ogalibData.loginInProgress = false;

      if(callback) {
        callback({
          {"error", "Unable to request PSN authorization."},
        });
      }
    }
  });
}

bool ogalib::SendURL(const std::string& url, const json& params, json& result) {
  if(!ogalibData.initialized) {
    ogalibAssert(false, "ogalib is not initialized.");
    return false;
  }

  if(url.size() == 0)
    return false;

  int templateId = 0;
  int connectionId = 0;
  int requestId = 0;
  int statusCode = 0;
  int err;
  std::string response;

  err = sceHttpCreateTemplate(ogalibDataPS5.httpContextId, OGALIB_PS5_URL_HTTP_USER_AGENT, SCE_HTTP_VERSION_1_1, SCE_TRUE);
  if(err < 0) {
    ogalib_dbgprintf("Error in call to sceHttpCreateTemplate: 0x%08X\n", err);
  }
  else {
    templateId = err;

    err = sceHttpCreateConnectionWithURL(templateId, url.c_str(), SCE_TRUE);
    if(err < 0) {
      ogalib_dbgprintf("Error in call to sceHttpCreateConnectionWithURL: 0x%08X\n", err);
    }
    connectionId = err;

    std::string method;
    if(auto it = params.find("method")) {
      method = it.GetString();
    }
    else {
      method = "GET";
    }

    if(method == "POST") {
      if(auto it = params.find("data")) {
        size_t dataSize;
        const void* data = it.GetStringData(&dataSize);
        err = sceHttpCreateRequestWithURL(connectionId, SCE_HTTP_METHOD_POST, url.c_str(), dataSize);
        if(err < 0) {
          ogalib_dbgprintf("Error in call to sceHttpCreateRequestWithURL: 0x%08X\n", err);
        }
        requestId = err;
      }
      else {
        err = sceHttpCreateRequestWithURL(connectionId, SCE_HTTP_METHOD_POST, url.c_str(), 0);
        if(err < 0) {
          ogalib_dbgprintf("Error in call to sceHttpCreateRequestWithURL: 0x%08X\n", err);
        }
        requestId = err;
      }
    }
    else {
      err = sceHttpCreateRequestWithURL(connectionId, SCE_HTTP_METHOD_GET, url.c_str(), 0);
      if(err < 0) {
        ogalib_dbgprintf("Error in call to sceHttpCreateRequestWithURL: 0x%08X\n", err);
      }
      requestId = err;
    }

    if(auto it = params.find("data")) {
      if(auto it2 = params.find("contentType")) {
        const auto contentType = it2.GetString();
        err = sceHttpAddRequestHeader(requestId, "Content-Type", contentType.c_str(), SCE_HTTP_HEADER_OVERWRITE);
        if(err < 0) {
          ogalib_dbgprintf("Error in call to sceHttpAddRequestHeader: 0x%08X\n", err);
        }
      }
      else {
        err = sceHttpAddRequestHeader(requestId, "Content-Type", "application/x-www-form-urlencoded", SCE_HTTP_HEADER_OVERWRITE);
        if(err < 0) {
          ogalib_dbgprintf("Error in call to sceHttpAddRequestHeader: 0x%08X\n", err);
        }
      }
    }

    if(auto it = params.find("authorizationBearerToken")) {
      std::string authorizationBearerToken = it.GetString();
      if(authorizationBearerToken.length() > 0) {
        err = sceHttpAddRequestHeader(requestId, "Authorization", string_printf("Bearer %s", authorizationBearerToken.c_str()).c_str(), SCE_HTTP_HEADER_OVERWRITE);
        if(err < 0) {
          ogalib_dbgprintf("Error in call to sceHttpAddRequestHeader: 0x%08X\n", err);
        }
      }
    }

    bool ignoreSSLErrors;
    if(auto it = params.find("ignoreSSLErrors")) {
      ignoreSSLErrors = it.GetBool();
    }
    else {
      ignoreSSLErrors = false;
    }

    if(ignoreSSLErrors) {
      err = sceHttpsDisableOption(requestId, SCE_HTTPS_FLAG_SERVER_VERIFY);
      if(err != SCE_OK) {
        ogalib_dbgprintf("Error in call to sceHttpsDisableOption(requestId, SCE_HTTPS_FLAG_SERVER_VERIFY): 0x%08X\n", err);
      }

      err = sceHttpsDisableOption(requestId, SCE_HTTPS_FLAG_CN_CHECK);
      if(err != SCE_OK) {
        ogalib_dbgprintf("Error in call to sceHttpsDisableOption(requestId, SCE_HTTPS_FLAG_CN_CHECK): 0x%08X\n", err);
      }

      err = sceHttpsDisableOption(requestId, SCE_HTTPS_FLAG_NOT_AFTER_CHECK);
      if(err != SCE_OK) {
        ogalib_dbgprintf("Error in call to sceHttpsDisableOption(requestId, SCE_HTTPS_FLAG_NOT_AFTER_CHECK): 0x%08X\n", err);
      }

      err = sceHttpsDisableOption(requestId, SCE_HTTPS_FLAG_NOT_BEFORE_CHECK);
      if(err != SCE_OK) {
        ogalib_dbgprintf("Error in call to sceHttpsDisableOption(requestId, SCE_HTTPS_FLAG_NOT_BEFORE_CHECK): 0x%08X\n", err);
      }

      err = sceHttpsDisableOption(requestId, SCE_HTTPS_FLAG_KNOWN_CA_CHECK);
      if(err != SCE_OK) {
        ogalib_dbgprintf("Error in call to sceHttpsDisableOption(requestId, SCE_HTTPS_FLAG_KNOWN_CA_CHECK): 0x%08X\n", err);
      }
    }
    else {
      err = sceHttpsEnableOption(requestId, SCE_HTTPS_FLAG_SERVER_VERIFY);
      if(err != SCE_OK) {
        ogalib_dbgprintf("Error in call to sceHttpsEnableOption(requestId, SCE_HTTPS_FLAG_SERVER_VERIFY): 0x%08X\n", err);
      }

      err = sceHttpsEnableOption(requestId, SCE_HTTPS_FLAG_CN_CHECK);
      if(err != SCE_OK) {
        ogalib_dbgprintf("Error in call to sceHttpsEnableOption(requestId, SCE_HTTPS_FLAG_CN_CHECK): 0x%08X\n", err);
      }

      err = sceHttpsEnableOption(requestId, SCE_HTTPS_FLAG_NOT_AFTER_CHECK);
      if(err != SCE_OK) {
        ogalib_dbgprintf("Error in call to sceHttpsEnableOption(requestId, SCE_HTTPS_FLAG_NOT_AFTER_CHECK): 0x%08X\n", err);
      }

      err = sceHttpsEnableOption(requestId, SCE_HTTPS_FLAG_NOT_BEFORE_CHECK);
      if(err != SCE_OK) {
        ogalib_dbgprintf("Error in call to sceHttpsEnableOption(requestId, SCE_HTTPS_FLAG_NOT_BEFORE_CHECK): 0x%08X\n", err);
      }

      err = sceHttpsEnableOption(requestId, SCE_HTTPS_FLAG_KNOWN_CA_CHECK);
      if(err != SCE_OK) {
        ogalib_dbgprintf("Error in call to sceHttpsEnableOption(requestId, SCE_HTTPS_FLAG_KNOWN_CA_CHECK): 0x%08X\n", err);
      }
    }

    err = sceHttpSetResolveTimeOut(requestId, OGALIB_PS5_URL_RESOLVE_TIMEOUT * 1000 * 1000);
    if(err < 0) {
      ogalib_dbgprintf("Error in call to sceHttpAddRequestHeader: 0x%08X\n", err);
    }

    err = sceHttpSetConnectTimeOut(requestId, OGALIB_PS5_URL_CONNECT_TIMEOUT * 1000 * 1000);
    if(err < 0) {
      ogalib_dbgprintf("Error in call to sceHttpAddRequestHeader: 0x%08X\n", err);
    }

    err = sceHttpSetSendTimeOut(requestId, OGALIB_PS5_URL_REQUEST_TIMEOUT * 1000 * 1000);
    if(err < 0) {
      ogalib_dbgprintf("Error in call to sceHttpAddRequestHeader: 0x%08X\n", err);
    }

    err = sceHttpSetRecvTimeOut(requestId, OGALIB_PS5_URL_RECEIVE_TIMEOUT * 1000 * 1000);
    if(err < 0) {
      ogalib_dbgprintf("Error in call to sceHttpAddRequestHeader: 0x%08X\n", err);
    }

    if(auto it = params.find("data")) {
      size_t dataSize;
      const void* data = it.GetStringData(&dataSize);
      err = sceHttpSendRequest(requestId, data, dataSize);
      if(err < 0) {
        ogalib_dbgprintf("Error in call to sceHttpSendRequest: 0x%08X\n", err);
      }
    }
    else {
      err = sceHttpSendRequest(requestId, NULL, 0);
      if(err < 0) {
        ogalib_dbgprintf("Error in call to sceHttpSendRequest: 0x%08X\n", err);
      }
    }

    bool skipResponse;
    if(auto it = params.find("skipResponse")) {
      skipResponse = it.GetBool();
    }
    else {
      skipResponse = false;
    }

    if(!skipResponse) {
      err = sceHttpGetStatusCode(requestId, &statusCode);
      if(err < 0) {
        result["error"] = string_printf("Error in call to sceHttpGetStatusCode: 0x%08X\n", err);
      }

      int contentLengthType;
      uint64_t contentLength;
      err = sceHttpGetResponseContentLength(requestId, &contentLengthType, &contentLength);
      if(err < 0) {
        result["error"] = string_printf("Error in call to sceHttpGetContentLength: 0x%08X\n", err);
      }
      else {
        if(contentLengthType == SCE_HTTP_CONTENTLEN_EXIST || contentLengthType == SCE_HTTP_CONTENTLEN_CHUNK_ENC) {
          bool useStack = true;
          if(contentLength >= OGALIB_PS5_URL_RECV_BUFFER_SIZE) {
            char* recvBuff = new char[OGALIB_PS5_URL_RECV_BUFFER_SIZE];
            if(recvBuff) {
              useStack = false;
              bool go = true;
              while(go) {
                go = false;
                err = sceHttpReadData(requestId, recvBuff, OGALIB_PS5_URL_RECV_BUFFER_SIZE);
                if(err < 0) {
                  result["error"] = string_printf("Error in call to sceHttpReadData: 0x%08X\n", err);
                }
                else if(err > 0) {
                  response.append(recvBuff, err);
                  go = true;
                }
                else if(err == SCE_OK) {
                  break;
                }
              }

              delete[] recvBuff;
            }
            else {
              result["error"] = string_printf("Out of memory in SendURL.");
            }
          }
            
          if(useStack) {
            char recvBuff[OGALIB_PS5_URL_STACK_RECV_BUFFER_SIZE];
            bool go = true;
            while(go) {
              go = false;
              err = sceHttpReadData(requestId, recvBuff, sizeof(recvBuff));
              if(err < 0) {
                result["error"] = string_printf("Error in call to sceHttpReadData: 0x%08X\n", err);
              }
              else if(err > 0) {
                response.append(recvBuff, err);
                go = true;
              }
              else if(err == SCE_OK) {
                break;
              }
            }
          }
        }
      }
    }
  }

  if(requestId) {
    err = sceHttpDeleteRequest(requestId);
    if(err < 0) {
      ogalib_dbgprintf("Error in call to sceHttpDeleteRequest: 0x%08X\n", err);
    }
  }

  if(connectionId) {
    err = sceHttpDeleteConnection(connectionId);
    if(err < 0) {
      ogalib_dbgprintf("Error in call to sceHttpDeleteConnection: 0x%08X\n", err);
    }
  }

  if(templateId) {
    err = sceHttpDeleteTemplate(templateId);
    if(err < 0) {
      ogalib_dbgprintf("Error in call to sceHttpDeleteTemplate: 0x%08X\n", err);
    }
  }

  bool resultValue = true;

  if(auto itError = result.find("error")) {
    resultValue = false;
  }
  else if(statusCode == 200) {
    result["response"] = response;
  }
  else {
    result["error"] = string_printf("HTTP status code: %d", statusCode);
    resultValue = false;
  }

  return resultValue;
}

#endif
