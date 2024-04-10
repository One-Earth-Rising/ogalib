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

#include <Prime/Graphics/Graphics.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>
#include <Prime/Graphics/DeviceProgram.h>
#include <stdarg.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PRIME_GRAPHICS_MAX_TEX_UNITS 64

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

Graphics* Graphics::instance = nullptr;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

Graphics& Graphics::GetInstance() {
  PxRequireInit;

  if(instance)
    return *instance;

  PrimeAssert(false, "Target did not create a Graphics instance.");
  instance = new Graphics();
  PrimeAssert(instance, "Could not create Graphics instance.");
  instance->Init();
  return *instance;
}

Graphics::Graphics():
maxTexW(0),
maxTexH(0),
maxTexUnits(0) {

}

Graphics::~Graphics() {

}

void Graphics::Init() {
  model.LoadIdentity();
  view.LoadIdentity();
  projection.LoadIdentity();
  viewport = Viewport(0.0f, 0.0f, 0.0f, 0.0f);
  depthMask = true;
  depthEnabled = true;
  clearScreenColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
  clearScreenDepth = 1.0;
  nearZ = 1.0f;
  farZ = 100.0f;
  for(size_t i = 0; i < PRIME_DEVICE_PROGRAM_CLIP_PLANE_COUNT; i++) {
    clipPlane[i] = Vec4();
    clipPlaneEnabled[i] = false;
  }

  program = nullptr;
}

void Graphics::Shutdown() {

}

void Graphics::ShowScreen(const GraphicsScreenConfig* config) {

}

f32 Graphics::GetScreenW() const {
  return 0;
}

f32 Graphics::GetScreenH() const {
  return 0;
}

f32 Graphics::MapWindowToScreenX(f32 x) const {
  return x;
}

f32 Graphics::MapWindowToScreenY(f32 y) const {
  f32 h = GetScreenH();
  return h - y;
}

void Graphics::RequestFullscreenToggle() {

}

bool Graphics::IsFullscreenToggleRequested() const {
  return false;
}

bool Graphics::IsFullscreenToggled() const {
  return false;
}

void Graphics::RequestVsyncToggle() {

}

bool Graphics::IsVsyncToggleRequested() const {
  return false;
}

bool Graphics::IsVsyncToggled() const {
  return false;
}

void Graphics::StartFrame() {
  projection.Push();
  LoadScreenOrtho();
}

void Graphics::EndFrame() {
  projection.Pop();
}

void Graphics::LoadScreenOrtho() {
  projection.LoadOrtho(0.0f, 0.0f, GetScreenW(), GetScreenH(), -1.0f, 1.0f);
}

void Graphics::ClearScreen() {

}

void Graphics::ClearColor() {

}

void Graphics::ClearDepth() {

}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, Tex* tex) {
  if(tex) {
    Tex* texList[] = {tex};
    Draw(ab, ib, texList, 1);
  }
  else {
    Draw(ab, ib, (Tex**) NULL, 0);
  }
}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, Tex* tex) {
  Tex* texList[] = {tex};
  Draw(ab, ib, start, count, texList, 1);
}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, Tex* tex, Tex* tex2, ...) {
  va_list ap;
  va_start(ap, tex2);

  Draw(ab, ib, 0, ib->GetSyncCount(), tex, tex2, ap);

  va_end(ap);
}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, Tex* tex, Tex* tex2, ...) {
  va_list ap;
  va_start(ap, tex2);

  Draw(ab, ib, start, count, tex, tex2, ap);

  va_end(ap);
}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, Tex* tex, Tex* tex2, va_list ap) {
  Tex* texList[PRIME_GRAPHICS_MAX_TEX_UNITS] = {tex, tex2};
  size_t texCount = 2;

  for(size_t i = texCount; i < PRIME_GRAPHICS_MAX_TEX_UNITS; i++) {
    Tex* currTex = va_arg(ap, Tex*);
    if(currTex) {
      texList[texCount++] = currTex;
    }
    else {
      break;
    }
  }

  Draw(ab, ib, 0, ib->GetSyncCount(), texList, texCount);
}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, Tex* tex, Tex* tex2, va_list ap) {
  Tex* texList[PRIME_GRAPHICS_MAX_TEX_UNITS] = {tex, tex2};
  size_t texCount = 2;

  for(size_t i = texCount; i < PRIME_GRAPHICS_MAX_TEX_UNITS; i++) {
    Tex* currTex = va_arg(ap, Tex*);
    if(currTex) {
      texList[texCount++] = currTex;
    }
    else {
      break;
    }
  }

  Draw(ab, ib, start, count, texList, texCount);
}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, Tex* const* texList, size_t texCount) {
  Draw(ab, ib, 0, ib->GetSyncCount(), texList, texCount);
}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, Tex* const* texList, size_t texCount) {
  TexChannelTuple tupleList[PRIME_GRAPHICS_MAX_TEX_UNITS];
  for(size_t i = 0; i < texCount; i++) {
    tupleList[i] = {texList[i]};
  }

  Draw(ab, ib, start, count, tupleList, texCount);
}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, TexChannelTuple& tuple) {
  Draw(ab, ib, (TexChannelTuple*) nullptr, 0);
}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, TexChannelTuple& tuple) {
  Draw(ab, ib, start, count, &tuple, 1);
}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, TexChannelTuple const* tupleList, size_t tupleCount) {
  Draw(ab, ib, 0, ib->GetSyncCount(), tupleList, tupleCount);
}

void Graphics::Draw(ArrayBuffer* ab, IndexBuffer* ib, size_t start, size_t count, TexChannelTuple const* texList, size_t texCount) {

}
