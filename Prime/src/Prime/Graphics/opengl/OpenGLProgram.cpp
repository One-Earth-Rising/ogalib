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

#include <Prime/Graphics/opengl/OpenGLProgram.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/opengl/OpenGLGraphics.h>
#include <Prime/Graphics/opengl/OpenGLShader.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

OpenGLProgram::OpenGLProgram(const void* vertexShaderData, size_t vertexShaderDataSize, const void* fragmentShaderData, size_t fragmentShaderDataSize): DeviceProgram(vertexShaderData, vertexShaderDataSize, fragmentShaderData, fragmentShaderDataSize),
variableBufferId(GL_NONE),
uniformBlockIndex(GL_INVALID_INDEX),
variableBuffer(nullptr),
variableBufferSize(0),
variableInfo(nullptr),
variableInfoCount(0),
attributeInfo(nullptr),
attributeInfoCount(0) {

}

OpenGLProgram::OpenGLProgram(DeviceShader* vertexShader, DeviceShader* fragmentShader): DeviceProgram(vertexShader, fragmentShader),
variableBufferId(GL_NONE),
uniformBlockIndex(GL_INVALID_INDEX),
variableBuffer(nullptr),
variableBufferSize(0),
variableInfo(nullptr),
variableInfoCount(0),
attributeInfo(nullptr),
attributeInfoCount(0) {

}

OpenGLProgram::OpenGLProgram(const char* vertexShaderPath, const char* fragmentShaderPath): DeviceProgram(vertexShaderPath, fragmentShaderPath),
variableBufferId(GL_NONE),
uniformBlockIndex(GL_INVALID_INDEX),
variableBuffer(nullptr),
variableBufferSize(0),
variableInfo(nullptr),
variableInfoCount(0),
attributeInfo(nullptr),
attributeInfoCount(0) {

}

OpenGLProgram::~OpenGLProgram() {
  UnloadFromVRAM();

  PrimeSafeFree(variableBuffer);
  PrimeSafeDeleteArray(variableInfo);

  PrimeSafeDeleteArray(attributeInfo);
}

bool OpenGLProgram::LoadIntoVRAM() {
  PxRequireMainThread;

  if(loadedIntoVRAM)
    return true;

  if(!vertexShader || !fragmentShader)
    return false;

  bool vertexShaderLoadedIntoVRAM = vertexShader->LoadIntoVRAM();
  bool fragmentShaderLoadedIntoVRAM = fragmentShader->LoadIntoVRAM();

  if(!vertexShaderLoadedIntoVRAM || !fragmentShaderLoadedIntoVRAM)
    return false;

  programId = GLCMD(glCreateProgram());
  if(IsOpenGLOutOfMemory()) {
    PrimeAssert(false, "Out of memory.");
  }
  else {
    InitOpenGLProgram(vertexShader, fragmentShader);
  }

  ProcessOpenGLProgramData();

  GLint oldVariableBufferId;
  GLCMD(glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &oldVariableBufferId));
  GLCMD(glGenBuffers(1, &variableBufferId));
  GLCMD(glBindBuffer(GL_UNIFORM_BUFFER, variableBufferId));
  GLCMD(glBufferData(GL_UNIFORM_BUFFER, variableBufferSize, variableBuffer, GL_STATIC_DRAW));
  GLCMD(glBindBufferBase(GL_UNIFORM_BUFFER, 0, variableBufferId));
  GLCMD(glBindBuffer(GL_UNIFORM_BUFFER, oldVariableBufferId));

  loadedIntoVRAM = true;

  CheckVariableStatus();
  ApplyVariableValues();

  return true;
}

bool OpenGLProgram::UnloadFromVRAM() {
  PxRequireMainThread;

  if(!loadedIntoVRAM)
    return true;

  if(variableBufferId) {
    GLCMD(glDeleteBuffers(1, &variableBufferId));
    variableBufferId = 0;
  }

  if(programId) {
    GLCMD(glDeleteProgram(programId));
    programId = 0;
  }

  vertexShader->UnloadFromVRAM();
  fragmentShader->UnloadFromVRAM();

  loadedIntoVRAM = false;

  return true;
}

void OpenGLProgram::ProcessOpenGLProgramData() {
  uniformBlockIndex = GLCMD(glGetUniformBlockIndex(programId, "ShaderUniformBlock"));
  if(uniformBlockIndex != GL_INVALID_INDEX) {
    GLCMD(glUniformBlockBinding(programId, uniformBlockIndex, 0));
  }

  GLint queryVariableBufferSize = 0;
  GLCMD(glGetActiveUniformBlockiv(programId, uniformBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &queryVariableBufferSize));
  variableBufferSize = queryVariableBufferSize;

  variableBuffer = memalign(64, variableBufferSize);
  memset(variableBuffer, 0, variableBufferSize);

  GLint queryVariableInfoCount = 0;
  GLCMD(glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &queryVariableInfoCount));
  variableInfoCount = queryVariableInfoCount;

  if(variableInfoCount > 0) {
    variableInfo = new OpenGLProgramVariableInfo[variableInfoCount];

    for(size_t i = 0; i < variableInfoCount; i++) {
      OpenGLProgramVariableInfo& info = variableInfo[i];

      GLuint uniformIndex = (GLuint) i;
      GLsizei uniformNameLength = 0;
      GLchar uniformName[1024];
      uniformName[0] = 0;

      GLCMD(glGetActiveUniformName(programId, uniformIndex, sizeof(uniformName) - 1, &uniformNameLength, uniformName));
      if(uniformNameLength > 0) {
        std::string name(uniformName);

        std::string::size_type charPos = name.find("[");
        if(charPos != std::string::npos) {
          name = name.substr(0, charPos);
        }

        PrimeAssert(!variableInfoLookup.HasKey(name), "Variable info already exists by name: %s", name.c_str());

        GLint uniformParam = 0;

        GLCMD(glGetActiveUniformsiv(programId, 1, &uniformIndex, GL_UNIFORM_TYPE, &uniformParam));
        if(uniformParam == GL_SAMPLER_1D || uniformParam == GL_SAMPLER_2D || uniformParam == GL_SAMPLER_3D) {
          textureLocLookup[textureLocLookup.GetCount()] = GLCMD(glGetUniformLocation(programId, name.c_str()));
        }
        else {
          GLCMD(glGetActiveUniformsiv(programId, 1, &uniformIndex, GL_UNIFORM_OFFSET, &uniformParam));
          info.addr = uniformParam;

          GLCMD(glGetActiveUniformsiv(programId, 1, &uniformIndex, GL_UNIFORM_SIZE, &uniformParam));
          if(uniformParam > 1) {
            info.arraySize = uniformParam;

            GLCMD(glGetActiveUniformsiv(programId, 1, &uniformIndex, GL_UNIFORM_ARRAY_STRIDE, &uniformParam));
            info.itemSize = uniformParam;
            info.itemPaddedSize = info.itemSize;

            GLCMD(glGetActiveUniformsiv(programId, 1, &uniformIndex, GL_UNIFORM_MATRIX_STRIDE, &uniformParam));
            if(uniformParam > 0) {
              info.itemAlignmentSize = uniformParam;
            }
            else {
              info.itemAlignmentSize = info.itemSize;
            }
          }
          else {
            info.arraySize = 0;
          }
        }

        variableInfoLookup[name] = &info;
      }
    }
  }

  GLint queryAttributeInfoCount = 0;
  GLCMD(glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &queryAttributeInfoCount));
  attributeInfoCount = queryAttributeInfoCount;

  if(attributeInfoCount > 0) {
    attributeInfo = new OpenGLProgramAttributeInfo[attributeInfoCount];

    for(size_t i = 0; i < attributeInfoCount; i++) {
      OpenGLProgramAttributeInfo& info = attributeInfo[i];

      GLuint attributeIndex = (GLuint) i;
      GLint attributeSize;
      GLenum attributeType;
      GLsizei attributeNameLength = 0;
      GLchar attributeName[1024];
      attributeName[0] = 0;

      GLCMD(glGetActiveAttrib(programId, attributeIndex, sizeof(attributeName) - 1, &attributeNameLength, &attributeSize, &attributeType, attributeName));
      if(attributeNameLength > 0) {
        std::string name(attributeName);

        std::string::size_type charPos = name.find("[");
        if(charPos != std::string::npos) {
          name = name.substr(0, charPos);
        }

        PrimeAssert(!attributeInfoLookup.HasKey(name), "Attribute info already exists by name: %s", name.c_str());

        size_t itemSize = 0;

        switch(attributeType) {
        case GL_FLOAT:
          itemSize = sizeof(f32);
          break;

        case GL_FLOAT_VEC2:
          itemSize = sizeof(Vec2);
          break;

        case GL_FLOAT_VEC3:
          itemSize = sizeof(Vec3);
          break;

        case GL_FLOAT_VEC4:
          itemSize = sizeof(Vec4);
          break;

        default:
          dbgprintf("[Warning] Unsupported attribute type: name = %s", name.c_str());
          break;
        };

        info.name = name;
        info.size = attributeSize * itemSize;
        info.loc = GLCMD(glGetAttribLocation(programId, name.c_str()));

        attributeInfoLookup[name] = &info;
      }
    }
  }
}

void OpenGLProgram::CheckVariableStatus() {
  mvpVariableStatus = GetVariableInfo("mvp") ? DeviceProgramVariableStatusFound : DeviceProgramVariableStatusNotFound;
  modelVariableStatus = GetVariableInfo("model") ? DeviceProgramVariableStatusFound : DeviceProgramVariableStatusNotFound;
  viewVariableStatus = GetVariableInfo("view") ? DeviceProgramVariableStatusFound : DeviceProgramVariableStatusNotFound;
  vpVariableStatus = GetVariableInfo("vp") ? DeviceProgramVariableStatusFound : DeviceProgramVariableStatusNotFound;
  mvVariableStatus = GetVariableInfo("mv") ? DeviceProgramVariableStatusFound : DeviceProgramVariableStatusNotFound;
  normalMatVariableStatus = GetVariableInfo("normalMat") ? DeviceProgramVariableStatusFound : DeviceProgramVariableStatusNotFound;
  gposMatVariableStatus = GetVariableInfo("gposMat") ? DeviceProgramVariableStatusFound : DeviceProgramVariableStatusNotFound;

  for(size_t i = 0; i < PRIME_DEVICE_PROGRAM_CLIP_PLANE_COUNT; i++) {
    clipPlaneVariableStatus[i] = GetVariableInfo(string_printf("clipPlane%d", i)) ? DeviceProgramVariableStatusFound : DeviceProgramVariableStatusNotFound;
  }
}

void OpenGLProgram::SetVariable(const std::string& name, s32 v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t addr = variableInfo->addr;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      s32* p = (s32*)&(((u8*) variableBuffer)[addr]);
      *p = v;
    }
  }
  else {
    DeviceProgram::SetVariable(name, v);
  }
}

void OpenGLProgram::SetVariable(const std::string& name, f32 v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t addr = variableInfo->addr;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
      *p = v;
    }
  }
  else {
    DeviceProgram::SetVariable(name, v);
  }
}

void OpenGLProgram::SetVariable(const std::string& name, const Vec2& v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t addr = variableInfo->addr;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
      *p++ = v.x;
      *p++ = v.y;
    }
  }
  else {
    DeviceProgram::SetVariable(name, v);
  }
}

void OpenGLProgram::SetVariable(const std::string& name, const Vec3& v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t addr = variableInfo->addr;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
      *p++ = v.x;
      *p++ = v.y;
      *p++ = v.z;
    }
  }
  else {
    DeviceProgram::SetVariable(name, v);
  }
}

void OpenGLProgram::SetVariable(const std::string& name, const Vec4& v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t addr = variableInfo->addr;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
      *p++ = v.x;
      *p++ = v.y;
      *p++ = v.z;
      *p++ = v.w;
    }
  }
  else {
    DeviceProgram::SetVariable(name, v);
  }
}

void OpenGLProgram::SetVariable(const std::string& name, const Mat44& v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t addr = variableInfo->addr;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
      for(u32 i = 0; i < 16; i++) {
        *p++ = v.e[i];
      }
    }
  }
  else {
    DeviceProgram::SetVariable(name, v);
  }
}

void OpenGLProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, s32 v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t useItemSize = (variableInfo->itemSize + (variableInfo->itemAlignmentSize - 1)) / variableInfo->itemAlignmentSize * variableInfo->itemAlignmentSize;
      size_t addr = variableInfo->addr + useItemSize * arrayIndex;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      s32* p = (s32*)&(((u8*) variableBuffer)[addr]);
      *p++ = v;
    }
    else {
      PrimeAssert(false, "Setting array variable on a non-array uniform.");
    }
  }
  else {
    DeviceProgram::SetArrayVariable(name, arrayIndex, v);
  }
}

void OpenGLProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, f32 v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t useItemSize = (variableInfo->itemSize + (variableInfo->itemAlignmentSize - 1)) / variableInfo->itemAlignmentSize * variableInfo->itemAlignmentSize;
      size_t addr = variableInfo->addr + useItemSize * arrayIndex;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
      *p++ = v;
    }
  }
  else {
    DeviceProgram::SetArrayVariable(name, arrayIndex, v);
  }
}

void OpenGLProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec2& v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t itemSize = (variableInfo->itemSize + (variableInfo->itemAlignmentSize - 1)) / variableInfo->itemAlignmentSize * variableInfo->itemAlignmentSize;
      size_t addr = variableInfo->addr + itemSize * arrayIndex;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
      *p++ = v.x;
      *p++ = v.y;
    }
  }
  else {
    DeviceProgram::SetArrayVariable(name, arrayIndex, v);
  }
}

void OpenGLProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec3& v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t itemSize = (variableInfo->itemSize + (variableInfo->itemAlignmentSize - 1)) / variableInfo->itemAlignmentSize * variableInfo->itemAlignmentSize;
      size_t addr = variableInfo->addr + itemSize * arrayIndex;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
      *p++ = v.x;
      *p++ = v.y;
      *p++ = v.z;
    }
  }
  else {
    DeviceProgram::SetArrayVariable(name, arrayIndex, v);
  }
}

void OpenGLProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec4& v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t itemSize = (variableInfo->itemSize + (variableInfo->itemAlignmentSize - 1)) / variableInfo->itemAlignmentSize * variableInfo->itemAlignmentSize;
      size_t addr = variableInfo->addr + itemSize * arrayIndex;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
      *p++ = v.x;
      *p++ = v.y;
      *p++ = v.z;
      *p++ = v.w;
    }
  }
  else {
    DeviceProgram::SetArrayVariable(name, arrayIndex, v);
  }
}

void OpenGLProgram::SetArrayVariable(const std::string& name, size_t arrayIndex, const Mat44& v) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      size_t itemSize = (variableInfo->itemSize + (variableInfo->itemAlignmentSize - 1)) / variableInfo->itemAlignmentSize * variableInfo->itemAlignmentSize;
      size_t addr = variableInfo->addr + itemSize * arrayIndex;
      PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
      f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
      for(u32 i = 0; i < 16; i++) {
        *p++ = v.e[i];
      }
    }
  }
  else {
    DeviceProgram::SetArrayVariable(name, arrayIndex, v);
  }
}

void OpenGLProgram::SetArrayVariable1fv(const std::string& name, const f32* v, size_t count, size_t start) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      if(variableInfo->arraySize) {
        size_t itemSize = (variableInfo->itemSize + (variableInfo->itemAlignmentSize - 1)) / variableInfo->itemAlignmentSize * variableInfo->itemAlignmentSize;
        for(size_t i = 0; i < count; i++) {
          size_t addr = variableInfo->addr + itemSize * (i + start);
          PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
          f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
          *p = v[i];
        }
      }
      else {
        size_t itemSize = sizeof(f32);
        for(size_t i = 0; i < count; i++) {
          size_t addr = variableInfo->addr + itemSize * (i + start);
          PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
          f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
          *p = v[i];
        }
      }
    }
  }
  else {
    DeviceProgram::SetArrayVariable1fv(name, v, count, start);
  }
}

void OpenGLProgram::SetArrayVariable2fv(const std::string& name, const f32* v, size_t count, size_t start) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      if(variableInfo->arraySize) {
        size_t itemSize = (variableInfo->itemSize + (variableInfo->itemAlignmentSize - 1)) / variableInfo->itemAlignmentSize * variableInfo->itemAlignmentSize;
        for(size_t i = 0; i < count; i++) {
          size_t addr = variableInfo->addr + itemSize * (i + start);
          PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
          f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
          const f32* s = &v[i << 1];
          *p++ = *s++;
          *p++ = *s++;
        }
      }
      else {
        size_t itemSize = sizeof(f32) * 2;
        for(size_t i = 0; i < count; i++) {
          size_t addr = variableInfo->addr + itemSize * (i + start);
          PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
          f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
          const f32* s = &v[i << 1];
          *p++ = *s++;
          *p++ = *s++;
        }
      }
    }
  }
  else {
    DeviceProgram::SetArrayVariable2fv(name, v, count, start);
  }
}

void OpenGLProgram::SetArrayVariable3fv(const std::string& name, const f32* v, size_t count, size_t start) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      if(variableInfo->arraySize) {
        size_t itemSize = (variableInfo->itemSize + (variableInfo->itemAlignmentSize - 1)) / variableInfo->itemAlignmentSize * variableInfo->itemAlignmentSize;
        for(size_t i = 0; i < count; i++) {
          size_t addr = variableInfo->addr + itemSize * (i + start);
          PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
          f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
          const f32* s = &v[i * 3];
          *p++ = *s++;
          *p++ = *s++;
          *p++ = *s++;
        }
      }
      else {
        size_t itemSize = sizeof(f32) * 3;
        for(size_t i = 0; i < count; i++) {
          size_t addr = variableInfo->addr + itemSize * (i + start);
          PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
          f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
          const f32* s = &v[i * 3];
          *p++ = *s++;
          *p++ = *s++;
          *p++ = *s++;
        }
      }
    }
  }
  else {
    DeviceProgram::SetArrayVariable3fv(name, v, count, start);
  }
}

void OpenGLProgram::SetArrayVariable4fv(const std::string& name, const f32* v, size_t count, size_t start) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      if(variableInfo->arraySize) {
        size_t itemSize = (variableInfo->itemSize + (variableInfo->itemAlignmentSize - 1)) / variableInfo->itemAlignmentSize * variableInfo->itemAlignmentSize;
        for(size_t i = 0; i < count; i++) {
          size_t addr = variableInfo->addr + itemSize * (i + start);
          PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
          f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
          const f32* s = &v[i << 2];
          *p++ = *s++;
          *p++ = *s++;
          *p++ = *s++;
          *p++ = *s++;
        }
      }
      else {
        size_t itemSize = sizeof(f32) * 4;
        for(size_t i = 0; i < count; i++) {
          size_t addr = variableInfo->addr + itemSize * (i + start);
          PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
          f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
          const f32* s = &v[i << 2];
          *p++ = *s++;
          *p++ = *s++;
          *p++ = *s++;
          *p++ = *s++;
        }
      }
    }
  }
  else {
    DeviceProgram::SetArrayVariable4fv(name, v, count, start);
  }
}

void OpenGLProgram::SetArrayVariableMat44fv(const std::string& name, const f32* v, size_t count, size_t start) {
  if(loadedIntoVRAM) {
    if(auto it = variableInfoLookup.Find(name)) {
      auto variableInfo = it.value();
      if(variableInfo->arraySize) {
        size_t itemSize = (variableInfo->itemSize + (variableInfo->itemAlignmentSize - 1)) / variableInfo->itemAlignmentSize * variableInfo->itemAlignmentSize;
        for(size_t i = 0; i < count; i++) {
          size_t addr = variableInfo->addr + itemSize * (i + start);
          PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
          f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
          const f32* s = &v[i << 4];
          for(u32 j = 0; j < 16; j++) {
            p[j] = s[j];
          }
        }
      }
      else {
        size_t itemSize = sizeof(f32) * 16;
        for(size_t i = 0; i < count; i++) {
          size_t addr = variableInfo->addr + itemSize * (i + start);
          PrimeAssert(addr < variableBufferSize, "Program variable address is out of range.");
          f32* p = (f32*)&(((u8*) variableBuffer)[addr]);
          const f32* s = &v[i << 4];
          for(u32 j = 0; j < 16; j++) {
            p[j] = s[j];
          }
        }
      }
    }
  }
  else {
    DeviceProgram::SetArrayVariableMat44fv(name, v, count, start);
  }
}

void OpenGLProgram::LoadVariablesToShaderStage() {
  if(uniformBlockIndex != -1) {
    GLint id;
    GLCMD(glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &id));
    GLCMD(glBindBuffer(GL_UNIFORM_BUFFER, variableBufferId));
    GLCMD(glBufferData(GL_UNIFORM_BUFFER, variableBufferSize, variableBuffer, GL_STATIC_DRAW));
    GLCMD(glBindBufferBase(GL_UNIFORM_BUFFER, 0, variableBufferId));
    GLCMD(glBindBuffer(GL_UNIFORM_BUFFER, id));
  }
}

const OpenGLProgramVariableInfo* OpenGLProgram::GetVariableInfo(size_t index) const {
  if(index >= variableInfoCount)
    return nullptr;

  return &variableInfo[index];
}

const OpenGLProgramVariableInfo* OpenGLProgram::GetVariableInfo(const std::string& name) const {
  if(auto it = variableInfoLookup.Find(name)) {
    return it.value();
  }

  return nullptr;
}

const OpenGLProgramAttributeInfo* OpenGLProgram::GetAttributeInfo(size_t index) const {
  if(index >= attributeInfoCount)
    return nullptr;

  return &attributeInfo[index];
}

GLint OpenGLProgram::GetTextureLoc(size_t unit) const {
  if(auto it = textureLocLookup.Find(unit)) {
    return it.value();
  }

  return -1;
}

void OpenGLProgram::InitOpenGLProgram(DeviceShader* vertexShader, DeviceShader* fragmentShader) {
  OpenGLShader* vertexShaderOpenGL = static_cast<OpenGLShader*>(vertexShader);
  OpenGLShader* fragmentShaderOpenGL = static_cast<OpenGLShader*>(fragmentShader);

  PrimeAssert(vertexShaderOpenGL->GetShaderId() != 0, "Invalid vertex shader.");
  PrimeAssert(fragmentShaderOpenGL->GetShaderId() != 0, "Invalid fragment shader.");

  GLCMD(glAttachShader(programId, vertexShaderOpenGL->GetShaderId()));
  GLCMD(glAttachShader(programId, fragmentShaderOpenGL->GetShaderId()));

  GLCMD(glLinkProgram(programId));
  PrimeAssertOpenGLProgramLink(programId);
}
