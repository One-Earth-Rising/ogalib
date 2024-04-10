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

#include <Prime/Graphics/opengl/OpenGLGraphics.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Types/Mat44.h>
#include <Prime/Engine.h>

#include <Prime/Graphics/opengl/OpenGLTex.h>
#include <Prime/Graphics/opengl/OpenGLArrayBuffer.h>
#include <Prime/Graphics/opengl/OpenGLIndexBuffer.h>
#include <Prime/Input/opengl/OpenGLKeyboard.h>
#include <Prime/Input/opengl/OpenGLTouch.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define ShaderLogSize (8 * 1024)

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

bool __PrimeOpenGLOutOfMemoryError;

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

static const GLenum OpenGLBufferPrimitiveTable[] = {
  GL_NONE,
  GL_TRIANGLES,
  GL_TRIANGLE_FAN,
  GL_POINTS,
};

static const GLenum OpenGLIndexBufferTypeTable[] = {
  GL_NONE,
  GL_UNSIGNED_BYTE,
  GL_UNSIGNED_SHORT,
  GL_UNSIGNED_INT,
};

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

static void OnKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void OnScrollCallback(GLFWwindow* window, double x, double y);

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

OpenGLGraphics& OpenGLGraphics::GetInstance() {
  PxRequireInit;

  if(instance) {
    PrimeAssert(dynamic_cast<OpenGLGraphics*>(instance), "Graphics instance is not a OpenGLGraphics instance.");
    return *static_cast<OpenGLGraphics*>(instance);
  }

  OpenGLGraphics* inst = new OpenGLGraphics();
  PrimeAssert(inst, "Could not create OpenGLGraphics instance.");
  instance = inst;
  inst->Init();
  return *static_cast<OpenGLGraphics*>(instance);
}

OpenGLGraphics::OpenGLGraphics():
screenWindow(nullptr),
currentTextureStacks(nullptr) {
  currentIBOId = GL_NONE;
  currentABOId = GL_NONE;
  currentProgramId = GL_NONE;
  currentClearScreenColor = clearScreenColor;
  currentClearScreenDepth = clearScreenDepth;
  currentViewport = viewport;
  currentDepthMask = depthMask;
  currentDepthEnabled = depthEnabled;
}

OpenGLGraphics::~OpenGLGraphics() {

}

void OpenGLGraphics::Init() {
  Graphics::Init();

  drawMatMVP.LoadIdentity();
  drawMatModel.LoadIdentity();
  drawMatView.LoadIdentity();
  drawMatVP.LoadIdentity();
  drawMatMV.LoadIdentity();

  // Init GLFW Graphics
  glfwInit();

  OpenGLTex::InitGlobal();
}

void OpenGLGraphics::Shutdown() {
  OpenGLTex::ShutdownGlobal();

  glfwDestroyWindow(screenWindow);
  glfwTerminate();

  PrimeSafeDeleteArray(currentTextureStacks);

  Graphics::Shutdown();
}

void OpenGLGraphics::ShowScreen(const GraphicsScreenConfig* config) {
  if(screenWindow)
    return;

  static const GraphicsScreenConfig DefaultConfig = {
    "Prime Engine Game",
    1600,
    900,
    true,
    1,
  };

  const GraphicsScreenConfig* useConfig = config;
  if(!config) {
    useConfig = &DefaultConfig;
  }

  if(useConfig->windowed) {
    screenWindow = glfwCreateWindow(useConfig->w, useConfig->h, useConfig->title, NULL, NULL);
  }
  else {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    screenWindow = glfwCreateWindow(mode->width, mode->height, useConfig->title, monitor, NULL);
  }
  PrimeAssert(screenWindow, "Could not create main window.");

  glfwMakeContextCurrent(screenWindow);
  glfwSwapInterval(useConfig->swapInterval);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

  // Get OpenGL info.
  GLint intValue;

  GLCMD(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &intValue));
  maxTexW = intValue;
  maxTexH = intValue;

  GLCMD(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &intValue));
  maxTexUnits = intValue;

  ResetRenderState();

  GLCMD(glEnable(GL_FRAMEBUFFER_SRGB));

  GLCMD(glClearColor(clearScreenColor.r, clearScreenColor.g, clearScreenColor.b, clearScreenColor.a));
  GLCMD(glClearDepth(clearScreenDepth));

  GLCMD(glDepthFunc(GL_LEQUAL));
  GLCMD(glDepthMask(GL_TRUE));
  GLCMD(glEnable(GL_DEPTH_TEST));

  GLCMD(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GLCMD(glEnable(GL_BLEND));

  glfwSetKeyCallback(screenWindow, OnKeyCallback);
  glfwSetScrollCallback(screenWindow, OnScrollCallback);
}

f32 OpenGLGraphics::GetScreenW() const {
  int w;
  int h;
  glfwGetWindowSize(screenWindow, &w, &h);
  return (f32) w;
}

f32 OpenGLGraphics::GetScreenH() const {
  int w;
  int h;
  glfwGetWindowSize(screenWindow, &w, &h);
  return (f32) h;
}

void OpenGLGraphics::RequestFullscreenToggle() {

}

bool OpenGLGraphics::IsFullscreenToggleRequested() const {
  return false;
}

bool OpenGLGraphics::IsFullscreenToggled() const {
  return false;
}

void OpenGLGraphics::RequestVsyncToggle() {

}

bool OpenGLGraphics::IsVsyncToggleRequested() const {
  return false;
}

bool OpenGLGraphics::IsVsyncToggled() const {
  return false;
}

void OpenGLGraphics::StartFrame() {
  if(!screenWindow)
    return;

  int w;
  int h;
  glfwGetWindowSize(screenWindow, &w, &h);

  viewport.Push() = Viewport(0.0f, 0.0f, (f32) w, (f32) h);

  Graphics::StartFrame();
}

void OpenGLGraphics::EndFrame() {
  Graphics::EndFrame();

  viewport.Pop();

  if(!screenWindow)
    return;

  glfwSwapBuffers(screenWindow);
  glfwPollEvents();

  if(glfwWindowShouldClose(screenWindow)) {
    PxEngine.Stop();
  }
}

void OpenGLGraphics::ClearScreen() {
  if(currentClearScreenColor != clearScreenColor) {
    currentClearScreenColor = clearScreenColor;
    GLCMD(glClearColor(currentClearScreenColor.r, currentClearScreenColor.g, currentClearScreenColor.b, currentClearScreenColor.a));
  }
  if(currentClearScreenDepth != clearScreenDepth) {
    currentClearScreenDepth = clearScreenDepth;
    GLCMD(glClearDepth(currentClearScreenDepth));
  }

  GLCMD(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void OpenGLGraphics::ClearColor() {
  if(currentClearScreenColor != clearScreenColor) {
    currentClearScreenColor = clearScreenColor;
    GLCMD(glClearColor(currentClearScreenColor.r, currentClearScreenColor.g, currentClearScreenColor.b, currentClearScreenColor.a));
  }

  GLCMD(glClear(GL_COLOR_BUFFER_BIT));
}

void OpenGLGraphics::ClearDepth() {
  if(currentClearScreenDepth != clearScreenDepth) {
    currentClearScreenDepth = clearScreenDepth;
    GLCMD(glClearDepth(currentClearScreenDepth));
  }

  GLCMD(glClear(GL_DEPTH_BUFFER_BIT));
}

void OpenGLGraphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, TexChannelTuple const* tupleList, size_t tupleCount) {
  if(!program)
    return;

  PushDrawTexChannelTupleList(tupleList, tupleCount);
  PushDrawArrayBuffer(ab);
  PushDrawIndexBuffer(ib);
  PushDrawProgram(program);
  PushDrawMatrices();

  LoadDrawViewport();
  LoadDrawDepth();

  DeviceProgram* deviceProgram = program;
  OpenGLProgram& programOpenGL = *static_cast<OpenGLProgram*>(deviceProgram);
  OpenGLProgram& prog = programOpenGL;

  for(size_t i = 0; i < maxTexUnits; i++) {
    GLint textureLoc = programOpenGL.GetTextureLoc(i);
    if(textureLoc != -1) {
      GLCMD(glUniform1i(textureLoc, (GLint) i));
    }
  }

  {
    static const std::string mvpStr("mvp");
    static const std::string modelStr("model");
    static const std::string viewStr("view");
    static const std::string vpStr("vp");
    static const std::string mvStr("mv");
    static const std::string normalMatStr("normalMat");
    static const std::string gposMatStr("gposMat");
    static const std::string clipPlaneStr[] = {
      "clipPlane0",
      "clipPlane1",
      "clipPlane2",
      "clipPlane3",
      "clipPlane4",
      "clipPlane5",
    };
    static const size_t clipPlaneStrCount = sizeof(clipPlaneStr) / sizeof(clipPlaneStr[0]);

    if(prog.HasVariableMVP())
      prog.SetVariable(mvpStr, drawMatMVP);

    if(prog.HasVariableModel())
      prog.SetVariable(modelStr, drawMatModel);

    if(prog.HasVariableView())
      prog.SetVariable(viewStr, drawMatView);

    if(prog.HasVariableVP())
      prog.SetVariable(vpStr, drawMatVP);

    if(prog.HasVariableMV())
      prog.SetVariable(mvStr, drawMatMV);

    if(prog.HasVariableNormalMat()) {
      Mat44 normalMat = drawMatMV;
      normalMat.Invert();
      normalMat.Transpose();

      prog.SetVariable(normalMatStr, normalMat);
    }

    if(prog.HasVariableGPosMat()) {
      Mat44 gposMat = drawMatMV;
      f32 normalizeRange = 1.0f / (farZ - nearZ);
      gposMat = Mat44().LoadScaling(normalizeRange, normalizeRange, normalizeRange) * gposMat;
      prog.SetVariable(gposMatStr, gposMat);
    }

    for(u32 i = 0; i < PRIME_DEVICE_PROGRAM_CLIP_PLANE_COUNT; i++) {
      bool enabled = clipPlaneEnabled[i];

      if(enabled && prog.HasVariableClipPlane(i)) {
        if(i < clipPlaneStrCount) {
          prog.SetVariable(clipPlaneStr[i], clipPlane[i]);
        }
        else {
          prog.SetVariable(string_printf("clipPlane%zu", i), clipPlane[i]);
        }
      }

      if(enabled) {
        GLCMD(glEnable(GL_CLIP_DISTANCE0 + i));
      }
      else {
        GLCMD(glDisable(GL_CLIP_DISTANCE0 + i));
      }
    }
  }

  prog.LoadVariablesToShaderStage();

  GLsizei vertexStride = (GLsizei) ab->GetItemSize();

  size_t attributeCount = programOpenGL.GetAttributeCount();
  for(size_t i = 0; i < attributeCount; i++) {
    const OpenGLProgramAttributeInfo* info = programOpenGL.GetAttributeInfo(i);
    if(info) {
      const ArrayBufferAttribute* attribute = ab->GetAttribute(info->name);
      if(attribute) {
        GLCMD(glEnableVertexAttribArray(info->loc));
        GLCMD(glVertexAttribPointer(info->loc, (GLsizei) (info->size / sizeof(f32)), GL_FLOAT, GL_FALSE, vertexStride, (const GLvoid*) attribute->GetOffset()));
      }
    }
  }

  GLCMD(glDrawElements(OpenGLBufferPrimitiveTable[ab->GetPrimitive()], (GLsizei) count, OpenGLIndexBufferTypeTable[ib->GetFormat()], (const GLvoid*) (ib->GetIndexSize() * start)));

  PopDrawMatrices();
  PopDrawProgram();
  PopDrawIndexBuffer();
  PopDrawArrayBuffer();
  PopDrawTexChannelTupleList();
}

GLFWwindow* OpenGLGraphics::GetOpenGLGLFWScreenWindow() const {
  return screenWindow;
}

void OpenGLGraphics::ResetRenderState() {
  PrimeSafeDeleteArray(currentTextureStacks);

  if(maxTexUnits > 0) {
    currentTextureStacks = new TypeStack<OpenGLGraphicsCurrentTexture>[maxTexUnits];
    for(size_t i = 0; i < maxTexUnits; i++) {
      auto& currentTextureStack = currentTextureStacks[i];
      OpenGLGraphicsCurrentTexture& currentTexture = currentTextureStack;
      currentTexture.tex = NULL;
      currentTexture.channel = TexChannelMain;
      currentTexture.enabled = false;
      currentTexture.hasAlpha = false;
    }
  }

  currentIBOId = 0;
  currentABOId = 0;
  currentProgramId = 0;
}

void OpenGLGraphics::PushDrawTexChannelTupleList(TexChannelTuple const* tupleList, size_t tupleCount) {
  size_t i = 0;

  if(tupleList && tupleCount > 0) {
    for(; i < tupleCount; i++) {
      const TexChannelTuple& tuple = tupleList[i];
      PushDrawTex(tuple.tex, i, tuple.channel);
    }
  }

  for(; i < maxTexUnits; i++) {
    PushDrawTex(static_cast<Tex*>(nullptr), i);
  }
}

void OpenGLGraphics::PushDrawTex(Tex* tex, size_t unit, TexChannel channel) {
  if(unit >= maxTexUnits)
    return;

  GLint unitGL = (GLint) unit;
  auto& currentTextureStack = currentTextureStacks[unit];
  currentTextureStack.Push();

  OpenGLGraphicsCurrentTexture& currentTexture = currentTextureStack;

  if(tex) {
    OpenGLTex& texOpenGL = *static_cast<OpenGLTex*>(tex);

    if(!texOpenGL.IsLoadedIntoVRAM())
      texOpenGL.LoadIntoVRAM();

    if(texOpenGL.IsLoadedIntoVRAM()) {
      currentTexture.enabled = true;

      if(tex != currentTexture.tex || channel != currentTexture.channel) {
        currentTexture.tex = tex;
        currentTexture.channel = channel;
        currentTexture.hasAlpha = texOpenGL.HasA();

        if(currentTexture.channel == TexChannelMain) {
          GLCMD(glActiveTexture(GL_TEXTURE0 + unitGL));
          GLCMD(glBindTexture(GL_TEXTURE_2D, texOpenGL.GetTextureId()));
          GLCMD(glActiveTexture(GL_TEXTURE0));
        }
        else if(currentTexture.channel == TexChannelDepth) {
          GLCMD(glActiveTexture(GL_TEXTURE0 + unitGL));
          GLCMD(glBindTexture(GL_TEXTURE_2D, texOpenGL.GetDepthTextureId()));
          GLCMD(glActiveTexture(GL_TEXTURE0));
        }
        else {
          currentTexture.enabled = false;
          GLCMD(glActiveTexture(GL_TEXTURE0 + unitGL));
          GLCMD(glBindTexture(GL_TEXTURE_2D, 0));
          GLCMD(glActiveTexture(GL_TEXTURE0));
        }
      }
    }
    else {
      currentTexture.enabled = false;
      GLCMD(glActiveTexture(GL_TEXTURE0 + unitGL));
      GLCMD(glBindTexture(GL_TEXTURE_2D, 0));
      GLCMD(glActiveTexture(GL_TEXTURE0));
    }
  }
  else {
    if(currentTexture.tex != nullptr) {
      GLCMD(glActiveTexture(GL_TEXTURE0 + unitGL));
      GLCMD(glBindTexture(GL_TEXTURE_2D, 0));
      GLCMD(glActiveTexture(GL_TEXTURE0));
      currentTexture.tex = nullptr;
    }

    currentTexture.channel = channel;
    currentTexture.hasAlpha = false;
    currentTexture.enabled = false;
  }
}

void OpenGLGraphics::PushDrawIndexBuffer(IndexBuffer* ib) {
  currentIBOId.Push();

  if(ib) {
    OpenGLIndexBuffer& ibOpenGL = *static_cast<OpenGLIndexBuffer*>(ib);

    if(ibOpenGL.IsDataModified())
      ibOpenGL.Sync();

    if(!ibOpenGL.IsLoadedIntoVRAM())
      ibOpenGL.LoadIntoVRAM();

    if(ibOpenGL.IsLoadedIntoVRAM()) {
      GLuint iboId = ibOpenGL.GetIBOId();
      if(iboId != currentIBOId) {
        GLCMD(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId));
        currentIBOId = iboId;
      }
    }
    else {
      if(currentIBOId != 0) {
        GLCMD(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        currentIBOId = 0;
      }
    }
  }
  else {
    if(currentIBOId != 0) {
      GLCMD(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
      currentIBOId = 0;
    }
  }
}

void OpenGLGraphics::PushDrawArrayBuffer(ArrayBuffer* ab) {
  currentABOId.Push();

  if(ab) {
    OpenGLArrayBuffer& abOpenGL = *static_cast<OpenGLArrayBuffer*>(ab);

    if(abOpenGL.IsDataModified())
      abOpenGL.Sync();

    if(!abOpenGL.IsLoadedIntoVRAM())
      abOpenGL.LoadIntoVRAM();

    if(abOpenGL.IsLoadedIntoVRAM()) {
      GLuint aboId = abOpenGL.GetABOId();
      if(aboId != currentABOId) {
        GLCMD(glBindBuffer(GL_ARRAY_BUFFER, aboId));
        currentABOId = aboId;
      }
    }
    else {
      if(currentABOId != 0) {
        GLCMD(glBindBuffer(GL_ARRAY_BUFFER, 0));
        currentABOId = 0;
      }
    }
  }
  else {
    if(currentABOId != 0) {
      GLCMD(glBindBuffer(GL_ARRAY_BUFFER, 0));
      currentABOId = 0;
    }
  }
}

void OpenGLGraphics::PushDrawProgram(DeviceProgram* deviceProgram) {
  currentProgramId.Push();

  if(program) {
    OpenGLProgram& deviceProgramOpenGL = *static_cast<OpenGLProgram*>(deviceProgram);

    if(!deviceProgramOpenGL.IsLoadedIntoVRAM())
      deviceProgramOpenGL.LoadIntoVRAM();

    if(deviceProgramOpenGL.IsLoadedIntoVRAM()) {
      GLuint programId = deviceProgramOpenGL.GetProgramId();
      if(programId != currentProgramId) {
        GLCMD(glUseProgram(programId));
        currentProgramId = programId;
      }
    }
    else {
      if(currentProgramId != 0) {
        GLCMD(glUseProgram(0));
        currentProgramId = 0;
      }
    }
  }
  else {
    if(currentProgramId != 0) {
      GLCMD(glUseProgram(0));
      currentProgramId = 0;
    }
  }
}

void OpenGLGraphics::PushDrawMatrices() {
  drawMatVP.Push().LoadIdentity();

  drawMatView.Push() = view;

  drawMatVP.Multiply(projection * drawMatView);

  drawMatModel.Push() = model;

  drawMatMV.Push() = drawMatView * drawMatModel;
  drawMatMVP.Push() = drawMatVP * drawMatModel;
}

void OpenGLGraphics::PopDrawTexChannelTupleList() {
  for(size_t i = 0; i < maxTexUnits; i++) {
    PopDrawTex(i);
  }
}

void OpenGLGraphics::PopDrawTex(size_t unit) {
  if(unit >= maxTexUnits)
    return;

  GLint unitGL = (GLint) unit;
  auto& currentTextureStack = currentTextureStacks[unit];
  OpenGLGraphicsCurrentTexture prevTexture = currentTextureStack;
  currentTextureStack.Pop();

  OpenGLGraphicsCurrentTexture& currentTexture = currentTextureStack;

  if(currentTexture.tex) {
    OpenGLTex& texOpenGL = *static_cast<OpenGLTex*>((Tex*) currentTexture.tex);

    if(!texOpenGL.IsLoadedIntoVRAM())
      texOpenGL.LoadIntoVRAM();

    if(texOpenGL.IsLoadedIntoVRAM()) {
      if(currentTexture.tex != prevTexture.tex || currentTexture.channel != prevTexture.channel) {
        if(prevTexture.channel == TexChannelMain) {
          GLCMD(glActiveTexture(GL_TEXTURE0 + unitGL));
          GLCMD(glBindTexture(GL_TEXTURE_2D, texOpenGL.GetTextureId()));
          GLCMD(glActiveTexture(GL_TEXTURE0));
        }
        else if(prevTexture.channel == TexChannelDepth) {
          GLCMD(glActiveTexture(GL_TEXTURE0 + unitGL));
          GLCMD(glBindTexture(GL_TEXTURE_2D, texOpenGL.GetDepthTextureId()));
          GLCMD(glActiveTexture(GL_TEXTURE0));
        }
        else {
          currentTexture.enabled = false;
          GLCMD(glActiveTexture(GL_TEXTURE0 + unitGL));
          GLCMD(glBindTexture(GL_TEXTURE_2D, 0));
          GLCMD(glActiveTexture(GL_TEXTURE0));
        }
      }
    }
    else {
      currentTexture.enabled = false;
      GLCMD(glActiveTexture(GL_TEXTURE0 + unitGL));
      GLCMD(glBindTexture(GL_TEXTURE_2D, 0));
      GLCMD(glActiveTexture(GL_TEXTURE0));
    }
  }
  else {
    if(prevTexture.tex != nullptr) {
      GLCMD(glActiveTexture(GL_TEXTURE0 + unitGL));
      GLCMD(glBindTexture(GL_TEXTURE_2D, 0));
      GLCMD(glActiveTexture(GL_TEXTURE0));
    }
  }
}

void OpenGLGraphics::PopDrawIndexBuffer() {
  currentIBOId.Pop();
}

void OpenGLGraphics::PopDrawArrayBuffer() {
  currentABOId.Pop();
}

void OpenGLGraphics::PopDrawProgram() {
  currentProgramId.Pop();
}

void OpenGLGraphics::PopDrawMatrices() {
  drawMatMVP.Pop();
  drawMatModel.Pop();
  drawMatView.Pop();
  drawMatVP.Pop();
  drawMatMV.Pop();
}

void OpenGLGraphics::LoadDrawViewport() {
  if(viewport.x != currentViewport.x || viewport.y != currentViewport.y || viewport.w != currentViewport.w || viewport.h != currentViewport.h) {
    currentViewport = viewport;
    GLCMD(glViewport((GLint) currentViewport.x, (GLint) currentViewport.y, (GLsizei) currentViewport.w, (GLsizei) currentViewport.h));
  }
}

void OpenGLGraphics::LoadDrawDepth() {
  if(depthMask != currentDepthMask) {
    currentDepthMask = depthMask;
    if(currentDepthMask) {
      GLCMD(glDepthMask(GL_TRUE));
    }
    else {
      GLCMD(glDepthMask(GL_FALSE));
    }
  }

  if(depthEnabled != currentDepthEnabled) {
    currentDepthEnabled = depthEnabled;
    if(currentDepthEnabled) {
      GLCMD(glEnable(GL_DEPTH_TEST));
    }
    else {
      GLCMD(glDisable(GL_DEPTH_TEST));
    }
  }
}

void OnKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  PxOpenGLKeyboard.OnKey(window, key, scancode, action, mods);
}

void OnScrollCallback(GLFWwindow* window, double x, double y) {
  PxOpenGLTouch.OnScroll(window, x, y);
}

#if defined(_DEBUG)
void Prime::AssertOpenGLShaderCompileCore(GLuint shaderId) {
  static GLchar shaderInfoLog[ShaderLogSize + 1];
  GLint status;

  GLCMD(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status));

  GLCMD(glGetShaderInfoLog(shaderId, ShaderLogSize, NULL, shaderInfoLog));
  if(shaderInfoLog[0]) {
    dbgprintf("OpenGL shader compile info available:\n");
    dbgprintf("%s", shaderInfoLog);
    dbgprintf("\n");
  }
  if(status != GL_TRUE) {
    PrimeAssert(false, "Halting due to shader compile error.");
  }
}

void Prime::AssertOpenGLProgramLinkCore(GLuint programId) {
  static GLchar shaderInfoLog[ShaderLogSize + 1];
  GLint status;

  GLCMD(glGetProgramiv(programId, GL_LINK_STATUS, &status));

  GLCMD(glGetProgramInfoLog(programId, ShaderLogSize, NULL, shaderInfoLog));
  if(shaderInfoLog[0]) {
    dbgprintf("OpenGL program link info available:\n");
    dbgprintf("%s", shaderInfoLog);
    dbgprintf("\n");
  }
  if(status != GL_TRUE) {
    PrimeAssert(false, "Halting due to program link error.");
  }
}
#endif

#endif
