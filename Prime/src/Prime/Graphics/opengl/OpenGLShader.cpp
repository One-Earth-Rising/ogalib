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

#include <Prime/Graphics/opengl/OpenGLShader.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/opengl/OpenGLGraphics.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ThreadMutex* activeOpenGLShaderMutex = nullptr;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

namespace Prime {
void OnOpenGLShaderInitGlobal();
void OnOpenGLShaderShutdownGlobal();
void OnOpenGLShaderVRAMLost();
}

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

static const s8 PrimeOpenGLShaderFormatHeader[] = {'\xE3', 'P', 'S', 'O', '\x0D', '\x0A', '\x01', '\0'};

static const GLenum OpenGLShaderTypeMap[] = {
  GL_NONE,
  GL_VERTEX_SHADER,
  GL_FRAGMENT_SHADER,
};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

OpenGLShader::OpenGLShader(ShaderType type, const void* data, size_t dataSize): DeviceShader(type, data, dataSize),
shaderId(GL_NONE) {

}

OpenGLShader::OpenGLShader(ShaderType type, const char* path): DeviceShader(type, path),
shaderId(GL_NONE) {

}

OpenGLShader::~OpenGLShader() {
  UnloadFromVRAM();
}

bool OpenGLShader::LoadIntoVRAM() {
  PxRequireMainThread;

  if(loadedIntoVRAM)
    return true;

  if(data == nullptr || dataSize == 0)
    return false;

  shaderId = GLCMD(glCreateShader(OpenGLShaderTypeMap[deviceShaderType]));
  if(IsOpenGLOutOfMemory()) {
    PrimeAssert(false, "Out of memory.");
  }
  else {
    if(data && dataSize) {
      const GLchar* sourceList[] = {(const GLchar*) data, 0};
      const GLint sourceLengthList[] = {(GLint) dataSize, 0};
      GLCMD(glShaderSource(shaderId, 1, sourceList, sourceLengthList));
      if(IsOpenGLOutOfMemory()) {
        PrimeAssert(false, "Out of memory.");
      }
      GLCMD(glCompileShader(shaderId));
      PrimeAssertOpenGLShaderCompile(shaderId);
    }
    else {
      PrimeAssert(false, "Shader data is empty.");
    }
  }

  loadedIntoVRAM = true;

  return true;
}

bool OpenGLShader::UnloadFromVRAM() {
  PxRequireMainThread;

  if(!loadedIntoVRAM)
    return true;

  if(shaderId) {
    GLCMD(glDeleteShader(shaderId));
    shaderId = 0;
  }

  loadedIntoVRAM = false;

  return true;
}
