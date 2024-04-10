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

#include <Prime/Config.h>
#if defined(PrimeTargetOpenGL)

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#if PrimeTargetWindows
#include <glad/glad.h>
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <GL/GL.h>
#ifdef Yield
#undef Yield
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT         0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT        0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT        0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT        0x83F3
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT        0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT  0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT  0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT  0x8C4F

#define GL_TEXTURE_MAX_ANISOTROPY 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY 0x84FF

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG)
namespace Prime {
void AssertOpenGLShaderCompileCore(GLuint shaderId);
void AssertOpenGLProgramLinkCore(GLuint programId);
};
#endif

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG)
#define PrimeAssertGLError(m) {s32 errgl; __PrimeOpenGLOutOfMemoryError = false; errgl = glGetError(); if(errgl == GL_OUT_OF_MEMORY) __PrimeOpenGLOutOfMemoryError = true; PrimeAssert(errgl == GL_NO_ERROR || errgl == GL_OUT_OF_MEMORY, "OpenGL errors exist: 0x%X\n" # m, errgl);} (void) 0
#define GLCMD(c) c; PrimeAssertGLError(#c)
#define GLCMD_NE(c) c; {while(glGetError() != GL_NO_ERROR) {}}
#define IsOpenGLOutOfMemory() __PrimeOpenGLOutOfMemoryError
#define ResetOpenGLOutOfMemory() __PrimeOpenGLOutOfMemoryError = false
#define PrimeAssertOpenGLShaderCompile(id) Prime::AssertOpenGLShaderCompileCore(id)
#define PrimeAssertOpenGLProgramLink(id) Prime::AssertOpenGLProgramLinkCore(id)
#else
#define PrimeAssertGLError(m)
#define GLCMD(c) c; {s32 errgl; __PrimeOpenGLOutOfMemoryError = false; errgl = glGetError(); if(errgl == GL_OUT_OF_MEMORY) __PrimeOpenGLOutOfMemoryError = true;}
#define GLCMD_NE(c) c; {while(glGetError() != GL_NO_ERROR) {}}
#define IsOpenGLOutOfMemory() __PrimeOpenGLOutOfMemoryError
#define ResetOpenGLOutOfMemory() __PrimeOpenGLOutOfMemoryError = false
#define PrimeAssertOpenGLShaderCompile(id)
#define PrimeAssertOpenGLProgramLink(id)
#endif

////////////////////////////////////////////////////////////////////////////////
// Externs
////////////////////////////////////////////////////////////////////////////////

extern bool __PrimeOpenGLOutOfMemoryError;

#endif
