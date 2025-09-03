/*
ogalib

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

#include <ogalib/json.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <ogalib/ogalib.h>

using namespace ogalib;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

const rapidjson::Document::MemberIterator json::NullIter;
const rapidjson::Document::ValueIterator json::NullValueIter = nullptr;
const rapidjson::Document::ConstMemberIterator json::ConstNullIter;
const rapidjson::Document::ConstValueIterator json::ConstNullValueIter = nullptr;

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////


std::string json::iterator::tostring() const {
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  writer.SetIndent(' ', 2);
  value().Accept(writer);
  const char* result = buffer.GetString();
  return std::string(result ? result : "");
}

std::string json::const_iterator::tostring() const {
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  writer.SetIndent(' ', 2);
  value().Accept(writer);
  const char* result = buffer.GetString();
  return std::string(result ? result : "");
}

json::json() {
  doc.SetNull();
}

json::json(const json& v) {
  auto& allocator = doc.GetAllocator();
  allocator.Clear();
  doc.CopyFrom(v.doc, allocator);
}

json::json(const rapidjson::Value& v) {
  auto& allocator = doc.GetAllocator();
  allocator.Clear();
  doc.CopyFrom(v, allocator);
}

json::json(const std::initializer_list<jsonbuilder::builder::field_holder>& v) {
  auto& allocator = doc.GetAllocator();
  allocator.Clear();
  rapidjson::Value value(jsonbuilder::build_value(v, doc.GetAllocator()));
  doc.CopyFrom(value, doc.GetAllocator(), true);
}

json::json(const json::iterator& it) {
  auto& allocator = doc.GetAllocator();
  allocator.Clear();
  doc.CopyFrom(it.value(), allocator);
}

json::json(const json::const_iterator& it) {
  auto& allocator = doc.GetAllocator();
  allocator.Clear();
  doc.CopyFrom(it.value(), allocator);
}

json::~json() {

}

json::iterator json::operator[](const char* v) {
  if(auto it = find(v)) {
    return {it.js, it.iter, it.iterEnd};
  }
  else {
    if(doc.IsObject()) {
      doc.AddMember(rapidjson::Value(v, doc.GetAllocator()), rapidjson::Value(), doc.GetAllocator());
      return find(v);
    }
    else {
      return {*this};
    }
  }
}

json::iterator json::operator[](const std::string& v) {
  if(auto it = find(v)) {
    return {it.js, it.iter, it.iterEnd};
  }
  else {
    if(doc.IsObject()) {
      doc.AddMember(rapidjson::Value().SetString(v.c_str(), (rapidjson::SizeType) v.size(), doc.GetAllocator()), rapidjson::Value(), doc.GetAllocator());
      return find(v);
    }
    else {
      return {*this};
    }
  }
}

json::iterator json::operator[](const json& v) {
  if(v.IsNumber()) {
    if(doc.IsArray()) {
      size_t index = v.doc.GetUint64();
      if(index < doc.Size()) {
        return json::iterator(*this, doc.Begin() + index, doc.End());
      }
    }
  }
  else if(v.IsString()) {
    return (*this)[v.doc.GetString()];
  }

  return json::iterator(*this);
}

json::const_iterator json::operator[](const char* v) const {
  if(auto it = find(v)) {
    return {it.js, it.iter, it.iterEnd};
  }
  else {
    return {*this};
  }
}

json::const_iterator json::operator[](const std::string& v) const {
  if(auto it = find(v)) {
    return {it.js, it.iter, it.iterEnd};
  }
  else {
    return {*this};
  }
}

json::const_iterator json::operator[](const json& v) const {
  if(v.IsNumber()) {
    if(doc.IsArray()) {
      size_t index = v.doc.GetUint64();
      if(index < doc.Size()) {
        return json::const_iterator(*this, doc.Begin() + index, doc.End());
      }
    }
  }
  else if(v.IsString()) {
    return (*this)[v.doc.GetString()];
  }

  return json::const_iterator(*this);
}

json& json::operator=(bool v) {
  doc.SetBool(v);
  return *this;
}

json& json::operator=(int v) {
  doc.SetInt(v);
  return *this;
}

json& json::operator=(unsigned int v) {
  doc.SetUint(v);
  return *this;
}

json& json::operator=(int64_t v) {
  doc.SetInt64(v);
  return *this;
}

json& json::operator=(size_t v) {
  doc.SetUint64(v);
  return *this;
}

json& json::operator=(float v) {
  doc.SetFloat(v);
  return *this;
}

json& json::operator=(double v) {
  doc.SetDouble(v);
  return *this;
}

json& json::operator=(const char* v) {
  doc.SetString(std::string(v), doc.GetAllocator());
  return *this;
}

json& json::operator=(const json& v) {
  auto& allocator = doc.GetAllocator();
  allocator.Clear();
  doc.CopyFrom(v.doc, allocator);
  return *this;
}

json& json::operator=(const std::initializer_list<jsonbuilder::builder::field_holder>& v) {
  auto& allocator = doc.GetAllocator();
  allocator.Clear();
  rapidjson::Value value(jsonbuilder::build_value(v, doc.GetAllocator()));
  doc.CopyFrom(value, doc.GetAllocator(), true);
  return *this;
}

json& json::operator=(const json::iterator& v) {
  auto& allocator = doc.GetAllocator();
  allocator.Clear();
  doc.CopyFrom(v.value(), allocator);
  return *this;
}

json& json::operator=(const json::const_iterator& v) {
  auto& allocator = doc.GetAllocator();
  allocator.Clear();
  doc.CopyFrom(v.value(), allocator);
  return *this;
}

json& json::operator+=(const json& v) {
  if(doc.IsNull()) {
    if(v.IsArray()) {
      doc.SetArray();
    }
    else {
      doc.SetObject();
    }
  }

  if(v.IsObject()) {
    for(auto& it: v) {
      doc.AddMember(rapidjson::Value(it.key(), doc.GetAllocator()), rapidjson::Value(it.value(), doc.GetAllocator()), doc.GetAllocator());
    }
  }
  else if(v.IsArray()) {
    for(auto& it: v) {
      doc.PushBack(rapidjson::Value(it.value(), doc.GetAllocator()), doc.GetAllocator());
    }
  }

  return *this;
}

json& json::operator+=(const std::initializer_list<jsonbuilder::builder::field_holder>& v) {
  *this += json(v);
  return *this;
}

json json::operator+(const json& v) const {
  json result;
  result = *this;
  result += v;
  return result;
}

bool json::operator==(const json& v) const {
  return doc == v.doc;
}

json& json::null() {
  doc.SetNull();
  return *this;
}

json& json::array() {
  doc.SetArray();
  return *this;
}

json& json::object() {
  doc.SetObject();
  return *this;
}

bool json::parse(const char* v) {
  doc.Parse(v);

  if(doc.HasParseError()) {
    err = ogalib::string_printf("ogalib json parse error, RapidJSON error code: %d", doc.GetParseError());
    return false;
  }
  else {
    return true;
  }
}

bool json::parse(const std::string& v) {
  doc.Parse(v.c_str(), v.size());

  if(doc.HasParseError()) {
    err = string_printf("ogalib json parse error, RapidJSON error code: %d", doc.GetParseError());
    return false;
  }
  else {
    return true;
  }
}

bool json::parse(const void* data, size_t dataSize) {
  doc.Parse((const char*) data, dataSize);

  if(doc.HasParseError()) {
    err = string_printf("ogalib json parse error, RapidJSON error code: %d", doc.GetParseError());
    return false;
  }
  else {
    return true;
  }
}

json& json::append(int v) {
  if(doc.IsNull())
    doc.SetArray();

  if(doc.IsArray())
    doc.PushBack(v, doc.GetAllocator());

  return *this;
}

json& json::append(const char* v) {
  if(doc.IsNull())
    doc.SetArray();

  if(doc.IsArray())
    doc.PushBack(rapidjson::Value(v, doc.GetAllocator()), doc.GetAllocator());

  return *this;
}

json& json::append(const json& v) {
  if(doc.IsNull())
    doc.SetArray();

  if(doc.IsArray())
    doc.PushBack(rapidjson::Value().CopyFrom(v.doc, doc.GetAllocator()), doc.GetAllocator());

  return *this;
}

json& json::append(const json::iterator& v) {
  if(doc.IsNull())
    doc.SetArray();

  if(doc.IsArray())
    doc.PushBack(rapidjson::Value().CopyFrom(v.value(), doc.GetAllocator()), doc.GetAllocator());

  return *this;
}

json::iterator json::find(const char* v) {
  if(doc.IsNull())
    doc.SetObject();

  if(doc.IsObject())
    return json::iterator(*this, doc.FindMember(v), doc.MemberEnd());
  else
    return json::iterator(*this);
}

json::iterator json::find(const std::string& v) {
  if(doc.IsNull())
    doc.SetObject();

  if(doc.IsObject())
    return json::iterator(*this, doc.FindMember(v), doc.MemberEnd());
  else
    return json::iterator(*this);
}

json::const_iterator json::find(const char* v) const {
  if(doc.IsObject())
    return json::const_iterator(*this, doc.FindMember(v), doc.MemberEnd());
  else
    return json::const_iterator(*this);
}

json::const_iterator json::find(const std::string& v) const {
  if(doc.IsObject())
    return json::const_iterator(*this, doc.FindMember(v), doc.MemberEnd());
  else
    return json::const_iterator(*this);
}

json::iterator json::at(size_t v) {
  if(doc.IsArray()) {
    if(v < doc.Size()) {
      return json::iterator(*this, doc.Begin() + v, doc.End());
    }
    else {
      return json::iterator(*this);
    }
  }

  return json::iterator(*this);
}

json::const_iterator json::at(size_t v) const {
  if(doc.IsArray()) {
    if(v < doc.Size()) {
      return json::const_iterator(*this, doc.Begin() + v, doc.End());
    }
  }

  return json::const_iterator(*this);
}

json& json::erase(const char* v) {
  if(doc.IsObject()) {
    doc.EraseMember(v);
  }

  return *this;
}

json& json::erase(const std::string& v) {
  if(doc.IsObject()) {
    doc.EraseMember(v);
  }

  return *this;
}

json& json::erase(const iterator& it) {
  if(&it.js == this) {
    if(doc.IsObject()) {
      doc.EraseMember(it.value());
    }
  }

  return *this;
}

json& json::clear() {
  if(doc.IsObject()) {
    doc.SetObject();
  }
  else if(doc.IsArray()) {
    doc.Clear();
  }
  return *this;
}

size_t json::size() const {
  if(doc.IsObject()) {
    return doc.MemberCount();
  }
  if(doc.IsArray()) {
    return doc.Size();
  }
  else {
    return 0;
  }
}

bool json::empty() const {
  if(IsNull()) {
    return false;
  }
  else if(IsObject()) {
    return doc.MemberCount() == 0;
  }
  else if(IsArray()) {
    return size() == 0;
  }
  else {
    return false;
  }
}

json::iterator json::begin() {
  if(doc.IsObject()) {
    return iterator(*this, doc.MemberBegin(), doc.MemberEnd());
  }
  else if(doc.IsArray()) {
    return iterator(*this, doc.Begin(), doc.End());
  }
  else {
    return iterator(*this);
  }
}

json::iterator json::end() {
  if(doc.IsObject()) {
    return iterator(*this, doc.MemberEnd(), doc.MemberEnd());
  }
  else if(doc.IsArray()) {
    return iterator(*this, doc.End(), doc.End());
  }
  else {
    return iterator(*this);
  }
}

json::const_iterator json::begin() const {
  if(doc.IsObject()) {
    return const_iterator(*this, doc.MemberBegin(), doc.MemberEnd());
  }
  else if(doc.IsArray()) {
    return const_iterator(*this, doc.Begin(), doc.End());
  }
  else {
    return const_iterator(*this);
  }
}

json::const_iterator json::end() const {
  if(doc.IsObject()) {
    return const_iterator(*this, doc.MemberEnd(), doc.MemberEnd());
  }
  else if(doc.IsArray()) {
    return const_iterator(*this, doc.End(), doc.End());
  }
  else {
    return const_iterator(*this);
  }
}

bool json::IsObject() const {
  return doc.IsObject();
}

bool json::IsArray() const {
  return doc.IsArray();
}

bool json::IsBool() const {
  return doc.IsBool();
}

bool json::IsInt() const {
  return doc.IsInt();
}

bool json::IsUint() const {
  return doc.IsUint();
}

bool json::IsInt64() const {
  return doc.IsInt64();
}

bool json::IsUint64() const {
  return doc.IsUint64();
}

bool json::IsNumber() const {
  return doc.IsNumber();
}

bool json::IsFloat() const {
  return doc.IsFloat();
}

bool json::IsDouble() const {
  return doc.IsDouble();
}

bool json::IsString() const {
  return doc.IsString();
}

bool json::IsNull() const {
  return doc.IsNull();
}

bool json::GetBool() const {
  if(doc.IsBool()) {
    return doc.GetBool();
  }

  return false;
}

int json::GetInt() const {
  if(doc.IsInt()) {
    return doc.GetInt();
  }

  return 0;
}

long long json::GetInt64() const {
  if(doc.IsInt64()) {
    return doc.GetInt64();
  }

  return 0;
}

unsigned int json::GetUint() const {
  if(doc.IsUint()) {
    return doc.GetUint();
  }

  return 0;
}

unsigned long long json::GetUint64() const {
  if(doc.IsUint64()) {
    return doc.GetUint64();
  }

  return 0;
}

float json::GetFloat() const {
  if(doc.IsNumber()) {
    return doc.GetFloat();
  }

  return 0.0f;
}

double json::GetDouble() const {
  if(doc.IsDouble()) {
    return doc.GetDouble();
  }

  return 0.0;
}

std::string json::GetString() const {
  if(doc.IsString()) {
    const char* result = doc.GetString();
    return std::string(result ? result : "", doc.GetStringLength());
  }

  return std::string();
}

std::string json::tostring() const {
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  writer.SetIndent(' ', 2);
  doc.Accept(writer);
  const char* result = buffer.GetString();
  return std::string(result ? result : "");
}
