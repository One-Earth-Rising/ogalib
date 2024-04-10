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

#include <Prime/Graphics/DeviceProgram.h>
#include <Prime/Graphics/opengl/OpenGLShader.h>

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _OpenGLProgramVariableInfo {
  std::string name;
  size_t addr;
  size_t itemSize;
  size_t itemAlignmentSize;
  size_t itemPaddedSize;
  size_t arraySize;
  GLint loc;
} OpenGLProgramVariableInfo;

typedef struct _OpenGLProgramAttributeInfo {
  std::string name;
  size_t size;
  GLint loc;
} OpenGLProgramAttributeInfo;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class OpenGLProgram: public DeviceProgram {
protected:

  GLuint programId;

  GLuint variableBufferId;
  GLint uniformBlockIndex;

  void* variableBuffer;
  size_t variableBufferSize;

  Dictionary<std::string, OpenGLProgramVariableInfo*> variableInfoLookup;
  OpenGLProgramVariableInfo* variableInfo;
  size_t variableInfoCount;

  Dictionary<std::string, OpenGLProgramAttributeInfo*> attributeInfoLookup;
  OpenGLProgramAttributeInfo* attributeInfo;
  size_t attributeInfoCount;

  Dictionary<size_t, GLint> textureLocLookup;

public:

  GLuint GetProgramId() const {return programId;}

  GLuint GetVariableBufferId() const {return variableBufferId;}
  GLint GetUniformBlockIndex() const {return uniformBlockIndex;}

  size_t GetAttributeCount() const {return attributeInfoCount;}

public:

  OpenGLProgram(const void* vertexShaderData, size_t vertexShaderDataSize, const void* fragmentShaderData, size_t fragmentShaderDataSize);
  OpenGLProgram(DeviceShader* vertexShader, DeviceShader* fragmentShader);
  OpenGLProgram(const char* vertexShaderPath, const char* fragmentShaderPath);
  ~OpenGLProgram();

public:

  bool LoadIntoVRAM();
  bool UnloadFromVRAM();

  void ProcessOpenGLProgramData();

  void CheckVariableStatus() override;

  void SetVariable(const std::string& name, s32 v) override;
  void SetVariable(const std::string& name, f32 v) override;
  void SetVariable(const std::string& name, const Vec2& v) override;
  void SetVariable(const std::string& name, const Vec3& v) override;
  void SetVariable(const std::string& name, const Vec4& v) override;
  void SetVariable(const std::string& name, const Mat44& mat) override;

  void SetArrayVariable(const std::string& name, size_t arrayIndex, s32 v) override;
  void SetArrayVariable(const std::string& name, size_t arrayIndex, f32 v) override;
  void SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec2& v) override;
  void SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec3& v) override;
  void SetArrayVariable(const std::string& name, size_t arrayIndex, const Vec4& v) override;
  void SetArrayVariable(const std::string& name, size_t arrayIndex, const Mat44& mat) override;

  void SetArrayVariable1fv(const std::string& name, const f32* v, size_t count, size_t start = 0) override;
  void SetArrayVariable2fv(const std::string& name, const f32* v, size_t count, size_t start = 0) override;
  void SetArrayVariable3fv(const std::string& name, const f32* v, size_t count, size_t start = 0) override;
  void SetArrayVariable4fv(const std::string& name, const f32* v, size_t count, size_t start = 0) override;
  void SetArrayVariableMat44fv(const std::string& name, const f32* v, size_t count, size_t start = 0) override;

  void LoadVariablesToShaderStage() override;

  virtual const OpenGLProgramVariableInfo* GetVariableInfo(size_t index) const;
  virtual const OpenGLProgramVariableInfo* GetVariableInfo(const std::string& name) const;
  virtual const OpenGLProgramAttributeInfo* GetAttributeInfo(size_t index) const;
  virtual GLint GetTextureLoc(size_t unit) const;

private:

  void InitOpenGLProgram(DeviceShader* vertexShader, DeviceShader* fragmentShader);

};

};
