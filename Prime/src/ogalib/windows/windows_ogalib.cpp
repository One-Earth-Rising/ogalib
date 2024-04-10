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

#include <Windows.h>
#include <Shellapi.h>
#include <winhttp.h>
#include <ogalib/ogalib.h>

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

extern ogalib::Data ogalibData;

std::unordered_map<std::string, ogalib::json> urlResponseDataCache;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

bool ogalib::SendURL(const std::string& url, const json& params, json& result) {
  if(!ogalibData.initialized) {
    ogalibAssert(false, "ogalib is not initialized.");
    return false;
  }

  if(url.size() == 0)
    return false;

  ogalibData.assetCacheMutex->Lock();

  auto itCached = urlResponseDataCache.find(url);
  if(itCached != urlResponseDataCache.end()) {
    result = itCached->second;
    ogalibData.assetCacheMutex->Unlock();
    return true;
  }

  ogalibData.assetCacheMutex->Unlock();

  DWORD dwSize = 0;
  DWORD dwDownloaded = 0;
  LPSTR pszOutBuffer;
  BOOL bResults = FALSE;
  HINTERNET hSession = NULL;
  HINTERNET hConnect = NULL;
  HINTERNET hRequest = NULL;
  std::string responseData;
  int statusCode = 0;
  bool secure;

  bool ignoreSSLErrors;
  if(auto it = params.find("ignoreSSLErrors")) {
    auto& value = it.value();
    if(value.IsBool()) {
      ignoreSSLErrors = value.GetBool();
    }
    else {
      ignoreSSLErrors = false;
    }
  }
  else {
    ignoreSSLErrors = false;
  }

  if(auto it = result.find("error")) {
    result.erase(it);
  }
  result["status"] = 0;
  result["statusText"] = "";

  int32_t port;  
  if(auto it = params.find("port")) {
    auto& value = it.value();
    if(value.IsNumber()) {
      port = value.GetUint();
    }
    else {
      port = url.rfind("https", 0) == 0 ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    }
  }
  else {
    port = url.rfind("https", 0) == 0 ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
  }

  std::string server;
  std::string urlPath;

  static const std::string httpsPrefix("https://");
  static const std::string httpPrefix("http://");

  if(url.rfind(httpsPrefix, 0) == 0) {
    std::string serverStart = url.substr(httpsPrefix.size(), url.npos);
    size_t index = serverStart.find_first_of("/");
    if(index == serverStart.npos) {
      server = serverStart;
    }
    else {
      server = serverStart.substr(0, index);
      urlPath = serverStart.substr(index + 1, serverStart.npos);
    }
    secure = true;
  }
  else if(url.rfind(httpPrefix, 0) == 0) {
    std::string serverStart = url.substr(httpPrefix.size(), url.npos);
    size_t index = serverStart.find_first_of("/");
    if(index == serverStart.npos) {
      server = serverStart;
    }
    else {
      server = serverStart.substr(0, index);
      urlPath = serverStart.substr(index + 1, serverStart.npos);
    }
    secure = false;
  }

  if(server.size() == 0) {
    result["error"] = string_printf("Unhandled URL format: %s", url.c_str());
    return false;
  }

  hSession = WinHttpOpen(L"", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );

  if(hSession) {
    hConnect = WinHttpConnect(hSession, std::wstring(server.begin(), server.end()).c_str(), port, 0);
  }

  std::string method;
  if(auto it = params.find("method")) {
    method = it.GetString();
  }
  else {
    method = "GET";
  }

  if(hConnect) {
    hRequest = WinHttpOpenRequest(hConnect, std::wstring(method.begin(), method.end()).c_str(), std::wstring(urlPath.begin(), urlPath.end()).c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0);
  }

  if(hRequest) {
    if(auto it = params.find("data")) {
      if(method == "POST") {
        size_t dataSize;
        const void* data = it.GetStringData(&dataSize);
        if(data && dataSize > 0) {
          if(auto it2 = params.find("contentType")) {
            std::string contentType = it2.GetString();
            std::string useContentType = string_printf("Content-Type: %s", contentType.c_str());
            bResults = WinHttpSendRequest(hRequest, std::wstring(useContentType.begin(), useContentType.end()).c_str(), -1, const_cast<void*>(data), (DWORD) dataSize, (DWORD) dataSize, NULL);
          }
          else {
            bResults = WinHttpSendRequest(hRequest, L"Content-Type: application/x-www-form-urlencoded", -1, const_cast<void*>(data), (DWORD) dataSize, (DWORD) dataSize, NULL);
          }
        }
        else {
          bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
        }
      }
    }
    else {
      bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    }

    if(!bResults && secure && ignoreSSLErrors) {
      if(GetLastError() == ERROR_WINHTTP_SECURE_FAILURE) {
        DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;

        if(WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags))) {
          if(auto it = params.find("data")) {
            size_t dataSize;
            const void* data = it.GetStringData(&dataSize);
            if(data && dataSize > 0) {
              if(auto it2 = params.find("contentType")) {
                const auto& contentType = it2.GetString();
                std::string useContentType = string_printf("Content-Type: %s", contentType.c_str());
                bResults = WinHttpSendRequest(hRequest, std::wstring(useContentType.begin(), useContentType.end()).c_str(), -1, const_cast<void*>(data), (DWORD) dataSize, (DWORD) dataSize, NULL);
              }
              else {
                bResults = WinHttpSendRequest(hRequest, L"Content-Type: application/x-www-form-urlencoded", -1, const_cast<void*>(data), (DWORD) dataSize, (DWORD) dataSize, NULL);
              }
            }
            else {
              bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
            }
          }
          else {
            bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
          }
        }
      }
    }
  }

  bool skipResponseData;
  if(auto it = params.find("skipResponseData")) {
    auto& value = it.value();
    if(value.IsBool()) {
      skipResponseData = value.GetBool();;
    }
    else {
      skipResponseData = false;
    }
  }
  else {
    skipResponseData = false;
  }

  if(!skipResponseData) {
    if(bResults)
      bResults = WinHttpReceiveResponse(hRequest, NULL);

    if(bResults) {
      DWORD dwStatusCode = 0;
      DWORD dwSize = sizeof(dwStatusCode);
      const DWORD statusTextBufferSize = 1024;
      DWORD statusTextSize = statusTextBufferSize;
      wchar_t statusText[statusTextBufferSize + 1];
      statusText[0] = 0;
      statusText[statusTextBufferSize] = 0;

      if(WinHttpQueryHeaders(hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX)) {
        result["status"] = (uint32_t) dwStatusCode;
        statusCode = dwStatusCode;
      }

      if(WinHttpQueryHeaders(hRequest,
        WINHTTP_QUERY_STATUS_TEXT,
        WINHTTP_HEADER_NAME_BY_INDEX,
        statusText, &statusTextSize, WINHTTP_NO_HEADER_INDEX)) {
        char statusTextUTF8[statusTextBufferSize + 1];
        if(WideCharToMultiByte(CP_UTF8, 0, statusText, -1, statusTextUTF8, statusTextBufferSize, NULL, NULL)) {
          result["statusText"] = statusTextUTF8;
        }
      }

      do {
        dwSize = 0;
        if(!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
          result["error"] = string_printf("Error %u in WinHttpQueryDataAvailable.", GetLastError());
        }

        pszOutBuffer = new char[dwSize + 1];
        if(!pszOutBuffer) {
          dwSize = 0;
          result["error"] = string_printf("Out of memory in SendURL.");
        }
        else {
          ZeroMemory(pszOutBuffer, dwSize + 1);

          if(!WinHttpReadData(hRequest, (LPVOID) pszOutBuffer, dwSize, &dwDownloaded)) {
            result["error"] = string_printf("Error %u in WinHttpReadData.", GetLastError());
            break;
          }
          else {
            responseData.append(pszOutBuffer, dwDownloaded);
          }

          delete[] pszOutBuffer;
        }
      }
      while(dwSize > 0);
    }

    if(!bResults) {
      result["error"] = string_printf("Error %d has occurred in SendURL.", GetLastError());
    }
  }

  if(hRequest)
    WinHttpCloseHandle(hRequest);

  if(hConnect)
    WinHttpCloseHandle(hConnect);

  if(hSession)
    WinHttpCloseHandle(hSession);

  bool resultValue = true;

  if(auto itError = result.find("error")) {
    resultValue = false;
  }
  else if(statusCode == 200) {
    result["data"] = responseData;
  }
  else {
    result["error"] = string_printf("HTTP status code: %d", statusCode);
    resultValue = false;
  }

  if(resultValue) {
    ogalibData.assetCacheMutex->Lock();
    urlResponseDataCache[url] = result;
    ogalibData.assetCacheMutex->Unlock();
  }

  return resultValue;
}

#endif
