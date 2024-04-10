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
#if defined(PrimeTargetOpenGL)

#include <Prime/Graphics/opengl/OpenGLTex.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Graphics/opengl/OpenGLGraphics.h>
#include <png/png.h>
#include <png/pngstruct.h>
#include <png/pnginfo.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static void* pixelsBuffer = nullptr;
static size_t pixelsBufferSize = 0;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

static bool LoadPixelDataIntoBuffer(const TexData* texData);

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

static const u32 OpenGLTexWrapModeTable[] = {
  GL_CLAMP_TO_EDGE,
  GL_REPEAT,
  GL_MIRRORED_REPEAT,
};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

OpenGLTex::OpenGLTex(u32 w, u32 h, TexFormat format, const void* pixels, const json& options): Tex(w, h, format, pixels, options),
textureId(0),
depthTextureId(0),
frameBufferId(0),
renderBufferId(0),
bufferComplete(false) {

}

OpenGLTex::OpenGLTex(u32 w, u32 h, TexFormat format, const json& options): OpenGLTex(w, h, format, nullptr, options) {

}

OpenGLTex::OpenGLTex(u32 w, u32 h, TexFormat format, const void* pixels): OpenGLTex(w, h, format, pixels, json()) {

}

OpenGLTex::OpenGLTex(): Tex(),
textureId(0),
depthTextureId(0),
frameBufferId(0),
renderBufferId(0),
bufferComplete(false) {

}

OpenGLTex::OpenGLTex(const std::string& name, const std::string& data): Tex(name, data),
textureId(0),
depthTextureId(0),
frameBufferId(0),
renderBufferId(0),
bufferComplete(false) {

}

OpenGLTex::~OpenGLTex() {
  UnloadFromVRAM();
}

void OpenGLTex::SetFilteringEnabled(bool enabled) {
  if(loadedIntoVRAM) {
    GLint oldTextureId;
    GLCMD(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTextureId));
    GLCMD(glBindTexture(GL_TEXTURE_2D, textureId));
    
    if(enabled) {
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      if(loadedLevelCount > 1) {
        GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
      }
      else {
        GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      }
    }
    else {
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
      if(loadedLevelCount > 1) {
        GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
      }
      else {
        GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
      }
    }
    GLCMD(glBindTexture(GL_TEXTURE_2D, oldTextureId));
  }
  filteringEnabled = enabled;
}

void OpenGLTex::SetWrapModeX(WrapMode wrapModeX) {
  if(loadedIntoVRAM) {
    GLint oldTextureId;
    GLCMD(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTextureId));
    GLCMD(glBindTexture(GL_TEXTURE_2D, textureId));
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLTexWrapModeTable[wrapModeX]));
    GLCMD(glBindTexture(GL_TEXTURE_2D, oldTextureId));
  }

  Tex::SetWrapModeX(wrapModeX);
}

void OpenGLTex::SetWrapModeY(WrapMode wrapModeY) {
  if(loadedIntoVRAM) {
    GLint oldTextureId;
    GLCMD(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTextureId));
    GLCMD(glBindTexture(GL_TEXTURE_2D, textureId));
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLTexWrapModeTable[wrapModeY]));
    GLCMD(glBindTexture(GL_TEXTURE_2D, oldTextureId));
  }

  Tex::SetWrapModeY(wrapModeY);
}

bool OpenGLTex::LoadIntoVRAM() {
  bool result;

  if(renderBufferTexFormat)
    result = LoadIntoVRAMOpenGLRenderBuffer();
  else
    result = LoadIntoVRAMOpenGLTex();

  return result;
}

bool OpenGLTex::UnloadFromVRAM() {
  bool result;

  if(renderBufferTexFormat)
    result = UnloadFromVRAMOpenGLRenderBuffer();
  else
    result = UnloadFromVRAMOpenGLTex();

  return result;
}

void OpenGLTex::OnWillDeleteTexData(TexData& texData) {
  texDataGLLevelLookup.Remove(&texData);

  size_t levelCount = texDataGLLevelLookup.GetCount();
  if(levelCount == 0) {
    UnloadFromVRAM();
  }

  Tex::OnWillDeleteTexData(texData);
}

bool OpenGLTex::LoadIntoVRAMOpenGLTex() {
  if(loadedIntoVRAM)
    return true;

  OpenGLGraphics& g = PxOpenGLGraphics;

  GLint oldTextureId;
  GLCMD(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTextureId));
  GLCMD(glGenTextures(1, &textureId));
  GLCMD(glBindTexture(GL_TEXTURE_2D, textureId));

  GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLTexWrapModeTable[wrapModeX]));
  GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLTexWrapModeTable[wrapModeY]));

  Stack<TexDataLevelSortItem> levels;
  GetTexDataAsLevels(levels);
  size_t levelCount = levels.GetCount();
  if(levelCount > 0) {
    TexData* prevTexData = nullptr;
    size_t level = 0;
    for(const auto& item: levels) {
      TexData* texData = item.texData;

      if(prevTexData && (prevTexData->tw >> 1) != texData->tw) {
        break;
      }

      if(LoadPixelDataIntoBuffer(texData)) {
        GLint levelGL = (GLint) level;

        if(texData->format == TexFormatNative) {
          const std::string& formatName = texData->formatName;

          if(formatName == "bc1") {
            GLsizei size = (GLsizei) texData->pixels->GetSize();
            GLCMD(glCompressedTexImage2D(GL_TEXTURE_2D, levelGL, GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, (GLsizei) texData->tw, (GLsizei) texData->th, 0, size, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
          }
          else if(formatName == "bc2") {
            GLsizei size = (GLsizei) texData->pixels->GetSize();
            GLCMD(glCompressedTexImage2D(GL_TEXTURE_2D, levelGL, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, (GLsizei) texData->tw, (GLsizei) texData->th, 0, size, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
          }
          else if(formatName == "bc3") {
            GLsizei size = (GLsizei) texData->pixels->GetSize();
            GLCMD(glCompressedTexImage2D(GL_TEXTURE_2D, levelGL, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, (GLsizei) texData->tw, (GLsizei) texData->th, 0, size, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
          }
          else if(formatName == "R8G8B8A8_sRGB") {
            GLCMD(glTexImage2D(GL_TEXTURE_2D, levelGL, GL_SRGB8_ALPHA8, (GLsizei) texData->tw, (GLsizei) texData->th, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
          }
          else if(formatName == "R8G8B8_sRGB") {
            GLCMD(glTexImage2D(GL_TEXTURE_2D, levelGL, GL_SRGB8, (GLsizei) texData->tw, (GLsizei) texData->th, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
          }
          else if(formatName == "R16G16B16A16_sRGB") {
            GLCMD(glTexImage2D(GL_TEXTURE_2D, levelGL, GL_SRGB8_ALPHA8, (GLsizei) texData->tw, (GLsizei) texData->th, 0, GL_RGBA, GL_UNSIGNED_SHORT, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
          }
          else if(formatName == "R16G16B16_sRGB") {
            GLCMD(glTexImage2D(GL_TEXTURE_2D, levelGL, GL_SRGB8, (GLsizei) texData->tw, (GLsizei) texData->th, 0, GL_RGB, GL_UNSIGNED_SHORT, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
          }
        }
        else {
          switch(texData->format) {
          case TexFormatR8G8B8A8:
            GLCMD(glTexImage2D(GL_TEXTURE_2D, levelGL, GL_RGBA8, (GLsizei) texData->tw, (GLsizei) texData->th, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
            break;

          case TexFormatR8G8B8:
            GLCMD(glTexImage2D(GL_TEXTURE_2D, levelGL, GL_RGB8, (GLsizei) texData->tw, (GLsizei) texData->th, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
            break;

          case TexFormatR8G8:
            GLCMD(glTexImage2D(GL_TEXTURE_2D, levelGL, GL_RG, (GLsizei) texData->tw, (GLsizei) texData->th, 0, GL_RG, GL_UNSIGNED_BYTE, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
            break;

          case TexFormatR8:
            GLCMD(glTexImage2D(GL_TEXTURE_2D, levelGL, GL_RED, (GLsizei) texData->tw, (GLsizei) texData->th, 0, GL_RED, GL_UNSIGNED_BYTE, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
            break;

          case TexFormatR5G6B5:
            GLCMD(glTexImage2D(GL_TEXTURE_2D, levelGL, GL_RGB, (GLsizei) texData->tw, (GLsizei) texData->th, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
            break;

          case TexFormatR5G5B5A1:
            GLCMD(glTexImage2D(GL_TEXTURE_2D, levelGL, GL_RGBA, (GLsizei) texData->tw, (GLsizei) texData->th, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
            break;

          case TexFormatR4G4B4A4:
            GLCMD(glTexImage2D(GL_TEXTURE_2D, levelGL, GL_RGBA, (GLsizei) texData->tw, (GLsizei) texData->th, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, pixelsBuffer));
            texDataGLLevelLookup[texData] = levelGL;
            loadedLevelCount++;
            break;

          default:
            break;
          }
        }
      }

      level++;
      prevTexData = texData;
    }

    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint) loadedLevelCount - 1));

    if(filteringEnabled) {
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      if(loadedLevelCount > 1) {
        GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
      }
      else {
        GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      }
    }
    else {
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
      if(loadedLevelCount > 1) {
        GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
      }
      else {
        GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
      }
    }

    GLfloat anisotropicFilteringValue;
    GLCMD_NE(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisotropicFilteringValue));
    GLCMD_NE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, anisotropicFilteringValue));
  }
  else {
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));
  }

  GLCMD(glBindTexture(GL_TEXTURE_2D, oldTextureId));

  if(IsOpenGLOutOfMemory()) {
    ResetOpenGLOutOfMemory();
    GLCMD(glDeleteTextures(1, &textureId));
    textureId = 0;
    return false;
  }

  loadedIntoVRAM = true;

  return true;
}

bool OpenGLTex::LoadIntoVRAMOpenGLRenderBuffer() {
  if(loadedIntoVRAM)
    return true;

  OpenGLGraphics& g = PxOpenGLGraphics;

  u32 tw = GetRenderBufferTW();
  if(tw > g.GetMaxTexW()) {
    PrimeAssert(false, "Texture width is too large.");
    return false;
  }

  u32 th = GetRenderBufferTH();
  if(th > g.GetMaxTexH()) {
    PrimeAssert(false, "Texture height is too large.");
    return false;
  }

  GLint oldTextureId;
  GLCMD(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTextureId));
  GLCMD(glGenTextures(1, &textureId));
  GLCMD(glBindTexture(GL_TEXTURE_2D, textureId));

  GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLTexWrapModeTable[wrapModeX]));
  GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLTexWrapModeTable[wrapModeY]));

  switch(renderBufferTexFormat) {
  case TexFormatR8G8B8A8:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
    break;

  case TexFormatR8:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, tw, th, 0, GL_RED, GL_UNSIGNED_BYTE, 0));
    break;

  case TexFormatR8G8:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, tw, th, 0, GL_RG, GL_UNSIGNED_BYTE, 0));
    break;

  case TexFormatR8G8B8:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tw, th, 0, GL_RGB, GL_UNSIGNED_BYTE, 0));
    break;

  case TexFormatR5G6B5:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tw, th, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0));
    break;

  case TexFormatR5G5B5A1:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tw, th, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 0));
    break;

  case TexFormatR4G4B4A4:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA4, tw, th, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 0));
    break;

  case TexFormatFPBuffer:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, tw, th, 0, GL_RGBA, GL_FLOAT, 0));
    break;

  case TexFormatFPBufferHQ:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tw, th, 0, GL_RGBA, GL_FLOAT, 0));
    break;

  case TexFormatFPBufferNoAlpha:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, tw, th, 0, GL_RGB, GL_FLOAT, 0));
    break;

  case TexFormatFPBufferNoAlphaHQ:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, tw, th, 0, GL_RGB, GL_FLOAT, 0));
    break;

  case TexFormatDepthBuffer:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, tw, th, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0));
    break;

  case TexFormatShadowMap:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, tw, th, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0));
    break;

  case TexFormatPositionBuffer:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, tw, th, 0, GL_RGBA, GL_FLOAT, 0));
    break;

  case TexFormatNormalBuffer:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tw, th, 0, GL_RGB, GL_UNSIGNED_BYTE, 0));
    break;

  case TexFormatGlowBuffer:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, tw, th, 0, GL_RED, GL_FLOAT, 0));
    break;

  case TexFormatSpecularBuffer:
    GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, tw, th, 0, GL_RG, GL_FLOAT, 0));
    break;

  default:
    PrimeAssert(false, "Unknown or unsupported texture format for render buffer.");
    return false;
  }

  if(IsOpenGLOutOfMemory()) {
    return false;
  }

  switch(renderBufferTexFormat) {
  case TexFormatDepthBuffer:
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLTexWrapModeTable[wrapModeX]));
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLTexWrapModeTable[wrapModeY]));
    GLCMD(glGenFramebuffers(1, &frameBufferId));
    GLCMD(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId));
    GLCMD(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureId, 0));
    GLCMD(glDrawBuffer(GL_NONE));
    GLCMD(glReadBuffer(GL_NONE));
    break;

  case TexFormatShadowMap:
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLTexWrapModeTable[wrapModeX]));
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLTexWrapModeTable[wrapModeY]));
    GLCMD(glGenFramebuffers(1, &frameBufferId));
    GLCMD(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId));
    GLCMD(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureId, 0));
    GLCMD(glDrawBuffer(GL_NONE));
    GLCMD(glReadBuffer(GL_NONE));
    break;

  default:
    if(filteringEnabled) {
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    }
    else {
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    }
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLTexWrapModeTable[wrapModeX]));
    GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLTexWrapModeTable[wrapModeY]));
    GLCMD(glGenFramebuffers(1, &frameBufferId));
    GLCMD(glGenRenderbuffers(1, &renderBufferId));
    GLCMD(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId));
    GLCMD(glBindRenderbuffer(GL_RENDERBUFFER, renderBufferId));
    GLCMD(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0));
    if(renderBufferNeedsDepth) {
      GLCMD(glGenTextures(1, &depthTextureId));
      GLCMD(glBindTexture(GL_TEXTURE_2D, depthTextureId));
      GLCMD(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, tw, th, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGLTexWrapModeTable[wrapModeX]));
      GLCMD(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGLTexWrapModeTable[wrapModeY]));
      GLCMD(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId, 0));
    }
    break;
  }

  GLfloat anisotropicFilteringValue;
  GLCMD_NE(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisotropicFilteringValue));
  GLCMD_NE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, anisotropicFilteringValue));

  GLenum result = GLCMD(glCheckFramebufferStatus(GL_FRAMEBUFFER));
  bufferComplete = (result == GL_FRAMEBUFFER_COMPLETE) || (result == 0);
  PrimeAssert(bufferComplete, "Could not create framebuffer object: result = 0x%X", result);

  GLCMD(glBindRenderbuffer(GL_RENDERBUFFER, 0));
  GLCMD(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  GLCMD(glBindTexture(GL_TEXTURE_2D, oldTextureId));

  loadedIntoVRAM = true;

  return true;
}

bool OpenGLTex::UnloadFromVRAMOpenGLTex() {
  if(!loadedIntoVRAM)
    return true;

  if(depthTextureId) {
    GLCMD(glDeleteTextures(1, &depthTextureId));
    depthTextureId = 0;
  }

  if(textureId) {
    GLCMD(glDeleteTextures(1, &textureId));
    textureId = 0;
  }

  loadedLevelCount = 0;

  loadedIntoVRAM = false;

  return true;
}

bool OpenGLTex::UnloadFromVRAMOpenGLRenderBuffer() {
  if(!loadedIntoVRAM)
    return true;

  if(renderBufferId) {
    GLCMD(glDeleteRenderbuffers(1, &renderBufferId));
    renderBufferId = 0;
  }

  if(frameBufferId) {
    GLCMD(glDeleteFramebuffers(1, &frameBufferId));
    frameBufferId = 0;
  }

  if(depthTextureId) {
    GLCMD(glDeleteTextures(1, &depthTextureId));
    depthTextureId = 0;
  }

  if(textureId) {
    GLCMD(glDeleteTextures(1, &textureId));
    textureId = 0;
  }

  loadedIntoVRAM = false;

  return true;
}

void OpenGLTex::InitGlobal() {

}

void OpenGLTex::ShutdownGlobal() {
  PrimeSafeFree(pixelsBuffer);
  pixelsBufferSize = 0;
}

bool LoadPixelDataIntoBuffer(const TexData* texData) {
  if(!texData)
    return false;

  if(!texData->pixels)
    return false;

  const BlockBuffer* pixels = texData->pixels;
  size_t pixelsSize;

  size_t blockSize = pixels->GetBlockSize();
  size_t stride = blockSize;

  bool epblRequired = true;

  if(texData->format == TexFormatNative) {
    bool preformatted = texData->formatName == "bc1"
      || texData->formatName == "bc2"
      || texData->formatName == "bc3"
      || texData->formatName == "bc6h"
      || texData->formatName == "bc7";

    epblRequired = !preformatted;
  }

  if(epblRequired && (blockSize & 3) != 0) {  // OpenGL requires each row of pixels to be divisible by 4 bytes.
    u32 epbl = 4 - (blockSize & 3);
    stride = blockSize + epbl;
    pixelsSize = stride * texData->th;
  }
  else {
    pixelsSize = pixels->GetSize();
  }

  if(!pixelsBuffer || pixelsBufferSize < pixelsSize) {
    void* newPixelsBuffer = realloc(pixelsBuffer, pixelsSize);
    if(newPixelsBuffer) {
      pixelsBuffer = newPixelsBuffer;
      pixelsBufferSize = pixelsSize;
    }
  }

  if(pixelsBuffer && pixelsBufferSize >= pixelsSize) {
    size_t blockSize = pixels->GetBlockSize();
    if((blockSize & 3) != 0) {
      u8* pixelsBuffer8 = (u8*) pixelsBuffer;
      size_t destOffset = 0;
      size_t offset = 0;
      for(u32 y = 0; y < texData->th; y++) {
        pixels->Read(pixelsBuffer8 + destOffset, offset, blockSize);
        offset += blockSize;
        destOffset += stride;
      }
    }
    else {
      pixels->Read(pixelsBuffer, 0, pixelsSize);
    }
    return true;
  }

  return false;
}

#endif
