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

#include <Prime/Config.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Types/Dictionary.h>
#include <Prime/Content/Content.h>
#include <Prime/System/PrimePackFormat.h>
#include <Prime/Imagemap/ImagemapContent.h>
#include <Prime/Skinset/SkinsetContent.h>
#include <Prime/Skeleton/SkeletonContent.h>
#include <Prime/Model/ModelContent.h>
#include <Prime/Rig/RigContent.h>
#include <Prime/Font/FontContent.h>
#include <png/png.h>
#include <jpeg/jpeglib.h>
#include <jpeg/jerror.h>
#include <jsmn/jsmn.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

struct jpegErrorManager {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

static char jpegLastErrorMsg[JMSG_LENGTH_MAX];
static void jpegErrorExit(j_common_ptr cinfo);

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class ContentPPF: public Content {
private:

  PrimePackFormat* ppf;

public:

  PrimePackFormat* GetPPF() const {return ppf;}

public:

  ContentPPF(PrimePackFormat* ppf);
  ~ContentPPF();

};

ContentPPF::ContentPPF(PrimePackFormat* ppf): ppf(ppf) {

}

ContentPPF::~ContentPPF() {
  if(ppf) {
    delete ppf;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ThreadMutex* setjmpMutex = nullptr;
static ThreadMutex* contentDataMutex = nullptr;
static Dictionary<std::string, refptr<Content>> contentData;
static Dictionary<std::string, bool> contentDataLoadingLocked;
static Dictionary<std::string, size_t> contentDataLoading;
static Dictionary<std::string, refptr<ContentPPF>> contentPPFItems;
static Dictionary<std::string, std::string> contentURIMap;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

namespace Prime {
void InitContent();
void ShutdownContent();
void ProcessContentRefs();
void ReleaseAllContent();

static void GetContentByData(const std::string& uri, const void* data, size_t dataSize, const json& info, const std::function<void (Content*)>& callback);

static bool IncContentDataLoading(const std::string& uri);
static void DecContentDataLoading(const std::string& uri, bool locked);
static void WaitForContentDataLoading(const std::string& uri);
static void OnContentLoadingDone(Content* content, const std::string& uri, bool locked, const std::function<void (Content*)>& callback);
static void SetupLoadingContent(Content* content, const std::string& uri, const json& info);
};

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

void Prime::GetContent(const std::string& uri, const std::function<void (Content*)>& callback) {
  json info;
  GetContent(uri, info, callback);
}

void Prime::GetContent(const std::string& uri, const json& info, const std::function<void (Content*)>& callback) {
  PxRequireMainThread;

  const std::string& mappedURI = GetMapppedContentURI(uri);

  if(mappedURI.empty()) {
    callback(nullptr);
    return;
  }

  if(auto it = contentData.Find(mappedURI)) {
    callback(it.value());
    return;
  }

  for(auto it: contentPPFItems) {
    auto ppf = it.value()->GetPPF();
    const std::string& ppfContentPath = ppf->GetContentPath();

    if(StartsWith(uri, ppfContentPath)) {
      std::string subPath = uri.substr(ppfContentPath.length());
      if(ppf->HasItem(subPath)) {
        BlockBuffer* blockBuffer = ppf->GetItemData(subPath);
        if(blockBuffer) {
          size_t dataSize;
          void* data = blockBuffer->ConvertToBytes(&dataSize);
          delete blockBuffer;
          if(data) {
            GetContentByData(uri, data, dataSize, info, callback);
            free(data);
            return;
          }
        }
      }
    }

    if(auto it = info.find("_parentURI")) {
      std::string parentURI = it.GetString();
      if(!parentURI.empty()) {
        if(StartsWith(parentURI, ppfContentPath)) {
          if(ppf->HasItem(uri)) {
            BlockBuffer* blockBuffer = ppf->GetItemData(uri);
            if(blockBuffer) {
              size_t dataSize;
              void* data = blockBuffer->ConvertToBytes(&dataSize);
              delete blockBuffer;
              if(data) {
                std::string useURI = ppfContentPath + uri;
                GetContentByData(useURI, data, dataSize, info, callback);
                free(data);
                return;
              }
            }
          }
        }
      }
    }
  }

  std::string lowerURI = ToLower(mappedURI);

  if(StartsWith(lowerURI, "http")) {
    SendURL(mappedURI, [=](const json& response) {
      if(auto it = response.find("data")) {
        size_t dataSize;
        const void* data = it.GetStringData(&dataSize);
        GetContentByData(mappedURI, data, dataSize, info, callback);
      }
      else {
        callback(nullptr);
      }
    });
  }
  else {
    ReadFile(mappedURI, [=](void* data, size_t dataSize) {
      GetContentByData(mappedURI, data, dataSize, info, callback);
      if(data) {
        free(data);
      }
    });
  }
}

void Prime::GetContentRaw(const std::string& uri, const std::function<void (const void*, size_t)>& callback) {
  json info;
  GetContentRaw(uri, info, callback);
}

void Prime::GetContentRaw(const std::string& uri, const json& info, const std::function<void (const void*, size_t)>& callback) {
  PxRequireMainThread;

  const std::string& mappedURI = GetMapppedContentURI(uri);

  if(mappedURI.empty()) {
    callback(nullptr, 0);
    return;
  }

  for(auto it: contentPPFItems) {
    auto ppf = it.value()->GetPPF();
    const std::string& ppfContentPath = ppf->GetContentPath();

    if(StartsWith(uri, ppfContentPath)) {
      std::string subPath = uri.substr(ppfContentPath.length());
      if(ppf->HasItem(subPath)) {
        BlockBuffer* blockBuffer = ppf->GetItemData(subPath);
        if(blockBuffer) {
          size_t dataSize;
          void* data = blockBuffer->ConvertToBytes(&dataSize);
          delete blockBuffer;
          if(data) {
            callback(data, dataSize);
            free(data);
            return;
          }
        }
      }
    }

    if(auto it = info.find("_parentURI")) {
      std::string parentURI = it.GetString();
      if(!parentURI.empty()) {
        if(StartsWith(parentURI, ppfContentPath)) {
          if(ppf->HasItem(uri)) {
            BlockBuffer* blockBuffer = ppf->GetItemData(uri);
            if(blockBuffer) {
              size_t dataSize;
              void* data = blockBuffer->ConvertToBytes(&dataSize);
              delete blockBuffer;
              if(data) {
                callback(data, dataSize);
                free(data);
                return;
              }
            }
          }
        }
      }
    }
  }

  std::string lowerURI = ToLower(mappedURI);

  if(StartsWith(lowerURI, "http")) {
    SendURL(mappedURI, [=](const json& response) {
      if(auto it = response.find("data")) {
        size_t dataSize;
        const void* data = it.GetStringData(&dataSize);
        callback(data, dataSize);
      }
      else {
        callback(nullptr, 0);
      }
    });
  }
  else {
    ReadFile(mappedURI, [=](void* data, size_t dataSize) {
      callback(data, dataSize);
      if(data) {
        free(data);
      }
    });
  }
}

void Prime::MapContentURI(const std::string& mappedURI, const std::string& uri) {
  PxRequireMainThread;

  contentURIMap[mappedURI] = uri;
}

const std::string& Prime::GetMapppedContentURI(const std::string& uri) {
  PxRequireMainThread;

  if(auto it = contentURIMap.Find(uri)) {
    return GetMapppedContentURI(it.value());
  }
  else {
    return uri;
  }
}

void Prime::GetPackFilenames(const std::string& uri, Stack<std::string>& filenames) {
  if(auto it = contentPPFItems.Find(uri)) {
    auto ppf = it.value()->GetPPF();

    std::vector<std::string> filenamesVector;
    ppf->GetItemPaths(filenamesVector);

    for(auto& filename: filenamesVector) {
      filenames.Add(filename);
    }
  }
}

bool Prime::LockSetjmpMutex() {
  return setjmpMutex->Lock();
}

bool Prime::UnlockSetjmpMutex() {
  return setjmpMutex->Unlock();
}

void Prime::InitContent() {
  contentDataMutex = new ThreadMutex("Content Data");
  setjmpMutex = new ThreadMutex("setjmp", true);
}

void Prime::ShutdownContent() {
  PrimeSafeDelete(setjmpMutex);
  PrimeSafeDelete(contentDataMutex);
}

void Prime::ProcessContentRefs() {
  Stack<std::string> removeURIs;

  for(auto it: contentData) {
    const std::string& uri = it.key();

    Content* content = it.value();
    if(content->GetRefCount() == 1) {
      bool remove = true;

      contentDataMutex->Lock();
      if(auto itLoadingLocked = contentDataLoadingLocked.Find(uri)) {
        remove = false;
      }
      if(remove) {
        if(auto itLoading = contentDataLoading.Find(uri)) {
          remove = false;
        }
      }
      contentDataMutex->Unlock();

      if(remove) {
        removeURIs.Push(it.key());
      }
    }
  }

  for(const auto& uri: removeURIs) {
    contentData.Remove(uri);
  }
}

void Prime::ReleaseAllContent() {
  ProcessContentRefs();
  contentPPFItems.Clear();
  contentData.Clear();
}

void Prime::GetContentByData(const std::string& uri, const void* data, size_t dataSize, const json& info, const std::function<void (Content*)>& callback) {
  if(data == nullptr || dataSize == 0) {
    callback(nullptr);
    return;
  }

  if(IsFormatBC(data, dataSize, info)) {
    bool locked = IncContentDataLoading(uri);
    refptr<ImagemapContent> content;
    if(locked) {
      content = new ImagemapContent();
    }

    std::string dataCopy((const char*) data, dataSize);
    new Job([=](Job& job) {
      if(locked) {
        if(content) {
          SetupLoadingContent(content, uri, info);
          content->Load(dataCopy.c_str(), dataCopy.size(), info);
        }
      }
      else {
        WaitForContentDataLoading(uri);
      }
    }, [=](Job& job) {
      OnContentLoadingDone(content, uri, locked, callback);
    });

    return;
  }

  static const std::string _classNameStr("_className");
  static const std::string nodesStr("nodes");  
  std::string className;

  if(IsFormatJSONWithValue(data, dataSize, info, _classNameStr, className)) {
    json obj;
    if(IsFormatJSON(data, dataSize, info, obj)) {
      bool locked = IncContentDataLoading(uri);

      refptr<Content> content;

      if(locked) {
        if(className == "Imagemap") {
          content = new ImagemapContent();
        }
        else if(className == "Skinset") {
          content = new SkinsetContent();
        }
        else if(className == "Skeleton") {
          content = new SkeletonContent();
        }
        else if(className == "Model") {
          content = new ModelContent();
        }
        else if(className == "Rig") {
          content = new RigContent();
        }
        else {
          if(IsFormatJSONWithArray(data, dataSize, info, nodesStr)) {
            content = new RigContent();
          }
        }

#if defined(_DEBUG)
        if(!content) {
          dbgprintf("[Warning] Unknown content class: %s\n", className.c_str());
        }
#endif
      }

      new Job([=](Job& job) {
        if(locked) {
          if(content) {
            SetupLoadingContent(content, uri, info);
            content->Load(obj, info);
          }
        }
        else {
          WaitForContentDataLoading(uri);
        }
      }, [=](Job& job) {
        OnContentLoadingDone(content, uri, locked, callback);
      });
    }
    else {
      callback(nullptr);
    }
  }
  else if(IsFormatPNG(data, dataSize, info)) {
    bool locked = IncContentDataLoading(uri);
    refptr<ImagemapContent> content;
    if(locked) {
      content = new ImagemapContent();
    }

    std::string dataCopy((const char*) data, dataSize);
    new Job([=](Job& job) {
      if(locked) {
        if(content) {
          SetupLoadingContent(content, uri, info);
          content->Load(dataCopy.c_str(), dataCopy.size(), info);

          PrimePackFormat* ppf = new PrimePackFormat();
          if(ppf) {
            ppf->InitFromData(dataCopy.c_str(), dataCopy.size());
            if(ppf->GetError() == PrimePackFormatErrorNone) {
              if(ppf->GetItemCount() > 0) {
                ppf->SetContentPath(uri);
                job.data["ppf"] = ppf;
              }
            }
            else {
              delete ppf;
            }
          }
        }
      }
      else {
        WaitForContentDataLoading(uri);
      }
    }, [=](Job& job) {
      if(auto it = job.data.find("ppf")) {
        PrimePackFormat* ppf = it.GetPtr<PrimePackFormat>();
        contentPPFItems[uri] = new ContentPPF(ppf);
      }
      OnContentLoadingDone(content, uri, locked, callback);
    });
  }
  else if(IsFormatGLTF(data, dataSize, info) || IsFormatFBX(data, dataSize, info)) {
    bool locked = IncContentDataLoading(uri);
    refptr<ModelContent> content;
    if(locked) {
      content = new ModelContent();
    }

    std::string dataCopy((const char*) data, dataSize);
    new Job([=](Job& job) {
      if(locked) {
        if(content) {
          SetupLoadingContent(content, uri, info);
          content->Load(dataCopy.c_str(), dataCopy.size(), info);
        }
      }
      else {
        WaitForContentDataLoading(uri);
      }
    }, [=](Job& job) {
      OnContentLoadingDone(content, uri, locked, callback);
    });
  }
  else if(IsFormatJPEG(data, dataSize, info)) {
    bool locked = IncContentDataLoading(uri);
    refptr<ImagemapContent> content;
    if(locked) {
      content = new ImagemapContent();
    }

    std::string dataCopy((const char*) data, dataSize);
    new Job([=](Job& job) {
      if(locked) {
        if(content) {
          SetupLoadingContent(content, uri, info);
          content->Load(dataCopy.c_str(), dataCopy.size(), info);
        }
      }
      else {
        WaitForContentDataLoading(uri);
      }
    }, [=](Job& job) {
      OnContentLoadingDone(content, uri, locked, callback);
    });
    }
  else if(IsFormatOTF(data, dataSize, info)) {
    bool locked = IncContentDataLoading(uri);
    refptr<FontContent> content;
    if(locked) {
      content = new FontContent();
    }

    std::string dataCopy((const char*) data, dataSize);
    new Job([=](Job& job) {
      if(locked) {
        if(content) {
          SetupLoadingContent(content, uri, info);
          content->Load(dataCopy.c_str(), dataCopy.size(), info);
        }
      }
      else {
        WaitForContentDataLoading(uri);
      }
    }, [=](Job& job) {
      OnContentLoadingDone(content, uri, locked, callback);
    });
  }
  else {
    callback(nullptr);
  }
}

bool Prime::IncContentDataLoading(const std::string& uri) {
  bool locked = false;

  contentDataMutex->Lock();
  if(auto it = contentDataLoading.Find(uri)) {
    auto& value = it.value();
    value++;
  }
  else {
    if(auto it = contentData.Find(uri)) {
      auto& value = it.value();
      contentDataLoading[uri] = 1;
    }
    else {
      locked = true;
      contentDataLoading[uri] = 1;
      contentDataLoadingLocked[uri] = true;
    }
  }
  contentDataMutex->Unlock();

  return locked;
}

void Prime::DecContentDataLoading(const std::string& uri, bool locked) {
#if defined(_DEBUG)
  bool success = false;
#endif

  contentDataMutex->Lock();
  if(locked) {
    PrimeAssert(contentDataLoadingLocked.HasKey(uri), "Expected to find content load-lock: uri = %s", uri.c_str());

    contentDataLoadingLocked.Remove(uri);

    if(auto it = contentData.Find(uri)) {
      refptr<Content> content = it.value();

      PrimeAssert(content, "Expected to find content that was just loaded: uri = %s", uri.c_str());

      size_t loadingCount = 0;
      if(auto itLoadingCount = contentDataLoading.Find(uri)) {
        loadingCount = itLoadingCount;
      }

      PrimeAssert(loadingCount > 0, "Expected loading count to be at least 1 for the first content loaded: uri = %s", uri.c_str());

      if(loadingCount > 0) {
        loadingCount--;
      }

      // Transfer the existing loading count into references.
      for(size_t i = 0; i < loadingCount; i++) {
        content->IncRef();
      }
    }
  }

  if(auto it = contentDataLoading.Find(uri)) {
    auto& value = it.value();
    if(value > 0) {
      value--;
    }
    if(value == 0) {
      contentDataLoading.Remove(uri);
    }
#if defined(_DEBUG)
    success = true;
#endif
  }
  contentDataMutex->Unlock();

#if defined(_DEBUG)
  PrimeAssert(success, "Expected to decrement content data loading value: uri = %s", uri.c_str());
#endif
}

void Prime::WaitForContentDataLoading(const std::string& uri) {
  bool loading = true;
  while(loading) {
    loading = false;
    contentDataMutex->Lock();
    if(auto it = contentDataLoadingLocked.Find(uri)) {
      loading = true;
      Thread::Yield();
    }
    contentDataMutex->Unlock();
  }
}

void Prime::OnContentLoadingDone(Content* content, const std::string& uri, bool locked, const std::function<void (Content*)>& callback) {
  if(locked) {
#if defined(_DEBUG)
    PrimeAssert(!contentData.HasKey(uri), "Content data already exists: uri = %s", uri.c_str());
#endif

    if(content) {
      contentData[uri] = content;
    }
    DecContentDataLoading(uri, locked);
    callback(content);
  }
  else {
    if(auto it = contentData.Find(uri)) {
      refptr<Content> foundContent = it.value();
      DecContentDataLoading(uri, locked);
      callback(foundContent);
    }
    else {
#if defined(_DEBUG)
      //PrimeAssert(false, "Expected to find content data after waiting for loading: uri = %s", uri.c_str());
#endif

      callback(nullptr);
    }
  }
}

void Prime::SetupLoadingContent(Content* content, const std::string& uri, const json& info) {
  content->_uri = uri;
}

bool Prime::IsFormatJSON(const void* data, size_t dataSize, const json& info) {
  json output;
  return IsFormatJSON(data, dataSize, info, output);
}

bool Prime::IsFormatJSON(const void* data, size_t dataSize, const json& info, json& output) {
  if(data == nullptr || dataSize < 2)
    return false;

  bool result;
  char first = '\0';
  char last = '\0';
  const char* dataStr = (const char*) data;

  first = dataStr[0];
  if(first != '{' && first != '[') {
    return false;
  }

  s64 seekChar = 1;
  while(seekChar < (s64) dataSize) {
    last = dataStr[dataSize - seekChar];
    if(!std::isspace(last)) {
      break;
    }
    seekChar++;
  }

  if(last != '}' && last != ']') {
    return false;
  }

  result = false;
  if(output.parse(data, dataSize)) {
    result = true;
  }
  else {
    output["error"] = output.error();
  }

  return result;
}

bool Prime::IsFormatJSONWithValue(const void* data, size_t dataSize, const json& info, const std::string& key, std::string& value) {
  if(data == nullptr || dataSize < 2)
    return false;

  bool result;
  char first = '\0';
  char last = '\0';
  const char* dataStr = (const char*) data;

  first = dataStr[0];
  if(first != '{' && first != '[') {
    return false;
  }

  s64 seekChar = 1;
  while(seekChar < (s64) dataSize) {
    last = dataStr[dataSize - seekChar];
    if(!std::isspace(last)) {
      break;
    }
    seekChar++;
  }

  if(last != '}' && last != ']') {
    return false;
  }

  result = false;
  s32 tokenCount;
  jsmn_parser parser;
  const u32 maxTokenCount = 10;
  jsmntok_t tokens[maxTokenCount];

  const char* keyStr = key.c_str();

  jsmn_init(&parser);
  tokenCount = jsmn_parse(&parser, dataStr, dataSize, tokens, maxTokenCount);

  if(tokenCount == JSMN_ERROR_NOMEM) {
    tokenCount = maxTokenCount;
  }

  bool nextTokenIsValue = false;
  if(tokenCount > 0) {
    for(size_t i = 0; i < tokenCount; i++) {
      jsmntok_t& token = tokens[i];
      if(token.type == JSMN_STRING) {
        if(nextTokenIsValue) {
          value = std::string(dataStr + token.start, token.end - token.start);
          result = true;
          break;
        }
        else {
          if(strncmp(dataStr + token.start, keyStr, token.end - token.start) == 0) {
            nextTokenIsValue = true;
          }
        }
      }
    }
  }

  if(!result) {
    json js;
    if(js.parse(data, dataSize)) {
      if(auto it = js.find(keyStr)) {
        value = it.GetString();
        result = true;
      }
    }
  }

  return result;
}

bool Prime::IsFormatJSONWithArray(const void* data, size_t dataSize, const json& info, const std::string& key) {
  if(data == nullptr || dataSize < 2)
    return false;

  bool result;
  char first = '\0';
  char last = '\0';
  const char* dataStr = (const char*) data;

  first = dataStr[0];
  if(first != '{' && first != '[') {
    return false;
  }

  s64 seekChar = 1;
  while(seekChar < (s64) dataSize) {
    last = dataStr[dataSize - seekChar];
    if(!std::isspace(last)) {
      break;
    }
    seekChar++;
  }

  if(last != '}' && last != ']') {
    return false;
  }

  result = false;
  s32 tokenCount;
  jsmn_parser parser;
  const u32 maxTokenCount = 10;
  jsmntok_t tokens[maxTokenCount];

  const char* keyStr = key.c_str();

  jsmn_init(&parser);
  tokenCount = jsmn_parse(&parser, dataStr, dataSize, tokens, maxTokenCount);

  if(tokenCount == JSMN_ERROR_NOMEM) {
    tokenCount = maxTokenCount;
  }

  bool nextTokenIsArray = false;
  if(tokenCount > 0) {
    for(size_t i = 0; i < tokenCount; i++) {
      jsmntok_t& token = tokens[i];
      if(nextTokenIsArray) {
        result = token.type == JSMN_ARRAY;
        break;
      }
      else if(token.type == JSMN_STRING) {
        if(strncmp(dataStr + token.start, keyStr, token.end - token.start) == 0) {
          nextTokenIsArray = true;            
        }
      }
    }
  }

  return result;
}

bool Prime::IsFormatPNG(const void* data, size_t dataSize, const json& info) {
  if(data == nullptr)
    return false;

  u8 header[8];
  if(dataSize >= sizeof(header)) {
    memcpy(header, data, sizeof(header));
    return png_sig_cmp((png_bytep) header, 0, 8) == 0;
  }

  return false;
}

bool Prime::IsFormatJPEG(const void* data, size_t dataSize, const json& info) {
  if(data == nullptr)
    return false;

  LockSetjmpMutex();

  struct jpeg_decompress_struct jpegInfo;
  jpegErrorManager jerr;

  jpegInfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = jpegErrorExit;

  jpeg_create_decompress(&jpegInfo);

  if(setjmp(jerr.setjmp_buffer)) {
    // If we get here, the JPEG code has signaled an error.
    jpeg_destroy_decompress(&jpegInfo);
    UnlockSetjmpMutex();
    return false;
  }

  jpeg_mem_src(&jpegInfo, (const unsigned char*) data, dataSize);
  int result = jpeg_read_header(&jpegInfo, FALSE);
  jpeg_destroy_decompress(&jpegInfo);

  UnlockSetjmpMutex();

  return result == JPEG_HEADER_OK;
}

bool Prime::IsFormatBC(const void* data, size_t dataSize, const json& info) {
  if(data == nullptr)
    return false;

  if(auto it = info.find("format")) {
    std::string format = it.GetString();
    return format == "bc";
  }

  return false;
}

bool Prime::IsFormatGLTF(const void* data, size_t dataSize, const json& info) {
  if(data == nullptr)
    return false;

  u8 header[12];
  if(dataSize >= sizeof(header)) {
    memcpy(header, data, sizeof(header));

    if(memcmp(header, "glTF", 4) == 0) {
      u32 readVersion = header[4] | (header[5] << 8) | (header[6] << 16) | (header[7] << 24);
      u32 readSize = header[8] | (header[9] << 8) | (header[10] << 16) | (header[11] << 24);

      (void) readVersion;

      return readSize == dataSize;
    }
  }

  return false;
}

bool Prime::IsFormatFBX(const void* data, size_t dataSize, const json& info) {
  if(data == nullptr)
    return false;

  u8 header[27];
  if(dataSize >= sizeof(header)) {
    memcpy(header, data, sizeof(header));

    if(memcmp(header, "Kaydara FBX Binary\x20\x20\x00\x1a\x00", 21) == 0) {
      u32 readVersion = header[23] | (header[24] << 8) | (header[25] << 16) | (header[26] << 24);

      (void) readVersion;

      return true;
    }
  }

  return false;
}

bool Prime::IsFormatOTF(const void* data, size_t dataSize, const json& info) {
  if(data == nullptr)
    return false;

  u8 header[4];
  if(dataSize >= sizeof(header)) {
    memcpy(header, data, sizeof(header));

    if(memcmp(header, "OTTO", 4) == 0) {
      return true;
    }
  }

  return false;
}

static void jpegErrorExit(j_common_ptr cinfo) {
  jpegErrorManager* myerr = (jpegErrorManager*) cinfo->err;
  (*(cinfo->err->format_message)) (cinfo, jpegLastErrorMsg);
  longjmp(myerr->setjmp_buffer, 1);
}
