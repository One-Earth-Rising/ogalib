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

#include <Prime/Graphics/ArrayBuffer.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

ArrayBuffer::ArrayBuffer(size_t itemSize, const void* data, size_t itemCount, BufferPrimitive primitive):
itemSize(itemSize),
itemCount(itemCount),
primitive(primitive),
loadedIntoVRAM(false),
dataModified(false) {
  syncCount = data ? itemCount : 0;
}

ArrayBuffer::~ArrayBuffer() {
  PrimeAssert(!loadedIntoVRAM, "Array buffer still loaded in VRAM.");
}

bool ArrayBuffer::LoadIntoVRAM() {
  return false;
}

bool ArrayBuffer::UnloadFromVRAM() {
  return true;
}

void ArrayBuffer::LoadAttribute(const std::string& name, size_t size) {
  ArrayBufferAttribute attribute(name, size);
  size_t index = attributes.GetCount();
  attributes.Push(attribute);

  attributeLookup[name] = index;
}

const std::string& ArrayBuffer::GetAttributeName(size_t index) const {
  static const std::string noName;

  if(index < attributes.GetCount())
    return attributes[index].GetName();
  else
    return noName;
}

const ArrayBufferAttribute* ArrayBuffer::GetAttribute(const std::string& name) {
  if(auto it = attributeLookup.Find(name))
    return &attributes[it.value()];
  else
    return 0;
}

size_t ArrayBuffer::GetAttributeCount() const {
  return attributes.GetCount();
}

void ArrayBuffer::ProcessAttributes() {
  size_t offset = 0;
  for(auto& attribute: attributes) {
    attribute.offset = offset;
    offset += attribute.size;
  }
}

const void* ArrayBuffer::GetItem(size_t index) const {
  PrimeAssert(false, "Unimplemented get const item function for ArrayBuffer.");
  return NULL;
}

void* ArrayBuffer::GetItem(size_t index) {
  PrimeAssert(false, "Unimplemented get item function for ArrayBuffer.");
  return NULL;
}

void ArrayBuffer::SetItem(size_t index, const void* data) {
  PrimeAssert(false, "Unimplemented set item function for ArrayBuffer.");
}

void ArrayBuffer::SetSyncCount(size_t count) {
  if(!dataModified)
    dataModified = count > syncCount;
  syncCount = count;
}

void ArrayBuffer::Sync() {
  PrimeAssert(false, "Unimplemented sync function for ArrayBuffer.");
}
