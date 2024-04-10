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
#include <Prime/Graphics/GraphicsDictionary.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PRIME_DEVICE_PROGRAM_CLIP_PLANE_COUNT 6

////////////////////////////////////////////////////////////////////////////////
// Enums
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  DeviceProgramVariableStatusUnknown = 0,
  DeviceProgramVariableStatusFound = 1,
  DeviceProgramVariableStatusNotFound = 2,
} DeviceProgramVariableStatus;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class DeviceShader;

class DeviceProgram: public RefObject {
friend class Graphics;
protected:

  DeviceShader* vertexShader;
  DeviceShader* fragmentShader;
  bool vertexShaderManaged;
  bool fragmentShaderManaged;

  DeviceProgramVariableStatus mvpVariableStatus;
  DeviceProgramVariableStatus modelVariableStatus;
  DeviceProgramVariableStatus viewVariableStatus;
  DeviceProgramVariableStatus vpVariableStatus;
  DeviceProgramVariableStatus mvVariableStatus;
  DeviceProgramVariableStatus normalMatVariableStatus;
  DeviceProgramVariableStatus gposMatVariableStatus;
  DeviceProgramVariableStatus clipPlaneVariableStatus[PRIME_DEVICE_PROGRAM_CLIP_PLANE_COUNT];

  GraphicsDictionary variables;

  bool loadedIntoVRAM;

public:

  DeviceShader* GetVertexShader() const {return vertexShader;}
  DeviceShader* GetFragmentShader() const {return fragmentShader;}
  bool HasVariableMVP() const {return mvpVariableStatus == DeviceProgramVariableStatusFound;}
  bool HasVariableModel() const {return modelVariableStatus == DeviceProgramVariableStatusFound;}
  bool HasVariableView() const {return viewVariableStatus == DeviceProgramVariableStatusFound;}
  bool HasVariableVP() const {return vpVariableStatus == DeviceProgramVariableStatusFound;}
  bool HasVariableMV() const {return mvVariableStatus == DeviceProgramVariableStatusFound;}
  bool HasVariableNormalMat() const {return normalMatVariableStatus == DeviceProgramVariableStatusFound;}
  bool HasVariableGPosMat() const {return gposMatVariableStatus == DeviceProgramVariableStatusFound;}
  bool HasVariableClipPlane(size_t index) const {return clipPlaneVariableStatus[index] == DeviceProgramVariableStatusFound;}
  bool IsLoadedIntoVRAM() const {return loadedIntoVRAM;}

protected:

  DeviceProgram(const void* vertexShaderData, size_t vertexShaderDataSize, const void* fragmentShaderData, size_t fragmentShaderDataSize);
  DeviceProgram(DeviceShader* vertexShader, DeviceShader* fragmentShader);
  DeviceProgram(const char* vertexShaderPath, const char* fragmentShaderPath);

public:

  virtual ~DeviceProgram();

  static DeviceProgram* Create(const void* vertexShaderData, size_t vertexShaderDataSize, const void* fragmentShaderData, size_t fragmentShaderDataSize);
  static DeviceProgram* Create(DeviceShader* vertexShader, DeviceShader* fragmentShader);
  static DeviceProgram* Create(const char* vertexShaderPath, const char* fragmentShaderPath);

public:

  virtual bool LoadIntoVRAM();
  virtual bool UnloadFromVRAM();

  virtual void CheckVariableStatus();
  virtual void ApplyVariableValues();

  virtual void SetVariable(const std::string& name, s32 v);
  virtual void SetVariable(const std::string& name, f32 v);
  virtual void SetVariable(const std::string& name, const Vec2& v);
  virtual void SetVariable(const std::string& name, const Vec3& v);
  virtual void SetVariable(const std::string& name, const Vec4& v);
  virtual void SetVariable(const std::string& name, const Mat44& mat);

  virtual void SetArrayVariable(const std::string& name, size_t arrayIndex, s32 v);
  virtual void SetArrayVariable(const std::string& name, size_t arrayIndex, f32 v);
  virtual void SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec2& v);
  virtual void SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec3& v);
  virtual void SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec4& v);
  virtual void SetArrayVariable(const std::string& name, size_t arrayIndex, const Mat44& mat);

  virtual void SetArrayVariable1fv(const std::string& name, const f32* v, size_t count, size_t start = 0);
  virtual void SetArrayVariable2fv(const std::string& name, const f32* v, size_t count, size_t start = 0);
  virtual void SetArrayVariable3fv(const std::string& name, const f32* v, size_t count, size_t start = 0);
  virtual void SetArrayVariable4fv(const std::string& name, const f32* v, size_t count, size_t start = 0);
  virtual void SetArrayVariableMat44fv(const std::string& name, const f32* v, size_t count, size_t start = 0);

  virtual void LoadVariablesToShaderStage();

};

};
