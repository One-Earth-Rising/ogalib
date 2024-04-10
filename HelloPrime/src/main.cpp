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

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>
#include <Prime/Graphics/Graphics.h>
#include <Prime/Font/Font.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Entry
////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char* const* argv) {
  // Init engine.
  Engine& engine = PxEngine;

  // Load font.
  refptr font = new Font();

  GetContent("data/Font/NotoSansCJKtc-Regular.otf", {{"size", 36.0f}}, [=](Content* content) {
    font->SetContent(content);
  });

  // Load shaders.
  refptr texProgram = DeviceProgram::Create("data/Shader/Tex/Tex.vsh", "data/Shader/Tex/Tex.fsh");

  ////////////////////////////////////////
  // Main Loop
  ////////////////////////////////////////

  Graphics& g = PxGraphics;

  g.ShowScreen();
  g.program = texProgram;

  engine.Start();
  while(engine.IsRunning()) {
    engine.StartFrame();

    g.ClearScreen();
    g.model.LoadTranslation(g.GetScreenW() * 0.5f, g.GetScreenH() * 0.5f);

    font->Draw("Hello, World!", AlignCenter);

    engine.EndFrame();
  }

  return 0;
}
