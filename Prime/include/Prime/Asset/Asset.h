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

#include <Prime/Imagemap/Imagemap.h>
#include <Prime/Skeleton/Skeleton.h>
#include <Prime/Model/Model.h>
#include <Prime/Rig/Rig.h>
#include <Prime/Graphics/DeviceProgram.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class Asset: public RefObject {
private:

  Asset* parent;

  std::string apiRoot;
  Stack<std::string> acceptedTextureFormats;

  refptr<Imagemap> imagemap;
  refptr<Skeleton> skeleton;
  refptr<Model> model;
  refptr<Rig> rig;

  json info;
  std::string uri;
  std::string format;
  json dataManifest;
  Stack<refptr<Asset>> dataManifestAssets;
  bool textureFilteringEnabled;
  
  size_t loadingCount;
  size_t loadQueuedId;

  refptr<DeviceProgram> texProgram;
  refptr<DeviceProgram> skeletonProgram;
  refptr<DeviceProgram> modelProgram;
  refptr<DeviceProgram> modelAnimProgram;

public:

  Asset* GetParent() const {return parent;}
  const json& GetInfo() const {return info;}
  const json& GetDataManifest() const {return dataManifest;}
  const Stack<refptr<Asset>>& GetDataManifestAssets() const {return dataManifestAssets;}
  bool GetTextureFilteringEnabled() const {return textureFilteringEnabled;}

public:

  Asset();
  virtual ~Asset();

public:

  virtual void SetParent(Asset* parent);
  virtual void SetAPIRoot(const std::string& apiRoot);
  virtual void SetTexProgram(refptr<DeviceProgram> program);
  virtual void SetSkeletonProgram(refptr<DeviceProgram> program);
  virtual void SetModelProgram(refptr<DeviceProgram> program);
  virtual void SetModelAnimProgram(refptr<DeviceProgram> program);
  virtual void SetAcceptedTextureFormats(const Stack<std::string>& formats);
  virtual void SetTextureFilteringEnabled(bool enabled);

  virtual void Load(size_t id);

  virtual size_t GetActionCount() const;
  virtual const std::string& GetActionName() const;
  virtual size_t GetActionIndex() const;
  virtual f32 GetActionLen() const;
  virtual void RestartAction();
  virtual void SetNextAction();
  virtual void SetPrevAction();
  virtual void CancelLastActionBlend();

  virtual void Calc(f32 dt);
  virtual void Draw();

  virtual const std::string& GetURI() const;
  virtual std::string GetFormat() const;
  virtual f32 GetUniformSize() const;
  virtual Vec2 GetViewOffset() const;
  virtual bool Is2D() const;
  virtual bool IsImagemap() const;
  virtual bool IsSkeleton() const;
  virtual bool IsModel() const;
  virtual bool IsRig() const;

  virtual const std::string& GetAPIRoot() const;
  virtual refptr<DeviceProgram> GetTexProgram() const;
  virtual refptr<DeviceProgram> GetSkeletonProgram() const;
  virtual refptr<DeviceProgram> GetModelProgram() const;
  virtual refptr<DeviceProgram> GetModelAnimProgram() const;

  static Stack<std::string> SplitString(const std::string& str, const std::string& delim = std::string(" "));
  static std::string GetExtension(const std::string& uri);

protected:

  virtual void IncLoading();
  virtual void DecLoading();

};

};
