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

#pragma once

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Config.h>
#include <Prime/System/RefObject.h>
#include <Prime/Enum/ShaderType.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class DeviceShader: public RefObject {
protected:

  void* data;
  size_t dataSize;
  ShaderType deviceShaderType;
  bool loadedIntoVRAM;

public:

  ShaderType GetDeviceShaderType() const {return deviceShaderType;}
  bool IsLoadedIntoVRAM() const {return loadedIntoVRAM;}

protected:

  DeviceShader(ShaderType type, const void* shaderData, size_t shaderDataSize);
  DeviceShader(ShaderType type, const char* path);

public:

  virtual ~DeviceShader();

  static DeviceShader* Create(ShaderType type, const void* shaderData, size_t shaderDataSize);
  static DeviceShader* Create(ShaderType type, const char* path);

public:

  virtual bool LoadIntoVRAM();
  virtual bool UnloadFromVRAM();

};

};
