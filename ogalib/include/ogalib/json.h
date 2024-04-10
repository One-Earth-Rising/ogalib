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

#pragma once

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <ogalib/Types.h>

#ifdef _CRTDBG_MAP_ALLOC
#ifdef new
#undef new
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {

class json {
public:

  static const rapidjson::Document::MemberIterator NullIter;
  static const rapidjson::Document::ValueIterator NullValueIter;
  static const rapidjson::Document::ConstMemberIterator ConstNullIter;
  static const rapidjson::Document::ConstValueIterator ConstNullValueIter;

  class const_iterator;

  class iterator {
  friend class json;
  friend class const_iterator;
  private:

    json& js;
    rapidjson::Document::MemberIterator iter;
    rapidjson::Document::MemberIterator iterEnd;
    rapidjson::Document::ValueIterator valueIter;
    rapidjson::Document::ValueIterator valueIterEnd;

  public:

    iterator(json& js): js(js), iter(NullIter), iterEnd(NullIter), valueIter(NullValueIter), valueIterEnd(NullValueIter) {}
    iterator(json& js, const rapidjson::Document::MemberIterator& nativeIter, const rapidjson::Document::MemberIterator& nativeIterEnd): js(js), iter(nativeIter), iterEnd(nativeIterEnd), valueIter(NullValueIter), valueIterEnd(NullValueIter) {}
    iterator(json& js, const rapidjson::Document::ValueIterator& nativeIter, const rapidjson::Document::ValueIterator& nativeIterEnd): js(js), iter(NullIter), iterEnd(NullIter), valueIter(nativeIter), valueIterEnd(nativeIterEnd) {}

  private:

    iterator(const iterator& other): js(other.js), iter(other.iter), iterEnd(other.iterEnd), valueIter(other.valueIter), valueIterEnd(other.valueIterEnd) {}

  public:

    operator bool() const {
      if(iter != NullIter) {
        return iter != iterEnd;
      }
      else if(valueIter != NullValueIter) {
        return valueIter != valueIterEnd;
      }

      return false;
    }

    operator const char*() const {
      if(iter == NullIter) {
        return "";
      }
      else if(iter->value.IsString()) {
        return iter->value.GetString();
      }
      else {
        return "";
      }
    }

    iterator& operator=(const iterator& other) {
      ogalibAssert(&js == &other.js, "Cannot copy iterator from another structure.");
      if(iter != NullIter) {
        iter = other.iter;
      }
      else if(valueIter != NullValueIter) {
        valueIter = other.valueIter;
      }
      return *this;
    }

    iterator& operator++() {
      if(iter != NullIter) {
        ++iter;
      }
      else if(valueIter != NullValueIter) {
        ++valueIter;
      }
      return *this;
    }

    iterator& operator*() {
      return *this;
    }

    iterator& operator=(const char* v) {
      if(iter != NullIter)
        value() = rapidjson::Value(v, js.doc.GetAllocator());
      return *this;
    }

    iterator& operator=(const std::string& v) {
      if(iter != NullIter)
        value() = rapidjson::Value().SetString(v.c_str(), (rapidjson::SizeType) v.size(), js.doc.GetAllocator());
      return *this;
    }

    iterator& operator=(char v) {
      if(iter != NullIter)
        value() = v;
      return *this;
    }

    iterator& operator=(short v) {
      if(iter != NullIter)
        value() = v;
      return *this;
    }

    iterator& operator=(int v) {
      if(iter != NullIter)
        value() = v;
      return *this;
    }

    iterator& operator=(int64_t v) {
      if(iter != NullIter)
        value() = v;
      return *this;
    }

    iterator& operator=(unsigned char v) {
      if(iter != NullIter)
        value() = v;
      return *this;
    }

    iterator& operator=(unsigned short v) {
      if(iter != NullIter)
        value() = v;
      return *this;
    }

    iterator& operator=(unsigned int v) {
      if(iter != NullIter)
        value() = v;
      return *this;
    }

    iterator& operator=(float v) {
      if(iter != NullIter)
        value() = v;
      return *this;
    }

    iterator& operator=(double v) {
      if(iter != NullIter)
        value() = v;
      return *this;
    }

    iterator& operator=(bool v) {
      if(iter != NullIter)
        value() = v;
      return *this;
    }

    iterator& operator=(const json& v) {
      if(iter != NullIter)
        value().CopyFrom(v.doc, js.doc.GetAllocator());
      return *this;
    }

    iterator& operator=(size_t v) {
      if(iter != NullIter)
        value() = v;
      return *this;
    }

    iterator& operator=(void* v) {
      if(iter != NullIter)
        value() = (intptr_t) v;
      return *this;
    }

    iterator& operator=(const void* v) {
      if(iter != NullIter)
        value() = (intptr_t) v;
      return *this;
    }

    bool operator==(const iterator& other) const {
      if(iter != NullIter) {
        return iter == other.iter;
      }
      else if(valueIter != NullValueIter) {
        return valueIter == other.valueIter;
      }
      else {
        return false;
      }
    }

    bool operator==(const const_iterator& other) const {
      if(iter != NullIter) {
        return iter == other.iter;
      }
      else if(valueIter != NullValueIter) {
        return valueIter == other.valueIter;
      }
      else {
        return false;
      }
    }

    bool operator!=(const iterator& other) const {
      if(iter != NullIter) {
        return iter != other.iter;
      }
      else if(valueIter != NullValueIter) {
        return valueIter != other.valueIter;
      }
      else {
        return false;
      }
    }

    bool operator!=(const const_iterator& other) const {
      if(iter != NullIter) {
        return iter != other.iter;
      }
      else if(valueIter != NullValueIter) {
        return valueIter != other.valueIter;
      }
      else {
        return false;
      }
    }

    iterator operator[](const char* v) {
      if(js.iter) {
        return {js.iter.js, js.iter.value().FindMember(v), js.iter.iterEnd};
      }
      else {
        if(auto it = find(v)) {
          return it;
        }
        else {
          auto& val = value();
          if(val.IsObject()) {
            val.AddMember(rapidjson::Value(v, js.doc.GetAllocator()), rapidjson::Value(), js.doc.GetAllocator());
            return find(v);
          }
          else {
            return json::iterator(*this);
          }
        }
      }
    }

    iterator operator[](const std::string& v) {
      if(js.iter) {
        return {js.iter.js, js.iter.value().FindMember(v), js.iter.iterEnd};
      }
      else {
        if(auto it = find(v)) {
          return it;
        }
        else {
          auto& val = value();
          if(val.IsObject()) {
            val.AddMember(rapidjson::Value().SetString(v.c_str(), (rapidjson::SizeType) v.size(), js.doc.GetAllocator()), rapidjson::Value(), js.doc.GetAllocator());
            return find(v);
          }
          else {
            return json::iterator(*this);
          }
        }
      }
    }

    const rapidjson::Value& key() const {
      if(iter != NullIter) {
        return iter->name;
      }
      else {
        static rapidjson::Value noValue;
        return noValue;
      }
    }

    rapidjson::Value& value() const {
      if(iter != NullIter) {
        return iter->value;
      }
      else if(valueIter != NullValueIter) {
        return *valueIter;
      }
      else {
        static rapidjson::Value noValue;
        return noValue;
      }
    }

    const char* c_str() const {
      if(iter->value.IsString()) {
        return iter->value.GetString();
      }
      else {
        return nullptr;
      }
    }

    iterator find(const char* v) const {
      auto& val = value();
      if(val.IsObject()) {
        auto it = val.FindMember(v);
        auto itEnd = val.MemberEnd();
        if(it == itEnd) {
          return iterator(js);
        }
        else {
          return iterator(js, it, itEnd);
        }
      }
      else {
        return iterator(js);
      }
    }

    iterator find(const std::string& v) const {
      auto& val = value();
      if(val.IsObject()) {
        auto it = val.FindMember(v);
        auto itEnd = val.MemberEnd();
        if(it == val.MemberEnd()) {
          return iterator(js);
        }
        else {
          return iterator(js, it, itEnd);
        }
      }
      else {
        return iterator(js);
      }
    }

    iterator at(size_t index) const {
      auto& v = value();
      if(v.IsArray()) {
        if(index >= v.Size()) {
          return iterator(js);
        }
        else {
          return iterator(js, v.Begin() + index, v.End());
        }
      }
      else {
        return iterator(js);
      }
    }

    iterator begin() const {
      auto& v = value();
      if(v.IsObject()) {
        return iterator(js, v.MemberBegin(), v.MemberEnd());
      }
      else if(v.IsArray()) {
        return iterator(js, v.Begin(), v.End());
      }
      else {
        return iterator(js);
      }
    }

    iterator end() const {
      auto& v = value();
      if(v.IsObject()) {
        return iterator(js, v.MemberEnd(), v.MemberEnd());
      }
      else if(v.IsArray()) {
        return iterator(js, v.End(), v.End());
      }
      else {
        return iterator(js);
      }
    }

    bool IsObject() const {
      return value().IsObject();
    }

    bool IsArray() const {
      return value().IsArray();
    }

    bool IsBool() const {
      return value().IsBool();
    }

    bool IsInt() const {
      return value().IsInt();
    }

    bool IsUint() const {
      return value().IsUint();
    }

    bool IsInt64() const {
      return value().IsInt64();
    }

    bool IsUint64() const {
      return value().IsUint64();
    }

    bool IsNumber() const {
      return value().IsNumber();
    }

    bool IsDouble() const {
      return value().IsDouble();
    }

    bool IsString() const {
      return value().IsString();
    }

    bool IsNull() const {
      return value().IsNull();
    }

    bool IsSizeT() const {
      ogalibAssert(sizeof(size_t) <= sizeof(uint64_t), "Type size_t is too large to be supported by json values.");
      if(sizeof(size_t) > sizeof(uint64_t)) {
        return false;
      }

      return value().IsUint64();
    }

    bool IsVoidPtr() const {
      ogalibAssert(sizeof(intptr_t) <= sizeof(int64_t), "Type intptr_t is too large to be supported by json values.");
      if(sizeof(intptr_t) > sizeof(int64_t)) {
        return false;
      }

      return value().IsInt64();
    }

    bool GetBool() const {
      auto& v = value();
      if(v.IsBool()) {
        return v.GetBool();
      }

      return false;
    }


    int GetInt() const {
      auto& v = value();
      if(v.IsInt()) {
        return v.GetInt();
      }

      return 0;
    }

    long long GetInt64() const {
      auto& v = value();
      if(v.IsInt64()) {
        return v.GetInt64();
      }

      return 0;
    }

    unsigned int GetUint() const {
      auto& v = value();
      if(v.IsUint()) {
        return v.GetUint();
      }

      return 0;
    }

    unsigned long long GetUint64() const {
      auto& v = value();
      if(v.IsUint64()) {
        return v.GetUint64();
      }

      return 0;
    }

    float GetFloat() const {
      auto& v = value();
      if(v.IsNumber()) {
        return v.GetFloat();
      }

      return 0.0f;
    }

    double GetDouble() const {
      auto& v = value();
      if(v.IsDouble()) {
        return v.GetDouble();
      }

      return 0.0;
    }

    std::string GetString() const {
      auto& v = value();
      if(v.IsString()) {
        const char* result = v.GetString();
        return std::string(result ? result : "", v.GetStringLength());
      }

      return std::string();
    }

    const void* GetStringData(size_t* size) const {
      auto& v = value();
      if(v.IsString()) {
        const char* result = v.GetString();

        if(result) {
          if(size) {
            *size = v.GetStringLength();
          }

          return result;
        }
      }

      if(size) {
        *size = 0;
      }

      return nullptr;
    }

    template <class T>
    T GetEnum() {
      return (T) GetInt();
    }

    size_t GetSizeT() const {
      ogalibAssert(sizeof(size_t) <= sizeof(uint64_t), "Type size_t is too large to be supported by json values.");
      if(sizeof(size_t) > sizeof(uint64_t)) {
        return 0;
      }

      auto& v = value();
      if(v.IsUint64()) {
        return v.GetUint64();
      }

      return 0;
    }

    void* GetVoidPtr() const {
      ogalibAssert(sizeof(intptr_t) <= sizeof(int64_t), "Type intptr_t is too large to be supported by json values.");
      if(sizeof(intptr_t) > sizeof(int64_t)) {
        return nullptr;
      }

      auto& v = value();
      if(v.IsInt64()) {
        return (void*) v.GetInt64();
      }

      return nullptr;
    }

    template <class T>
    T* GetPtr() const {
      return static_cast<T*>(GetVoidPtr());
    }

    iterator& SetObject() {
      value().SetObject();
      return *this;
    }

    iterator& SetArray() {
      value().SetArray();
      return *this;
    }

    size_t size() const {
      auto& v = value();
      if(v.IsArray()) {
        return v.Size();
      }
      else {
        return 0;
      }
    }

    iterator& append(int v) {
      auto& arr = value();
      if(arr.IsArray()) {
        arr.PushBack(v, js.doc.GetAllocator());
      }
      return *this;
    }

    iterator& append(const char* v) {
      auto& arr = value();
      if(arr.IsArray()) {
        arr.PushBack(rapidjson::Value(v, js.doc.GetAllocator()), js.doc.GetAllocator());
      }
      return *this;
    }

    iterator& append(const json& v) {
      auto& arr = value();
      if(arr.IsArray()) {
        arr.PushBack(rapidjson::Value().CopyFrom(v.doc, js.doc.GetAllocator()), js.doc.GetAllocator());
      }
      return *this;
    }

    iterator& append(const iterator& v) {
      auto& arr = value();
      if(arr.IsArray()) {
        arr.PushBack(rapidjson::Value().CopyFrom(v.value(), js.doc.GetAllocator()), js.doc.GetAllocator());
      }
      return *this;
    }

    std::string tostring() const;
  };

  class const_iterator {
  friend class json;
  friend class iterator;
  private:

    const json& js;
    rapidjson::Document::ConstMemberIterator iter;
    rapidjson::Document::ConstMemberIterator iterEnd;
    rapidjson::Document::ConstValueIterator valueIter;
    rapidjson::Document::ConstValueIterator valueIterEnd;

  public:

    const_iterator(const json& js): js(js), iter(ConstNullIter), iterEnd(ConstNullIter), valueIter(ConstNullValueIter), valueIterEnd(ConstNullValueIter) {}
    const_iterator(const json& js, const rapidjson::Document::ConstMemberIterator& nativeIter, const rapidjson::Document::ConstMemberIterator& nativeIterEnd): js(js), iter(nativeIter), iterEnd(nativeIterEnd), valueIter(ConstNullValueIter), valueIterEnd(ConstNullValueIter) {}
    const_iterator(const json& js, const rapidjson::Document::ConstValueIterator& nativeIter, const rapidjson::Document::ConstValueIterator& nativeIterEnd): js(js), iter(ConstNullIter), iterEnd(ConstNullIter), valueIter(nativeIter), valueIterEnd(nativeIterEnd) {}

  private:

    const_iterator(const const_iterator& other): js(other.js), iter(other.iter), iterEnd(other.iterEnd), valueIter(other.valueIter), valueIterEnd(other.valueIterEnd) {}

  public:

    operator bool() const {
      if(iter != ConstNullIter) {
        return iter != iterEnd;
      }
      else if(valueIter != ConstNullValueIter) {
        return valueIter != valueIterEnd;
      }

      return false;
    }

    const_iterator& operator=(const const_iterator& other) {
      ogalibAssert(&js == &other.js, "Cannot copy iterator from another structure.");
      if(iter != NullIter) {
        iter = other.iter;
      }
      else if(valueIter != NullValueIter) {
        valueIter = other.valueIter;
      }
      return *this;
    }

    const_iterator& operator++() {
      if(iter != NullIter) {
        ++iter;
      }
      else if(valueIter != NullValueIter) {
        ++valueIter;
      }
      return *this;
    }

    const_iterator& operator*() {
      return *this;
    }

    bool operator==(const const_iterator& other) const {
      if(iter != NullIter) {
        return iter == other.iter;
      }
      else if(valueIter != NullValueIter) {
        return valueIter == other.valueIter;
      }
      else {
        return false;
      }
    }

    bool operator==(const iterator& other) const {
      if(iter != NullIter) {
        return iter != other.iter;
      }
      else if(valueIter != NullValueIter) {
        return valueIter != other.valueIter;
      }
      else {
        return false;
      }
    }

    bool operator!=(const const_iterator& other) const {
      if(iter != NullIter) {
        return iter != other.iter;
      }
      else if(valueIter != NullValueIter) {
        return valueIter != other.valueIter;
      }
      else {
        return false;
      }
    }

    bool operator!=(const iterator& other) const {
      if(iter != NullIter) {
        return iter != other.iter;
      }
      else if(valueIter != NullValueIter) {
        return valueIter != other.valueIter;
      }
      else {
        return false;
      }
    }

    const rapidjson::Value& key() const {
      if(iter != NullIter) {
        return iter->name;
      }
      else {
        static rapidjson::Value noValue;
        return noValue;
      }
    }

    const rapidjson::Value& value() const {
      if(iter != NullIter) {
        return iter->value;
      }
      else if(valueIter != NullValueIter) {
        return *valueIter;
      }
      else {
        static rapidjson::Value noValue;
        return noValue;
      }
    }

    const char* c_str() const {
      if(iter->value.IsString()) {
        return iter->value.GetString();
      }
      else {
        return nullptr;
      }
    }

    const_iterator find(const char* v) const {
      auto& val = value();
      if(val.IsObject()) {
        auto it = val.FindMember(v);
        auto itEnd = val.MemberEnd();
        if(it == itEnd) {
          return const_iterator(js);
        }
        else {
          return const_iterator(js, it, itEnd);
        }
      }
      else {
        return const_iterator(js);
      }
    }

    const_iterator find(const std::string& v) const {
      auto& val = value();
      if(val.IsObject()) {
        auto it = val.FindMember(v);
        auto itEnd = val.MemberEnd();
        if(it == itEnd) {
          return const_iterator(js);
        }
        else {
          return const_iterator(js, it, itEnd);
        }
      }
      else {
        return const_iterator(js);
      }
    }

    const_iterator at(size_t index) const {
      auto& v = value();
      if(v.IsArray()) {
        if(index >= v.Size()) {
          return const_iterator(js);
        }
        else {
          return const_iterator(js, v.Begin() + index, v.End());
        }
      }
      else {
        return const_iterator(js);
      }
    }

    const_iterator begin() const {
      auto& v = value();
      if(v.IsObject()) {
        return const_iterator(js, v.MemberBegin(), v.MemberEnd());
      }
      else if(v.IsArray()) {
        return const_iterator(js, v.Begin(), v.End());
      }
      else {
        return const_iterator(js);
      }
    }

    const_iterator end() const {
      auto& v = value();
      if(v.IsObject()) {
        return const_iterator(js, v.MemberEnd(), v.MemberEnd());
      }
      else if(v.IsArray()) {
        return const_iterator(js, v.End(), v.End());
      }
      else {
        return const_iterator(js);
      }
    }

    bool IsObject() const {
      return value().IsObject();
    }

    bool IsArray() const {
      return value().IsArray();
    }

    bool IsBool() const {
      return value().IsBool();
    }

    bool IsInt() const {
      return value().IsInt();
    }

    bool IsUint() const {
      return value().IsUint();
    }

    bool IsInt64() const {
      return value().IsInt64();
    }

    bool IsUint64() const {
      return value().IsUint64();
    }

    bool IsNumber() const {
      return value().IsNumber();
    }

    bool IsDouble() const {
      return value().IsDouble();
    }

    bool IsString() const {
      return value().IsString();
    }

    bool IsNull() const {
      return value().IsNull();
    }

    bool IsSizeT() const {
      ogalibAssert(sizeof(size_t) <= sizeof(uint64_t), "Type size_t is too large to be supported by json values.");
      if(sizeof(size_t) > sizeof(uint64_t)) {
        return false;
      }

      return value().IsUint64();
    }

    bool IsVoidPtr() const {
      ogalibAssert(sizeof(intptr_t) <= sizeof(uint64_t), "Type intptr_t is too large to be supported by json values.");
      if(sizeof(intptr_t) > sizeof(uint64_t)) {
        return false;
      }

      return value().IsUint64();
    }

    bool GetBool() const {
      auto& v = value();
      if(v.IsBool()) {
        return v.GetBool();
      }

      return false;
    }


    int GetInt() const {
      auto& v = value();
      if(v.IsInt()) {
        return v.GetInt();
      }

      return 0;
    }

    long long GetInt64() const {
      auto& v = value();
      if(v.IsInt64()) {
        return v.GetInt64();
      }

      return 0;
    }

    unsigned int GetUint() const {
      auto& v = value();
      if(v.IsUint()) {
        return v.GetUint();
      }

      return 0;
    }

    unsigned long long GetUint64() const {
      auto& v = value();
      if(v.IsUint64()) {
        return v.GetUint64();
      }

      return 0;
    }

    float GetFloat() const {
      auto& v = value();
      if(v.IsNumber()) {
        return v.GetFloat();
      }

      return 0.0f;
    }

    double GetDouble() const {
      auto& v = value();
      if(v.IsDouble()) {
        return v.GetDouble();
      }

      return 0.0;
    }

    std::string GetString() const {
      auto& v = value();
      if(v.IsString()) {
        const char* result = v.GetString();
        return std::string(result ? result : "", v.GetStringLength());
      }

      return std::string();
    }

    const void* GetStringData(size_t* size) const {
      auto& v = value();
      if(v.IsString()) {
        const char* result = v.GetString();

        if(result) {
          if(size) {
            *size = v.GetStringLength();
          }

          return result;
        }
      }

      if(size) {
        *size = 0;
      }

      return nullptr;
    }

    template <class T>
    T GetEnum() {
      return (T) GetInt();
    }

    size_t GetSizeT() const {
      ogalibAssert(sizeof(size_t) <= sizeof(uint64_t), "Type size_t is too large to be supported by json values.");
      if(sizeof(size_t) > sizeof(uint64_t)) {
        return 0;
      }

      auto& v = value();
      if(v.IsUint64()) {
        return v.GetUint64();
      }

      return 0;
    }

    void* GetVoidPtr() const {
      ogalibAssert(sizeof(intptr_t) <= sizeof(uint64_t), "Type intptr_t is too large to be supported by json values.");
      if(sizeof(intptr_t) > sizeof(uint64_t)) {
        return nullptr;
      }

      auto& v = value();
      if(v.IsInt64()) {
        return (void*) (intptr_t) v.GetInt64();
      }

      return nullptr;
    }

    template <class T>
    T* GetPtr() const {
      return static_cast<T*>(GetVoidPtr());
    }

    size_t size() const {
      auto& v = value();
      if(v.IsArray()) {
        return v.GetArray().Size();
      }
      else {
        return 0;
      }
    }

    std::string tostring() const;
  };

private:

  rapidjson::Document doc;
  json::iterator iter;
  json::const_iterator constIter;
  std::string err;

public:

  const char* error() const {return err.c_str();}

public:

  json();
  json(const json& v);
  json(const std::initializer_list<jsonbuilder::builder::field_holder>& v);
  json(const json::iterator& it);
  json(const json::const_iterator& it);
  virtual ~json();

public:

  json& operator=(bool v);
  json& operator=(int v);
  json& operator=(unsigned int v);
  json& operator=(long long v);
  json& operator=(size_t v);
  json& operator=(float v);
  json& operator=(double v);
  json& operator=(const char* v);
  json& operator=(const json& v);
  json& operator=(const std::initializer_list<jsonbuilder::builder::field_holder>& v);
  json& operator=(const json::iterator& it);
  json& operator=(const json::const_iterator& it);

  json& operator+=(const json& v);

  json operator+(const json& v) const;

  iterator operator[](const char* v);
  iterator operator[](const std::string& v);
  iterator operator[](const json& v);
  const_iterator operator[](const char* v) const;
  const_iterator operator[](const std::string& v) const;
  const_iterator operator[](const json& v) const;

  bool parse(const char* v);
  bool parse(const std::string& v);
  bool parse(const void* data, size_t dataSize);

  json& append(int v);
  json& append(const char* v);
  json& append(const json& v);
  json& append(const json::iterator& v);

  iterator find(const char* v);
  iterator find(const std::string& v);
  const_iterator find(const char* v) const;
  const_iterator find(const std::string& v) const;

  iterator at(size_t v);
  const_iterator at(size_t v) const;

  json& erase(const char* v);
  json& erase(const std::string& v);
  json& erase(const iterator& it);

  json& clear();

  size_t size() const;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;

  bool IsObject() const;
  bool IsArray() const;
  bool IsBool() const;
  bool IsInt() const;
  bool IsUint() const;
  bool IsInt64() const;
  bool IsUint64() const;
  bool IsNumber() const;
  bool IsDouble() const;
  bool IsString() const;
  bool IsNull() const;

  bool GetBool() const;
  int GetInt() const;
  unsigned int GetUint() const;
  long long GetInt64() const;
  unsigned long long GetUint64() const;
  float GetFloat() const;
  double GetDouble() const;
  std::string GetString() const;

  std::string tostring() const;

public:

  static json array();
  static json object();

};

};
