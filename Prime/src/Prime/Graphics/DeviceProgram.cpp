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

#include <Prime/Graphics/DeviceProgram.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/DeviceShader.h>
#include <Prime/Graphics/Graphics.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

DeviceProgram::DeviceProgram(const void* vertexShaderData, size_t vertexShaderDataSize, const void* fragmentShaderData, size_t fragmentShaderDataSize):
vertexShader(nullptr),
fragmentShader(nullptr),
vertexShaderManaged(false),
fragmentShaderManaged(false),
mvpVariableStatus(DeviceProgramVariableStatusUnknown),
modelVariableStatus(DeviceProgramVariableStatusUnknown),
viewVariableStatus(DeviceProgramVariableStatusUnknown),
vpVariableStatus(DeviceProgramVariableStatusUnknown),
mvVariableStatus(DeviceProgramVariableStatusUnknown),
normalMatVariableStatus(DeviceProgramVariableStatusUnknown),
gposMatVariableStatus(DeviceProgramVariableStatusUnknown),
loadedIntoVRAM(false) {
  for(size_t i = 0; i < PRIME_DEVICE_PROGRAM_CLIP_PLANE_COUNT; i++) {
    clipPlaneVariableStatus[i] = DeviceProgramVariableStatusUnknown;
  }

  vertexShader = DeviceShader::Create(ShaderTypeVertex, vertexShaderData, vertexShaderDataSize);
  fragmentShader = DeviceShader::Create(ShaderTypeFragment, fragmentShaderData, fragmentShaderDataSize);
  vertexShaderManaged = true;
  fragmentShaderManaged = true;
}

DeviceProgram::DeviceProgram(DeviceShader* vertexShader, DeviceShader* fragmentShader):
vertexShader(vertexShader),
fragmentShader(fragmentShader),
vertexShaderManaged(false),
fragmentShaderManaged(false),
mvpVariableStatus(DeviceProgramVariableStatusUnknown),
modelVariableStatus(DeviceProgramVariableStatusUnknown),
viewVariableStatus(DeviceProgramVariableStatusUnknown),
vpVariableStatus(DeviceProgramVariableStatusUnknown),
mvVariableStatus(DeviceProgramVariableStatusUnknown),
normalMatVariableStatus(DeviceProgramVariableStatusUnknown),
gposMatVariableStatus(DeviceProgramVariableStatusUnknown),
loadedIntoVRAM(false) {
  for(size_t i = 0; i < PRIME_DEVICE_PROGRAM_CLIP_PLANE_COUNT; i++) {
    clipPlaneVariableStatus[i] = DeviceProgramVariableStatusUnknown;
  }
}

DeviceProgram::DeviceProgram(const char* vertexShaderPath, const char* fragmentShaderPath):
vertexShader(nullptr),
fragmentShader(nullptr),
vertexShaderManaged(false),
fragmentShaderManaged(false),
mvpVariableStatus(DeviceProgramVariableStatusUnknown),
modelVariableStatus(DeviceProgramVariableStatusUnknown),
viewVariableStatus(DeviceProgramVariableStatusUnknown),
vpVariableStatus(DeviceProgramVariableStatusUnknown),
mvVariableStatus(DeviceProgramVariableStatusUnknown),
normalMatVariableStatus(DeviceProgramVariableStatusUnknown),
gposMatVariableStatus(DeviceProgramVariableStatusUnknown),
loadedIntoVRAM(false) {
  for(size_t i = 0; i < PRIME_DEVICE_PROGRAM_CLIP_PLANE_COUNT; i++) {
    clipPlaneVariableStatus[i] = DeviceProgramVariableStatusUnknown;
  }

  IncRef();

  std::string fragmentShaderPath2(fragmentShaderPath);

  ReadFile(vertexShaderPath, [=](void* data, size_t size) mutable {
    void* vertexShaderData = data;
    size_t vertexShaderDataSize = size;

    ReadFile(fragmentShaderPath2.c_str(), [=](void* data, size_t size) mutable {
      void* fragmentShaderData = data;
      size_t fragmentShaderDataSize = size;

      if(vertexShaderData && fragmentShaderData) {
        PrimeSafeDelete(vertexShader);
        PrimeSafeDelete(fragmentShader);

        vertexShader = DeviceShader::Create(ShaderTypeVertex, vertexShaderData, vertexShaderDataSize);
        fragmentShader = DeviceShader::Create(ShaderTypeFragment, fragmentShaderData, fragmentShaderDataSize);
        vertexShaderManaged = true;
        fragmentShaderManaged = true;
      }

      if(vertexShaderData)
        free(vertexShaderData);

      if(fragmentShaderData)
        free(fragmentShaderData);

      DecRef();
    });
  });
}

DeviceProgram::~DeviceProgram() {
  PrimeAssert(!loadedIntoVRAM, "Device program still loaded in VRAM.");

  if(fragmentShaderManaged && fragmentShader) {
    PrimeSafeDelete(fragmentShader);
  }

  if(vertexShaderManaged && vertexShader) {
    PrimeSafeDelete(vertexShader);
  }
}

bool DeviceProgram::LoadIntoVRAM() {
  if(loadedIntoVRAM)
    return true;

  bool vertexShaderLoaded = vertexShader->LoadIntoVRAM();
  bool fragmentShaderLoaded = fragmentShader->LoadIntoVRAM();

  loadedIntoVRAM = vertexShaderLoaded && fragmentShaderLoaded;

  return loadedIntoVRAM;
}

bool DeviceProgram::UnloadFromVRAM() {
  if(!loadedIntoVRAM)
    return true;

  bool vertexShaderUnloaded = vertexShader->UnloadFromVRAM();
  bool fragmentShaderUnloaded = fragmentShader->UnloadFromVRAM();

  loadedIntoVRAM = vertexShaderUnloaded && fragmentShaderUnloaded;

  return !loadedIntoVRAM;
}

void DeviceProgram::ApplyVariableValues() {
  for(auto it: variables) {
    const GraphicsDictionaryKey& key = it.key();
    const GraphicsDictionaryValue& value = it.value();

    switch(value.GetType()) {
    case GraphicsDictionaryValueTypeNone:
      break;

    case GraphicsDictionaryValueTypeF32:
      if(key.isArray)
        SetArrayVariable(key.name, key.arrayIndex, value.GetF32());
      else
        SetVariable(key.name, value.GetF32());
      break;

    case GraphicsDictionaryValueTypeS32:
      if(key.isArray)
        SetArrayVariable(key, key.arrayIndex, value.GetS32());
      else
        SetVariable(key, value.GetS32());
      break;

    case GraphicsDictionaryValueTypeVec2: {
        Vec2 v = value.GetVec2();
        if(key.isArray)
          SetArrayVariable(key, key.arrayIndex, v);
        else
          SetVariable(key, v);
        break;
    }

    case GraphicsDictionaryValueTypeVec3: {
      Vec3 v = value.GetVec3();
      if(key.isArray)
        SetArrayVariable(key, key.arrayIndex, v);
      else
        SetVariable(key, v);
      break;
    }

    case GraphicsDictionaryValueTypeVec4: {
      Vec4 v = value.GetVec4();
      if(key.isArray)
        SetArrayVariable(key, key.arrayIndex, v);
      else
        SetVariable(key, v);
      break;
    }

    case GraphicsDictionaryValueTypeMat44: {
      const Mat44& mat = value.GetMat44();
      if(key.isArray)
        SetArrayVariable(key, key.arrayIndex, mat);
      else
        SetVariable(key, mat);
      break;
    }

    default:
      PrimeAssert(false, "Unknown device program variable value type.");
      break;
    }
  }
}

void DeviceProgram::CheckVariableStatus() {

}

void DeviceProgram::SetVariable(const std::string& name, s32 v) {
  variables[name] = v;
}

void DeviceProgram::SetVariable(const std::string& name, f32 v) {
  variables[name] = v;
}

void DeviceProgram::SetVariable(const std::string& name, const Vec2& v) {
  variables[name] = v;
}

void DeviceProgram::SetVariable(const std::string& name, const Vec3& v) {
  variables[name] = v;
}

void DeviceProgram::SetVariable(const std::string& name, const Vec4& v) {
  variables[name] = v;
}

void DeviceProgram::SetVariable(const std::string& name, const Mat44& mat) {
  variables[name] = mat;
}

void DeviceProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, s32 v) {
  variables[GraphicsDictionaryKey(name, arrayIndex)] = v;
}

void DeviceProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, f32 v) {
  variables[GraphicsDictionaryKey(name, arrayIndex)] = v;
}

void DeviceProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec2& v) {
  variables[GraphicsDictionaryKey(name, arrayIndex)] = v;
}

void DeviceProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec3& v) {
  variables[GraphicsDictionaryKey(name, arrayIndex)] = v;
}

void DeviceProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec4& v) {
  variables[GraphicsDictionaryKey(name, arrayIndex)] = v;
}

void DeviceProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, const Mat44& mat) {
  variables[GraphicsDictionaryKey(name, arrayIndex)] = mat;
}

void DeviceProgram::SetArrayVariable1fv(const std::string& name, const f32* v, size_t count, size_t start) {
  for(size_t i = 0; i < count; i++) {
    variables[GraphicsDictionaryKey(name, start + i)] = v[i];
  }
}

void DeviceProgram::SetArrayVariable2fv(const std::string& name, const f32* v, size_t count, size_t start) {
  for(size_t i = 0; i < count; i++) {
    variables[GraphicsDictionaryKey(name, start + i)] = Vec2(v[i * 2], v[i * 2 + 1]);
  }
}

void DeviceProgram::SetArrayVariable3fv(const std::string& name, const f32* v, size_t count, size_t start) {
  for(size_t i = 0; i < count; i++) {
    variables[GraphicsDictionaryKey(name, start + i)] = Vec3(v[i * 3], v[i * 3 + 1], v[i * 3 + 2]);
  }
}

void DeviceProgram::SetArrayVariable4fv(const std::string& name, const f32* v, size_t count, size_t start) {
  for(size_t i = 0; i < count; i++) {
    variables[GraphicsDictionaryKey(name, start + i)] = Vec4(v[i * 4], v[i * 4 + 1], v[i * 4 + 2], v[i * 4 + 3]);
  }
}

void DeviceProgram::SetArrayVariableMat44fv(const std::string& name, const f32* v, size_t count, size_t start) {
  for(size_t i = 0; i < count; i++) {
    const f32* p = &v[16 * i];
    variables[GraphicsDictionaryKey(name, start + i)] = Mat44(p);
  }
}

void DeviceProgram::LoadVariablesToShaderStage() {

}
