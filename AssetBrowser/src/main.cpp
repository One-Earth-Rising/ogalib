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
#include <Prime/Input/Keyboard.h>
#include <Prime/Input/Touch.h>
#include <Prime/Font/Font.h>
#include <Prime/Asset/Asset.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define ApiRoot "https://ogahub.com/API"

#define FirstAssetId 1

#define TouchViewAzimuthStart 0.0f
#define TouchViewAltitudeStart 0.0f
#define TouchViewSensitivity 0.2f
#define TouchViewZoomStart 2.0f
#define TouchViewZoomSensitivity 0.1f
#define TouchViewZoomSensitivityFast 0.5f
#define TouchViewZoomMin 0.5f
#define TouchViewZoomMax 100.0f

#define ButtonHLineScale 1.3f

#define DataManifestAssetViewportH 100.0f
#define DataManifestAssetViewportHSpacePct 0.05f
#define DataManifestAssetViewportScrollWheelFast (DataManifestAssetViewportH * 1.0f)
#define DataManifestAssetViewportScrollWheel (DataManifestAssetViewportH * 0.3f)
#define DataManifestAssetFontScale 0.55f

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

typedef struct _ButtonRect {
  f32 x, y;
  f32 w, h;

  struct _ButtonRect(): x(0.0f), y(0.0f), w(0.0f), h(0.0f) {}

  bool Contains(f32 px, f32 py) const {
    return px >= x && px <= x + w && py >= y && py <= y + h;
  }

} ButtonRect;

void DrawRect(DeviceProgram* program, ArrayBuffer* ab, IndexBuffer* ib, f32 x, f32 y, f32 w, f32 h, f32 red, f32 green, f32 blue, f32 alpha = 1.0f) {
  Graphics& g = PxGraphics;
  program->SetVariable("colorScale", Vec4(red, green, blue, alpha));

  g.program.Push() = program;

  g.model.Push();
  g.model.Translate(x, y);
  g.model.Scale(w, h);
  
  g.Draw(ab, ib);

  g.model.Pop();

  g.program.Pop();
}

ButtonRect DrawButton(DeviceProgram* rectProgram, DeviceProgram* texProgram, ArrayBuffer* ab, IndexBuffer* ib, Font* font, const std::string& text, f32 x, f32 y, f32 w, f32 h, f32 red, f32 green, f32 blue, f32 alpha = 1.0f) {
  Graphics& g = PxGraphics;
  ButtonRect dim;

  g.model.Push();
  g.model.Translate(x, y);
  g.model.Scale(w, h);

  Vec2 pos = g.model * Vec2(0.0f, 0.0f);
  dim.x = pos.x;
  dim.y = pos.y;
  dim.w = w;
  dim.h = h;

  g.model.Pop();

  DrawRect(rectProgram, ab, ib, x, y, w, h, red, green, blue, alpha);

  g.program.Push() = texProgram;
  g.model.Push().Translate(x + (w - font->GetStringW(text)) * 0.5f, y + (h - font->GetLineH()) * 0.5f);
  
  font->Draw(text);

  g.model.Pop();
  g.program.Pop();

  return dim;
}

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

class AssetHistory {
public:

  size_t& assetId;
  refptr<Asset>& asset;

  Stack<size_t> assetIds;
  size_t number;

  AssetHistory(size_t& assetId, refptr<Asset>& asset): assetId(assetId), asset(asset), number(0) {
    Push(assetId);
  }

  void Push(size_t id) {
    while(assetIds.GetCount() > number) {
      assetIds.Pop();
    }

    assetIds.Push(id);
    number = assetIds.GetCount();

    assetId = id;
    asset->Load(assetId);
  }

  void Back() {
    if(number > 1) {
      assetId = assetIds[--number - 1];
      asset->Load(assetId);
    }
  }

  void Forward() {
    if(number < assetIds.GetCount()) {
      assetId = assetIds[number++];
      asset->Load(assetId);
    }
  }
};

class AssetSortItem {
public:

  refptr<Asset> asset;

  AssetSortItem(refptr<Asset> asset = nullptr): asset(asset) {}
  AssetSortItem(const AssetSortItem& other) {asset = other.asset;}
  AssetSortItem& operator=(const AssetSortItem& other) {asset = other.asset; return *this;}

  bool operator<(const AssetSortItem& other) const {
    if(asset && other.asset) {
      u32 h1 = 0;
      u32 h2 = 0;

      if(auto it = asset->GetInfo().find("h"))
        h1 = it.GetUint();

      if(auto it = asset->GetInfo().find("height"))
        h1 = it.GetUint();

      if(auto it = other.asset->GetInfo().find("h"))
        h2 = it.GetUint();

      if(auto it = other.asset->GetInfo().find("height"))
        h2 = it.GetUint();

      if(h1 == h2) {
        std::string name1;
        std::string name2;

        if(auto it = asset->GetInfo().find("name"))
          name1 = it.GetString();

        if(auto it = other.asset->GetInfo().find("name"))
          name2 = it.GetString();

        if(name1 != name2) {
          return name1 < name2;
        }
        else {
          size_t id1 = 0;
          size_t id2 = 0;

          if(auto it = asset->GetInfo().find("id"))
            id1 = it.GetUint();

          if(auto it = other.asset->GetInfo().find("id"))
            id2 = it.GetUint();

          if(id1 != id2) {
            return id1 < id2;
          }
        }
      }
      else {
        return h1 < h2;
      }
    }

    return (intptr_t) asset < (intptr_t) other.asset;
  }
};

////////////////////////////////////////////////////////////////////////////////
// Entry
////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char* const* argv) {
#if defined(_DEBUG) && defined(PrimeTargetWindows)
  // If debugging in Windows, enable memory leak detection at end of program execution.
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  // Init engine.
  Engine& engine = PxEngine;

  // Load font.
  refptr font = new Font();

  GetContent("data/Font/NotoSansCJKtc-Regular.otf", [=](Content* content) {
    font->SetContent(content);
  });

  // Load shaders.
  refptr rectProgram = DeviceProgram::Create("data/Shader/Rect/Rect.vsh", "data/Shader/Rect/Rect.fsh");
  refptr texProgram = DeviceProgram::Create("data/Shader/Tex/Tex.vsh", "data/Shader/Tex/Tex.fsh");
  refptr skeletonProgram = DeviceProgram::Create("data/Shader/Skeleton/Skeleton.vsh", "data/Shader/Skeleton/Skeleton.fsh");
  refptr modelProgram = DeviceProgram::Create("data/Shader/Model/Model.vsh", "data/Shader/Model/Model.fsh");
  refptr modelAnimProgram = DeviceProgram::Create("data/Shader/Model/ModelAnim.vsh", "data/Shader/Model/ModelAnim.fsh");

  // Rectangle buffer for drawing.
  struct RectVertex {
    f32 x, y;
  };

  RectVertex rectVertices[] = {
    {0.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 1.0f},
    {1.0f, 0.0f},
  };

  u8 rectIndices[] = {
    0, 1, 2,
    0, 2, 3,
  };

  refptr rectAB = ArrayBuffer::Create(sizeof(RectVertex), rectVertices, sizeof(rectVertices) / sizeof(RectVertex));
  rectAB->LoadAttribute("vPos", sizeof(f32) * 2);

  refptr rectIB = IndexBuffer::Create(IndexFormatSize8, rectIndices, sizeof(rectIndices) / sizeof(u8));
  
  // Touch/mouse input.
  bool lastTouchButtonHeld = false;
  f32 touchPressX = 0.0f;
  f32 touchPressY = 0.0f;
  f32 touchViewAzimuth = TouchViewAzimuthStart;
  f32 touchViewAltitude = TouchViewAltitudeStart;
  f32 touchViewAzimuthPressed = 0.0f;
  f32 touchViewAltitudePressed = 0.0f;
  f32 touchViewZoom = TouchViewZoomStart;

  // Asset data.
  refptr asset = new Asset();
  asset->SetAPIRoot(ApiRoot);
  asset->SetTexProgram(texProgram);
  asset->SetSkeletonProgram(skeletonProgram);
  asset->SetModelProgram(modelProgram);
  asset->SetModelAnimProgram(modelAnimProgram);
  asset->SetAcceptedTextureFormats({"bc"});

  size_t assetId = FirstAssetId;
  AssetHistory assetHistory(assetId, asset);

  bool assetTextureFilteringEnabled = true;
  bool assetActionPlaying = true;

  // Asset ID input.
  bool inputtingAssetId = false;
  char inputtingAssetIdBuffer[64];
  u32 inputtingAssetIdBufferIndex = 0;
  f32 inputtingAssetIdTime = 0.0f;

  // Asset data manifest.
  f32 dataManifestScroll = 0.0f;

  // App loop.
  Graphics& g = PxGraphics;
  Keyboard& kb = PxKeyboard;
  Touch& touch = PxTouch;

  g.ShowScreen();
  g.clearScreenColor = Color(0.0f, 0.0f, 0.1f);

  engine.Start();
  while(engine.IsRunning()) {
    f32 dt = engine.StartFrame();
    f32 screenW = g.GetScreenW();
    f32 screenH = g.GetScreenH();

    g.ClearScreen();

    // Process input.
    if(kb.IsKeyPressed('[')) {
      asset->SetNextAction();
    }
    else if(kb.IsKeyPressed(']')) {
      asset->SetPrevAction();
    }
    else if(kb.IsKeyPressed(',')) {
      if(assetId > 1) {
        assetHistory.Push(--assetId);
      }
    }
    else if(kb.IsKeyPressed('.')) {
      assetHistory.Push(++assetId);
    }
    else if(kb.IsKeyPressed('F')) {
      assetTextureFilteringEnabled = !assetTextureFilteringEnabled;
      asset->SetTextureFilteringEnabled(assetTextureFilteringEnabled);
    }
    else if(kb.IsKeyPressed(' ')) {
      if(asset->Is2D()) {
        touchViewAzimuth = 0.0f;
        touchViewAltitude = 0.0f;
      }
      else {
        touchViewAzimuth = TouchViewAzimuthStart;
        touchViewAltitude = TouchViewAltitudeStart;
      }
      touchViewZoom = TouchViewZoomStart;
      dataManifestScroll = 0.0f;
    }
    else {
      char digitPressed = '\0';
      
      for(u32 i = 0; i < 10; i++) {
        char c = '0' + i;
        if(kb.IsKeyPressed(c)) {
          digitPressed = c;
          break;
        }
      }

      if(inputtingAssetId) {
        inputtingAssetIdTime += dt;

        if(digitPressed) {
          if(inputtingAssetIdBufferIndex < sizeof(inputtingAssetIdBuffer) - 1) {
            inputtingAssetIdBuffer[inputtingAssetIdBufferIndex++] = digitPressed;
            inputtingAssetIdTime = 0.0f;
            assetId = atoll(inputtingAssetIdBuffer);
            assetHistory.Push(assetId);
          }
        }
        else {
          if(kb.IsKeyPressed(KeyEscape) || kb.IsKeyPressed(KeyEnter) || kb.IsKeyPressed(KeyNumPadEnter) || inputtingAssetIdTime >= 2.0f) {
            inputtingAssetId = false;
          }
          else if(kb.IsKeyPressed(KeyBackspace)) {
            if(inputtingAssetIdBufferIndex > 0) {
              inputtingAssetIdBuffer[--inputtingAssetIdBufferIndex] = 0;
              inputtingAssetIdTime = 0.0f;
              assetId = atoll(inputtingAssetIdBuffer);
              assetHistory.Push(assetId);
            }
          }
        }
      }

      if(digitPressed) {
        if(!inputtingAssetId) {
          inputtingAssetId = true;
          inputtingAssetIdTime = 0.0f;
          inputtingAssetIdBufferIndex = 1;
          memset(inputtingAssetIdBuffer, 0, sizeof(inputtingAssetIdBuffer));
          inputtingAssetIdBuffer[0] = digitPressed;
          assetId = atoll(inputtingAssetIdBuffer);
          assetHistory.Push(assetId);
        }
      }
    }

    f32 cursorX, cursorY;
    touch.GetMainCursorPos(cursorX, cursorY);
    bool touchButtonHeld = touch.IsButtonHeld(TouchButton1);
    bool touchButtonPressed = !lastTouchButtonHeld && touchButtonHeld;
    lastTouchButtonHeld = touchButtonHeld;

    if(touchButtonPressed) {
      touchPressX = cursorX;
      touchPressY = cursorY;
      touchViewAzimuthPressed = touchViewAzimuth;
      touchViewAltitudePressed = touchViewAltitude;
    }
    else if(touchButtonHeld) {
      f32 dx = cursorX - touchPressX;
      f32 dy = cursorY - touchPressY;
      touchViewAzimuth = touchViewAzimuthPressed + dx * TouchViewSensitivity;
      touchViewAltitude = touchViewAltitudePressed + dy * TouchViewSensitivity;
    }

    if(cursorX >= screenW - DataManifestAssetViewportH * screenW / screenH) {
      // Scroll the data manifest list.
      f32 amount;
      if(kb.IsKeyHeld(KeyLShift) || kb.IsKeyHeld(KeyRShift))
        amount = DataManifestAssetViewportScrollWheelFast;
      else
        amount = DataManifestAssetViewportScrollWheel;

      if(touch.IsActionPressed(TouchActionScrollDown)) {
        dataManifestScroll += amount;
      }
      else if(touch.IsActionPressed(TouchActionScrollUp)) {
        dataManifestScroll -= amount;
      }
    }
    else {
      // Perform standard asset zooming.
      f32 amount;
      if(kb.IsKeyHeld(KeyLShift) || kb.IsKeyHeld(KeyRShift))
        amount = TouchViewZoomSensitivityFast;
      else
        amount = TouchViewZoomSensitivity;

      if(touch.IsActionPressed(TouchActionScrollDown)) {
        touchViewZoom += amount;
        if(touchViewZoom > TouchViewZoomMax) {
          touchViewZoom = TouchViewZoomMax;
        }
      }
      else if(touch.IsActionPressed(TouchActionScrollUp)) {
        touchViewZoom -= amount;
        if(touchViewZoom < TouchViewZoomMin) {
          touchViewZoom = TouchViewZoomMin;
        }
      }
    }

    if(touch.IsButtonPressed(TouchButton4)) {
      assetHistory.Back();
    }

    if(touch.IsButtonPressed(TouchButton5)) {
      assetHistory.Forward();
    }

    // Process asset.
    if(assetActionPlaying)
      asset->Calc(dt);

    f32 assetUniformSize = asset->GetUniformSize();
    if(assetUniformSize > 0.0f) {
      Vec2 viewOffset = asset->GetViewOffset();

      g.projection.Push().LoadPerspective(60.0f, g.GetScreenW() / g.GetScreenH(), assetUniformSize * 0.1f, assetUniformSize * 20.0f);
      g.view.Push()
        .LoadTranslation(0.0f, 0.0f, -touchViewZoom * assetUniformSize)
        .Rotate(touchViewAltitude, 1.0f, 0.0f, 0.0f)
        .Rotate(touchViewAzimuth, 0.0f, 1.0f, 0.0f);
      g.model.Push().LoadTranslation(-viewOffset.x, -viewOffset.y);

      asset->Draw();

      g.model.Pop();
      g.view.Pop();
      g.projection.Pop();
    }

    g.ClearDepth();

    ////////////////////////////////////////
    // Asset Info Overlay
    ////////////////////////////////////////

    g.program.Push() = texProgram;
    g.projection.Push() = Mat44().LoadOrtho(0.0f, 0.0f, screenW, screenH, -1.0f, 1.0f);

    const f32 pad = max(screenW, screenH) * 0.01f;

    f32 lineH = font->GetLineH();
    f32 buttonH = lineH * ButtonHLineScale;
    f32 buttonW = 0.0f;
    std::string text;
    f32 textW;

    ////////////////////////////////////////
    // Left Column
    ////////////////////////////////////////

    g.model.Push().LoadTranslation(pad, screenH - pad);

    // Draw asset info.
    g.model.Translate(0.0f, -lineH);
    if(inputtingAssetId) {
      if(assetId) {
        font->Draw(string_printf("Asset ID: %zu_", assetId));
      }
      else {
        font->Draw(string_printf("Asset ID: _"));
      }
    }
    else {
      if(assetId) {
        font->Draw(string_printf("Asset ID: %zu", assetId));
      }
      else {
        font->Draw(string_printf("Asset ID:"));
      }
    }

    g.model.Translate(0.0f, -lineH);
    font->Draw(string_printf("URI: %s", asset->GetURI().c_str()));

    if(auto itName = asset->GetInfo().find("name")) {
      text = itName.GetString();
      g.model.Translate(0.0f, -lineH);
      font->Draw(string_printf("Name: %s", text.c_str()));
    }

    g.model.Translate(0.0f, -lineH);
    font->Draw(string_printf("Format: %s", asset->GetFormat().c_str()));

    // Draw asset buttons.
    g.model.Translate(0.0f, -buttonH - pad);
    g.model.Push();

    buttonW = buttonH * 3.0f;
    ButtonRect prevAssetButton = DrawButton(rectProgram, texProgram, rectAB, rectIB, font, "Previous", 0.0f, 0.0f, buttonW, buttonH, 0.0f, 0.2f, 0.2f);
    g.model.Translate(buttonW + pad, 0.0f);

    buttonW = buttonH * 3.0f;
    ButtonRect nextAssetButton = DrawButton(rectProgram, texProgram, rectAB, rectIB, font, "Next", 0.0f, 0.0f, buttonW, buttonH, 0.0f, 0.2f, 0.2f);
    g.model.Translate(buttonW + pad, 0.0f);

    g.model.Pop();

    // Draw asset details.
    g.model.Translate(0.0f, -buttonH);
    font->Draw("Action:");

    g.model.Translate(0.0f, -buttonH);
    font->Draw(string_printf("Count: %zu", asset->GetActionCount()));

    size_t actionIndex = asset->GetActionIndex();
    if(actionIndex != PrimeNotFound) {
      g.model.Translate(0.0f, -lineH);
      font->Draw(string_printf("Index: %zu", actionIndex));
    }

    g.model.Translate(0.0f, -lineH);
    font->Draw(string_printf("Name: %s", asset->GetActionName().c_str()));

    g.model.Translate(0.0f, -lineH);
    font->Draw(string_printf("Length: %.2f sec", asset->GetActionLen()));

    g.model.Translate(0.0f, -lineH);
    font->Draw(string_printf("Playback:"));

    // Draw playback buttons.
    g.model.Translate(0.0f, -buttonH - pad);
    g.model.Push();

    buttonW = buttonH;
    ButtonRect prevActionButton = DrawButton(rectProgram, texProgram, rectAB, rectIB, font, "<<", 0.0f, 0.0f, buttonW, buttonH, 0.0f, 0.2f, 0.2f);
    g.model.Translate(buttonW + pad, 0.0f);

    buttonW = buttonH;
    ButtonRect restartActionButton = DrawButton(rectProgram, texProgram, rectAB, rectIB, font, "|<", 0.0f, 0.0f, buttonW, buttonH, 0.1f, 0.1f, 0.2f);
    g.model.Translate(buttonW + pad, 0.0f);

    buttonW = buttonH * 2.0f;
    ButtonRect playActionButton;
    if(assetActionPlaying)
      playActionButton = DrawButton(rectProgram, texProgram, rectAB, rectIB, font, "Stop", 0.0f, 0.0f, buttonW, buttonH, 0.2f, 0.0f, 0.0f);
    else
      playActionButton = DrawButton(rectProgram, texProgram, rectAB, rectIB, font, "Play", 0.0f, 0.0f, buttonW, buttonH, 0.0f, 0.2f, 0.0f);
    g.model.Translate(buttonW + pad, 0.0f);

    buttonW = buttonH;
    ButtonRect nextActionButton = DrawButton(rectProgram, texProgram, rectAB, rectIB, font, ">>", 0.0f, 0.0f, buttonW, buttonH, 0.0f, 0.2f, 0.2f);
    g.model.Translate(buttonW + pad, 0.0f);

    g.model.Pop();

    g.model.Pop();

    ////////////////////////////////////////
    // Right Column
    ////////////////////////////////////////

    f32 dmViewportH = DataManifestAssetViewportH;
    f32 dmViewportSpacing = dmViewportH * DataManifestAssetViewportHSpacePct;
    f32 dmViewportW = dmViewportH * screenW / screenH;
    if(dmViewportW < 1.0f)
      dmViewportW = 1.0f;
    if(dmViewportH < 1.0f)
      dmViewportH = 1.0f;

    g.model.Push().LoadTranslation(screenW - pad, screenH - pad);
    Vec2 dmViewportPos = g.model * Vec2(0.0f, 0.0f);
    g.model.Translate(-dmViewportW - pad, 0.0f);

    // Draw data manifest.
    auto& dataManifest = asset->GetDataManifest();
    auto& dataManifestAssets = asset->GetDataManifestAssets();

    g.model.Translate(0.0f, -lineH);
    text = "Data Manifest";
    textW = font->GetStringW(text);
    g.model.Push().Translate(-textW, 0.0f);
    font->Draw(text);
    g.model.Pop();

    g.model.Translate(0.0f, -lineH);
    text = string_printf("Item Count: %zu", dataManifest.size());
    textW = font->GetStringW(text);
    g.model.Push().Translate(-textW, 0.0f);
    font->Draw(text);
    g.model.Pop();

    // Draw data manifest assets.
    g.model.Push();

    Stack<AssetSortItem> dataManifestAssetsSorted;
    for(auto dmAsset: dataManifestAssets) {
      dataManifestAssetsSorted.Add(dmAsset);
    }
    dataManifestAssetsSorted.Sort();

    for(auto& dmAssetSortItem: dataManifestAssetsSorted) {
      auto dmAsset = dmAssetSortItem.asset;

      f32 dmAssetUniformSize = dmAsset->GetUniformSize();

      g.viewport.Push() = Viewport(dmViewportPos.x - dmViewportW, dmViewportPos.y - dmViewportH + dataManifestScroll, dmViewportW, dmViewportH);

      if(dmAssetUniformSize > 0.0f) {
        Vec2 dmViewOffset = dmAsset->GetViewOffset();

        g.projection.Push().LoadPerspective(60.0f, dmViewportW / dmViewportH, dmAssetUniformSize * 0.1f, dmAssetUniformSize * 20.0f);
        g.view.Push().LoadTranslation(-dmViewOffset.x, -dmViewOffset.y, -1.5f * dmAssetUniformSize);
        g.model.Push().LoadIdentity();

        dmAsset->Draw();

        g.model.Pop();
        g.view.Pop();
        g.projection.Pop();
      }

      g.projection.Push().LoadOrtho(0.0f, 0.0f, dmViewportW, dmViewportH, -1.0f, 1.0f);
      g.view.Push().LoadIdentity();

      if(dmAssetUniformSize == 0.0f) {
        text = string_printf("(%s file)", dmAsset->GetFormat().c_str());
        g.model.Push().LoadIdentity();
        DrawButton(rectProgram, texProgram, rectAB, rectIB, font, text, 0.0f, 0.0f, dmViewportW, dmViewportH, 0.0f, 0.0f, 0.05f);
        g.model.Pop();
      }

      if(auto itName = dmAsset->GetInfo().find("name")) {
        text = itName.GetString();
      }
      else {
        text = dmAsset->GetFormat();
      }
      textW = font->GetStringW(text) * DataManifestAssetFontScale;
      g.model.Push().LoadTranslation((dmViewportW - textW) * 0.5f, 0.0f).Scale(DataManifestAssetFontScale, DataManifestAssetFontScale);
      font->Draw(text);
      g.model.Pop();

      g.view.Pop();
      g.projection.Pop();

      g.viewport.Pop();

      dmViewportPos.y -= dmViewportH + dmViewportSpacing;
    }
    g.model.Pop();

    ////////////////////////////////////////
    // End Right Column
    ////////////////////////////////////////

    g.model.Pop();

    ////////////////////////////////////////
    // Input Help
    ////////////////////////////////////////

    g.model.Push().LoadTranslation(pad, pad + lineH * 4);

    g.model.Translate(0.0f, -lineH);
    text = "Spacebar: Reset camera";
    font->Draw(text);

    g.model.Translate(0.0f, -lineH);
    text = string_printf("Scroll Wheel: Zoom in/out", dataManifest.size());
    font->Draw(text);

    g.model.Translate(0.0f, -lineH);
    text = string_printf("Shift: Zoom faster");
    font->Draw(text);

    g.model.Translate(0.0f, -lineH);
    text = string_printf("F: Toggle texture filtering");
    font->Draw(text);

    g.model.Pop();

    ////////////////////////////////////////
    // End Asset Info Overlay
    ////////////////////////////////////////

    g.projection.Pop();
    g.program.Pop();

    ////////////////////////////////////////
    // Button Input
    ////////////////////////////////////////

    if(cursorX >= screenW - DataManifestAssetViewportH * screenW / screenH) {
      if(touchButtonPressed) {
        f32 itemH = (1.0f + DataManifestAssetViewportHSpacePct) * DataManifestAssetViewportH;
        s32 dmAssetIndex = (s32) (cursorY + dataManifestScroll - pad) / (s32) itemH;
        s32 dmAssetCount = (s32) dataManifestAssetsSorted.GetCount();

        if(dmAssetIndex >= 0 && dmAssetIndex < dmAssetCount) {
          auto dmAsset = dataManifestAssetsSorted[dmAssetIndex].asset;
          size_t dmAssetId = PrimeNotFound;
            
          if(auto itId = dmAsset->GetInfo().find("id")) {
            dmAssetId = itId.GetSizeT();
          }
            
          if(dmAssetId != PrimeNotFound) {
            assetHistory.Push(dmAssetId);
          }
        }
      }
    }
    else {
      if(touchButtonPressed) {
        f32 touchX, touchY;
        touch.GetMainCursorPos(touchX, touchY);
        touchX = g.MapWindowToScreenX(touchX);
        touchY = g.MapWindowToScreenY(touchY);

        if(prevAssetButton.Contains(touchX, touchY)) {
          if(assetId > 1) {
            assetHistory.Push(--assetId);
          }
        }

        if(nextAssetButton.Contains(touchX, touchY)) {
          assetHistory.Push(++assetId);
        }

        if(prevActionButton.Contains(touchX, touchY)) {
          asset->SetPrevAction();
        }

        if(restartActionButton.Contains(touchX, touchY)) {
          asset->RestartAction();
          if(!assetActionPlaying) {
            asset->CancelLastActionBlend();
          }
        }

        if(playActionButton.Contains(touchX, touchY)) {
          assetActionPlaying = !assetActionPlaying;
        }

        if(nextActionButton.Contains(touchX, touchY)) {
          asset->SetNextAction();
        }
      }
    }

    engine.EndFrame();
  }

  return 0;
}
