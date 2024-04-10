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

#include <Prime/Graphics/windows/WindowsShader.h>
#include <Prime/Graphics/windows/WindowsProgram.h>
#include <Prime/Graphics/windows/WindowsIndexBuffer.h>
#include <Prime/Graphics/windows/WindowsArrayBuffer.h>
#include <Prime/Graphics/windows/WindowsTex.h>
#include <Prime/Graphics/Graphics.h>

#include <Windows.h>
#include <utf8/utf8.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

f64 Prime::GetSystemTime() {
#if defined(PrimeTargetOpenGL)
  return glfwGetTime();
#else
  return 0.0;
#endif
}

f64 Prime::GetTargetRTCSeconds() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / 1000000000.0;
}

void* Prime::ReadFile(const std::string& path, size_t* size) {
  void* result = nullptr;
  size_t resultSize = 0;

  FILE* file = fopen(path.c_str(), "rb");
  if(file) {
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    result = malloc(fileSize);
    if(result) {
      u8* d = (u8*) result;
      while(resultSize < fileSize) {
        size_t bytesToRead = fileSize - resultSize;
        size_t bytesRead = fread(d + resultSize, 1, bytesToRead, file);
        if(bytesRead > 0) {
          resultSize += bytesRead;
        }
        else {
          break;
        }
      }
    }

    fclose(file);
  }

  if(size) {
    *size = resultSize;
  }

  return result;
}

void Prime::ReadFile(const std::string& path, const std::function<void (void*, size_t)>& callback) {
  if(path.empty()) {
    callback(nullptr, 0);
    return;
  }

  // Force paths to be read from folder tree below the running exe file.
  CHAR cwd[8 * 1024];
  GetCurrentDirectoryA(sizeof(cwd) - 1, cwd);

  std::string fullPath = cwd;

  if(StartsWith(path, "/") || StartsWith(path, "\\")) {
    fullPath += path.substr(1);
  }
  else {
    fullPath += "/" + path;
  }

  new Job([=](Job& cb) {
    size_t size;
    void* result = ReadFile(fullPath.c_str(), &size);
    cb.data["result"] = result;
    cb.data["size"] = size;
  }, [=](Job& cb) {
    void* result = cb.data["result"].GetVoidPtr();
    size_t size = cb.data["size"].GetSizeT();
    callback(result, size);
  });
}

////////////////////////////////////////////////////////////////////////////////
// Create Functions (Shader)
////////////////////////////////////////////////////////////////////////////////

DeviceShader* DeviceShader::Create(ShaderType type, const void* data, size_t dataSize) {
  return new WindowsShader(type, data, dataSize);
}

DeviceShader* DeviceShader::Create(ShaderType type, const char* path) {
  return new WindowsShader(type, path);
}

////////////////////////////////////////////////////////////////////////////////
// Create Functions (Program)
////////////////////////////////////////////////////////////////////////////////

DeviceProgram* DeviceProgram::Create(const void* vertexShaderData, size_t vertexShaderDataSize, const void* fragmentShaderData, size_t fragmentShaderDataSize) {
  return new WindowsProgram(vertexShaderData, vertexShaderDataSize, fragmentShaderData, fragmentShaderDataSize);
}

DeviceProgram* DeviceProgram::Create(DeviceShader* vertexShader, DeviceShader* fragmentShader) {
  return new WindowsProgram(vertexShader, fragmentShader);
}

DeviceProgram* DeviceProgram::Create(const char* vertexShaderPath, const char* fragmentShaderPath) {
  return new WindowsProgram(vertexShaderPath, fragmentShaderPath);
}

////////////////////////////////////////////////////////////////////////////////
// Create Functions (Index Buffer)
////////////////////////////////////////////////////////////////////////////////

IndexBuffer* IndexBuffer::Create(IndexFormat format, const void* data, size_t indexCount) {
  return new WindowsIndexBuffer(format, data, indexCount);
}

////////////////////////////////////////////////////////////////////////////////
// Create Functions (Array Buffer)
////////////////////////////////////////////////////////////////////////////////

ArrayBuffer* ArrayBuffer::Create(size_t itemSize, const void* data, size_t itemCount, BufferPrimitive primitive) {
  return new WindowsArrayBuffer(itemSize, data, itemCount, primitive);
}

////////////////////////////////////////////////////////////////////////////////
// Create Functions (Tex)
////////////////////////////////////////////////////////////////////////////////

Tex* Tex::Create(u32 w, u32 h, TexFormat format, const void* pixels, const json& options) {
  return new WindowsTex(w, h, format, pixels, options);
}

Tex* Tex::Create(u32 w, u32 h, TexFormat format, const json& options) {
  return new WindowsTex(w, h, format, options);
}

Tex* Tex::Create(u32 w, u32 h, TexFormat format, const void* pixels) {
  return new WindowsTex(w, h, format, pixels);
}

Tex* Tex::Create() {
  return new WindowsTex();
}

Tex* Tex::Create(const std::string& name, const std::string& data) {
  return new WindowsTex(name, data);
}

////////////////////////////////////////////////////////////////////////////////
// Assert
////////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG)
void Prime::AssertCore(const char* file, u32 line, const char* f, ...) {
  va_list ap, ap2;
  volatile s32 wait = 1;
  std::string buffer;

  buffer.append("A failed assertion has occurred.\n");
  buffer.append(string_printf("File: %s\n", file));
  buffer.append(string_printf("Line: %d\n\n", line));

  va_start(ap, f);
  va_start(ap2, f);
  buffer.append(string_vprintf(f, ap, ap2));
  dbgprintf("%s", buffer.c_str());
  dbgprintf("\n");

#if defined(_WIN32) || defined(_WIN64)
  MessageBoxA(NULL, buffer.c_str(), "Assertion", MB_OK);
#endif

#if defined(_DEBUG)
  while(wait) {
    volatile s32 d;
    d = 1;
  }
#else
  exit(0);
#endif

  va_end(ap2);
  va_end(ap);
}
#endif
