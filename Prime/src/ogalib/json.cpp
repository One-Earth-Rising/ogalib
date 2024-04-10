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
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  value().Accept(writer);
  const char* result = buffer.GetString();
  return std::string(result ? result : "");
}

std::string json::const_iterator::tostring() const {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  value().Accept(writer);
  const char* result = buffer.GetString();
  return std::string(result ? result : "");
}

json::json():
iter(json::iterator(*this)),
constIter(json::const_iterator(*this)) {

}

json::json(const json& v):
iter(json::iterator(*this)),
constIter(json::const_iterator(*this)) {
  if(v.iter)
    doc.CopyFrom(v.iter.value(), doc.GetAllocator());
  else if(v.constIter)
    doc.CopyFrom(v.constIter.value(), doc.GetAllocator());
  else
    doc.CopyFrom(v.doc, doc.GetAllocator());
}

json::json(const std::initializer_list<jsonbuilder::builder::field_holder>& v):
iter(json::iterator(*this)),
constIter(json::const_iterator(*this)) {
  rapidjson::Value rapid_value(jsonbuilder::build_value(v, doc.GetAllocator()), doc.GetAllocator());
  doc.CopyFrom(rapid_value, doc.GetAllocator(), true);
}

json::json(const json::iterator& it):
iter(it),
constIter(json::const_iterator(*this)) {

}

json::json(const json::const_iterator& it):
iter(json::iterator(*this)),
constIter(it) {

}

json::~json() {

}

json::iterator json::operator[](const char* v) {
  if(iter) {
    return {iter.js, iter.value().FindMember(v), iter.iterEnd};
  }
  else {
    if(auto it = find(v)) {
      return it;
    }
    else {
      if(doc.IsObject()) {
        doc.AddMember(rapidjson::Value(v, doc.GetAllocator()), rapidjson::Value(), doc.GetAllocator());
        return find(v);
      }
      else {
        return json::iterator(*this);
      }
    }
  }
}

json::iterator json::operator[](const std::string& v) {
  if(iter) {
    return {iter.js, iter.value().FindMember(v), iter.iterEnd};
  }
  else {
    if(auto it = find(v)) {
      return it;
    }
    else {
      if(doc.IsObject()) {
        doc.AddMember(rapidjson::Value().SetString(v.c_str(), (rapidjson::SizeType) v.size(), doc.GetAllocator()), rapidjson::Value(), doc.GetAllocator());
        return find(v);
      }
      else {
        return json::iterator(*this);
      }
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
  if(constIter) {
    return {constIter.js, constIter.value().FindMember(v), constIter.iterEnd};
  }
  else {
    if(auto it = find(v)) {
      return it;
    }
    else {
      return json::const_iterator(*this);
    }
  }
}

json::const_iterator json::operator[](const std::string& v) const {
  if(constIter) {
    return {constIter.js, constIter.value().FindMember(v), constIter.iterEnd};
  }
  else {
    if(auto it = find(v)) {
      return it;
    }
    else {
      return json::const_iterator(*this);
    }
  }
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

json& json::operator=(long long v) {
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
  doc.CopyFrom(v.doc, doc.GetAllocator());
  return *this;
}

json& json::operator=(const std::initializer_list<jsonbuilder::builder::field_holder>& v) {
  rapidjson::Value rapid_value(jsonbuilder::build_value(v, doc.GetAllocator()));
  rapid_value.Swap(doc);
  return *this;
}

json& json::operator=(const json::iterator& v) {
  doc.CopyFrom(v.value(), doc.GetAllocator());
  return *this;
}

json& json::operator=(const json::const_iterator& v) {
  doc.CopyFrom(v.value(), doc.GetAllocator());
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

json json::operator+(const json& v) const {
  json result;
  result = *this;
  result += v;
  return result;
}

bool json::parse(const char* v) {
  doc.Parse(v);

  if(doc.HasParseError()) {
    err = string_printf("ogalib json parse error, RapidJSON error code: %d", doc.GetParseError());
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
  if(iter) {
    return {iter.js, iter.find(v).iter, iter.iterEnd};
  }
  else if(constIter) {
    return json::iterator(*this);
  }
  else {
    if(doc.IsNull())
      doc.SetObject();

    if(doc.IsObject())
      return json::iterator(*this, doc.FindMember(v), doc.MemberEnd());
    else
      return json::iterator(*this);
  }
}

json::iterator json::find(const std::string& v) {
  if(iter) {
    return {iter.js, iter.find(v).iter, iter.iterEnd};
  }
  else {
    if(doc.IsNull())
      doc.SetObject();

    if(doc.IsObject())
      return json::iterator(*this, doc.FindMember(v), doc.MemberEnd());
    else
      return json::iterator(*this);
  }
}

json::const_iterator json::find(const char* v) const {
  if(iter) {
    return {iter.js, iter.find(v).iter, iter.iterEnd};
  }
  else if(constIter) {
    return {constIter.js, constIter.find(v).iter, constIter.iterEnd};
  }
  else {
    if(doc.IsObject())
      return json::const_iterator(*this, doc.FindMember(v), doc.MemberEnd());
    else
      return json::const_iterator(*this);
  }
}

json::const_iterator json::find(const std::string& v) const {
  if(iter) {
    return {iter.js, iter.find(v).iter, iter.iterEnd};
  }
  else if(constIter) {
    return {constIter.js, constIter.find(v).iter, iter.iterEnd};
  }
  else {
    if(doc.IsObject())
      return json::const_iterator(*this, doc.FindMember(v), doc.MemberEnd());
    else
      return json::const_iterator(*this);
  }
}

json::iterator json::at(size_t v) {
  if(iter) {
    return {iter.js, iter.at(v).valueIter, iter.valueIterEnd};
  }
  else if(doc.IsArray()) {
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
  if(constIter) {
    return {constIter.js, constIter.at(v).valueIter, constIter.valueIterEnd};
  }
  else if(doc.IsArray()) {
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
  if(doc.IsArray()) {
    return doc.Size();
  }
  else {
    return 0;
  }
}

json::iterator json::begin() {
  if(iter) {
    return iter.begin();
  }
  else {
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
}

json::iterator json::end() {
  if(iter) {
    return iter.end();
  }
  else {
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
}

json::const_iterator json::begin() const {
  if(constIter) {
    return constIter.begin();
  }
  else {
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
}

json::const_iterator json::end() const {
  if(constIter) {
    return constIter.end();
  }
  else {
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
}

bool json::IsObject() const {
  if(iter) {
    return iter.IsObject();
  }
  else if(constIter) {
    return constIter.IsObject();
  }
  else {
    return doc.IsObject();
  }
}

bool json::IsArray() const {
  if(iter) {
    return iter.IsArray();
  }
  else if(constIter) {
    return constIter.IsArray();
  }
  else {
    return doc.IsArray();
  }
}

bool json::IsBool() const {
  if(iter) {
    return iter.IsBool();
  }
  else if(constIter) {
    return constIter.IsBool();
  }
  else {
    return doc.IsBool();
  }
}

bool json::IsInt() const {
  if(iter) {
    return iter.IsInt();
  }
  else if(constIter) {
    return constIter.IsInt();
  }
  else {
    return doc.IsInt();
  }
}

bool json::IsUint() const {
  if(iter) {
    return iter.IsUint();
  }
  else if(constIter) {
    return constIter.IsUint();
  }
  else {
    return doc.IsUint();
  }
}

bool json::IsInt64() const {
  if(iter) {
    return iter.IsInt64();
  }
  else if(constIter) {
    return constIter.IsInt64();
  }
  else {
    return doc.IsInt64();
  }
}

bool json::IsUint64() const {
  if(iter) {
    return iter.IsUint64();
  }
  else if(constIter) {
    return constIter.IsUint64();
  }
  else {
    return doc.IsUint64();
  }
}

bool json::IsNumber() const {
  if(iter) {
    return iter.IsNumber();
  }
  else if(constIter) {
    return constIter.IsNumber();
  }
  else {
    return doc.IsNumber();
  }
}

bool json::IsDouble() const {
  if(iter) {
    return iter.IsDouble();
  }
  else if(constIter) {
    return constIter.IsDouble();
  }
  else {
    return doc.IsDouble();
  }
}

bool json::IsString() const {
  if(iter) {
    return iter.IsString();
  }
  else if(constIter) {
    return constIter.IsString();
  }
  else {
    return doc.IsString();
  }
}

bool json::IsNull() const {
  if(iter) {
    return iter.IsNull();
  }
  else if(constIter) {
    return constIter.IsNull();
  }
  else {
    return doc.IsNull();
  }
}

bool json::GetBool() const {
  if(iter) {
    auto& v = iter.value();
    if(v.IsBool()) {
      return v.GetBool();
    }
  }
  else if(constIter) {
    auto& v = constIter.value();
    if(v.IsBool()) {
      return v.GetBool();
    }
  }
  else {
    if(doc.IsBool()) {
      return doc.GetBool();
    }
  }

  return false;
}

int json::GetInt() const {
  if(iter) {
    auto& v = iter.value();
    if(v.IsInt()) {
      return v.GetInt();
    }
  }
  else if(constIter) {
    auto& v = constIter.value();
    if(v.IsInt()) {
      return v.GetInt();
    }
  }
  else {
    if(doc.IsInt()) {
      return doc.GetInt();
    }
  }

  return 0;
}

long long json::GetInt64() const {
  if(iter) {
    auto& v = iter.value();
    if(v.IsInt64()) {
      return v.GetInt64();
    }
  }
  else if(constIter) {
    auto& v = constIter.value();
    if(v.IsInt64()) {
      return v.GetInt64();
    }
  }
  else {
    if(doc.IsInt64()) {
      return doc.GetInt64();
    }
  }

  return 0;
}

unsigned int json::GetUint() const {
  if(iter) {
    auto& v = iter.value();
    if(v.IsUint()) {
      return v.GetUint();
    }
  }
  else if(constIter) {
    auto& v = constIter.value();
    if(v.IsUint()) {
      return v.GetUint();
    }
  }
  else {
    if(doc.IsUint()) {
      return doc.GetUint();
    }
  }

  return 0;
}

unsigned long long json::GetUint64() const {
  if(iter) {
    auto& v = iter.value();
    if(v.IsUint64()) {
      return v.GetUint64();
    }
  }
  else if(constIter) {
    auto& v = constIter.value();
    if(v.IsUint64()) {
      return v.GetUint64();
    }
  }
  else {
    if(doc.IsUint64()) {
      return doc.GetUint64();
    }
  }

  return 0;
}

float json::GetFloat() const {
  if(iter) {
    auto& v = iter.value();
    if(v.IsNumber()) {
      return v.GetFloat();
    }
  }
  else if(constIter) {
    auto& v = constIter.value();
    if(v.IsNumber()) {
      return v.GetFloat();
    }
  }
  else {
    if(doc.IsNumber()) {
      return doc.GetFloat();
    }
  }

  return 0.0f;
}

double json::GetDouble() const {
  if(iter) {
    auto& v = iter.value();
    if(v.IsDouble()) {
      return v.GetDouble();
    }
  }
  else if(constIter) {
    auto& v = constIter.value();
    if(v.IsDouble()) {
      return v.GetDouble();
    }
  }
  else {
    if(doc.IsDouble()) {
      return doc.GetDouble();
    }
  }

  return 0.0;
}

std::string json::GetString() const {
  if(iter) {
    auto& v = iter.value();
    if(v.IsString()) {
      const char* result = v.GetString();
      return std::string(result ? result : "");
    }
  }
  else if(constIter) {
    auto& v = constIter.value();
    if(v.IsString()) {
      const char* result = v.GetString();
      return std::string(result ? result : "");
    }
  }
  else {
    if(doc.IsString()) {
      const char* result = doc.GetString();
      return std::string(result ? result : "");
    }
  }

  return std::string();
}

std::string json::tostring() const {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  if(iter) {
    iter.value().Accept(writer);
  }
  else if(constIter) {
    constIter.value().Accept(writer);
  }
  else {
    doc.Accept(writer);
  }
  const char* result = buffer.GetString();
  return std::string(result ? result : "");
}

json json::array() {
  json js;
  js.doc.SetArray();
  return js;
}

json json::object() {
  json js;
  js.doc.SetObject();
  return js;
}
