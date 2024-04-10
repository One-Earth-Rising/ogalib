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

#include <Prime/Graphics/DeviceShader.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

DeviceShader::DeviceShader(ShaderType type, const void* data, size_t dataSize):
deviceShaderType(type),
data(NULL),
dataSize(0),
loadedIntoVRAM(false) {
  PrimeAssert(data && dataSize > 0, "Shader data is empty.");
  if(dataSize > 0) {
    this->data = malloc(dataSize);
    PrimeAssert(this->data, "Could not create data.");
    if(data) {
      this->dataSize = dataSize;
      memcpy(this->data, data, this->dataSize);
    }
  }
}

DeviceShader::DeviceShader(ShaderType type, const char* path):
deviceShaderType(type),
data(NULL),
dataSize(0),
loadedIntoVRAM(false) {
  IncRef();

  ReadFile(path, [this](void* data, size_t size) {
    PrimeSafeFree(this->data);
    this->data = data;
    this->dataSize = dataSize;

    DecRef();
  });
}

DeviceShader::~DeviceShader() {
  PrimeAssert(!loadedIntoVRAM, "Shader still loaded in VRAM.");

  PrimeSafeFree(data);
}

bool DeviceShader::LoadIntoVRAM() {
  return false;
}

bool DeviceShader::UnloadFromVRAM() {
  return true;
}
