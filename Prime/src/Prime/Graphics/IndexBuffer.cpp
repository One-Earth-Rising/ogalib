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

#include <Prime/Graphics/IndexBuffer.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

IndexBuffer::IndexBuffer(IndexFormat format, const void* data, size_t indexCount):
format(format),
indexCount(indexCount),
loadedIntoVRAM(false),
dataModified(false) {
  syncCount = data ? indexCount : 0;
}

IndexBuffer::~IndexBuffer() {
  PrimeAssert(!loadedIntoVRAM, "Index buffer still loaded in VRAM.");
}

bool IndexBuffer::LoadIntoVRAM() {
  return false;
}

bool IndexBuffer::UnloadFromVRAM() {
  return true;
}

size_t IndexBuffer::GetIndexSize() const {
  PrimeAssert(false, "Unimplemented get index size function for IndexBuffer.");
  return 0;
}

size_t IndexBuffer::GetValue(size_t index) const {
  PrimeAssert(false, "Unimplemented get value function for IndexBuffer.");
  return 0;
}

void IndexBuffer::SetValue(size_t index, size_t value) {
  PrimeAssert(false, "Unimplemented set value function for IndexBuffer.");
}

void IndexBuffer::SetValues(size_t start, size_t count, const void* data) {
  PrimeAssert(false, "Unimplemented set values function for IndexBuffer.");
}

void IndexBuffer::CopyValueBlock(size_t index, size_t fromIndex, size_t count) {
  PrimeAssert(false, "Unimplemented copy block function for IndexBuffer.");
}

void IndexBuffer::SetSyncCount(size_t count) {
  if(!dataModified)
    dataModified = count > syncCount;
  syncCount = count;
}

void IndexBuffer::Sync() {
  PrimeAssert(false, "Unimplemented sync function for IndexBuffer.");
}
