/*
 * Tencent is pleased to support the open source community by making ScriptX available.
 * Copyright (C) 2021 THL A29 Limited, a Tencent company.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "PyHelper.hpp"
#include "PyReference.hpp"
#include "../../src/Native.hpp"
#include "../../src/Reference.h"
#include "../../src/Utils.h"
#include "../../src/Value.h"

namespace script
{
  namespace py_backend
  {
    void valueConstructorCheck(const PyObject *value)
    {
      SCRIPTX_UNUSED(value);
#ifndef NDEBUG
      if (!value)
        throw Exception("null reference");
#endif
    }
  } // namespace py_backend

#define REF_IMPL_BASIC_FUNC(ValueType)                                                        \
  Local<ValueType>::Local(const Local<ValueType> &copy) : val_(py_backend::incRef(copy.val_)) \
  {                                                                                           \
  }                                                                                           \
  Local<ValueType>::Local(Local<ValueType> &&move) noexcept : val_(move.val_)                 \
  {                                                                                           \
    move.val_ = nullptr;                                                                      \
  }                                                                                           \
  Local<ValueType>::~Local()                                                                  \
  {                                                                                           \
    py_backend::decRef(val_);                                                                 \
  }                                                                                           \
  Local<ValueType> &Local<ValueType>::operator=(const Local &from)                            \
  {                                                                                           \
    Local(from).swap(*this);                                                                  \
    return *this;                                                                             \
  }                                                                                           \
  Local<ValueType> &Local<ValueType>::operator=(Local &&move) noexcept                        \
  {                                                                                           \
    Local(std::move(move)).swap(*this);                                                       \
    return *this;                                                                             \
  }                                                                                           \
  void Local<ValueType>::swap(Local &rhs) noexcept                                            \
  {                                                                                           \
    std::swap(val_, rhs.val_);                                                                \
  }

#define REF_IMPL_BASIC_EQUALS(ValueType)                                             \
  bool Local<ValueType>::operator==(const script::Local<script::Value> &other) const \
  {                                                                                  \
    return asValue() == other;                                                       \
  }

#define REF_IMPL_BASIC_NOT_VALUE(ValueType)                 \
  Local<ValueType>::Local(InternalLocalRef val) : val_(val) \
  {                                                         \
    py_backend::valueConstructorCheck(val);                 \
  }                                                         \
  Local<String> Local<ValueType>::describe() const          \
  {                                                         \
    return asValue().describe();                            \
  }                                                         \
  std::string Local<ValueType>::describeUtf8() const        \
  {                                                         \
    return asValue().describeUtf8();                        \
  }

#define REF_IMPL_TO_VALUE(ValueType)               \
  Local<Value> Local<ValueType>::asValue() const   \
  {                                                \
    return Local<Value>(py_backend::incRef(val_)); \
  }

  REF_IMPL_BASIC_FUNC(Value)

  REF_IMPL_BASIC_FUNC(Object)
  REF_IMPL_BASIC_NOT_VALUE(Object)
  REF_IMPL_BASIC_EQUALS(Object)
  REF_IMPL_TO_VALUE(Object)

  REF_IMPL_BASIC_FUNC(String)
  REF_IMPL_BASIC_NOT_VALUE(String)
  REF_IMPL_BASIC_EQUALS(String)
  REF_IMPL_TO_VALUE(String)

  REF_IMPL_BASIC_FUNC(Number)
  REF_IMPL_BASIC_NOT_VALUE(Number)
  REF_IMPL_BASIC_EQUALS(Number)
  REF_IMPL_TO_VALUE(Number)

  REF_IMPL_BASIC_FUNC(Boolean)
  REF_IMPL_BASIC_NOT_VALUE(Boolean)
  REF_IMPL_BASIC_EQUALS(Boolean)
  REF_IMPL_TO_VALUE(Boolean)

  REF_IMPL_BASIC_FUNC(Function)
  REF_IMPL_BASIC_NOT_VALUE(Function)
  REF_IMPL_BASIC_EQUALS(Function)
  REF_IMPL_TO_VALUE(Function)

  REF_IMPL_BASIC_FUNC(Array)
  REF_IMPL_BASIC_NOT_VALUE(Array)
  REF_IMPL_BASIC_EQUALS(Array)
  REF_IMPL_TO_VALUE(Array)

  REF_IMPL_BASIC_FUNC(ByteBuffer)
  REF_IMPL_BASIC_NOT_VALUE(ByteBuffer)
  REF_IMPL_BASIC_EQUALS(ByteBuffer)
  REF_IMPL_TO_VALUE(ByteBuffer)

  REF_IMPL_BASIC_FUNC(Unsupported)
  REF_IMPL_BASIC_NOT_VALUE(Unsupported)
  REF_IMPL_BASIC_EQUALS(Unsupported)
  REF_IMPL_TO_VALUE(Unsupported)

  // ==== value ====

  Local<Value>::Local() noexcept
      : val_()
  {
  }

  Local<Value>::Local(InternalLocalRef ref)
      : val_(ref)
  {
  }

  bool Local<Value>::isNull() const {
    return val_ == nullptr;
  }

  void Local<Value>::reset()
  {
    py_backend::decRef(val_);
    val_ = nullptr;
  }

  ValueKind Local<Value>::getKind() const
  {
    if (isNull())
    {
      return ValueKind::kNull;
    }
    if (isString())
    {
      return ValueKind::kString;
    }
    if (isNumber())
    {
      return ValueKind::kNumber;
    }
    if (isBoolean())
    {
      return ValueKind::kBoolean;
    }
    if (isFunction())
    {
      return ValueKind::kFunction;
    }
    if (isArray())
    {
      return ValueKind::kArray;
    }
    if (isByteBuffer())
    {
      return ValueKind::kByteBuffer;
    }
    if (isObject())
    {
      return ValueKind::kObject;
    }
    return ValueKind::kUnsupported;
  }

  bool Local<Value>::isString() const
  {
    return PyBytes_Check(val_);
  }

  bool Local<Value>::isNumber() const { return PyNumber_Check(val_); }

  bool Local<Value>::isBoolean() const { return PyBool_Check(val_); }

  bool Local<Value>::isFunction() const { return PyFunction_Check(val_); }

  bool Local<Value>::isArray() { return false; }

  bool Local<Value>::isByteBuffer() { return false; }

  bool Local<Value>::isObject() const
  {
    // todo
    return val_ and PyCallable_Check(val_);
  }

  bool Local<Value>::isUnsupported() { return false; }

  Local<String> Local<Value>::asString() const
  {
    [[maybe_unused]] auto &&chars = PyBytes_AsString(val_);
    [[maybe_unused]] auto &&linestring = String::newString(PyBytes_AsString(val_));
    return String::newString(PyBytes_AsString(val_));
  }

  Local<Number> Local<Value>::asNumber() const
  {
    return Number::newNumber(static_cast<int32_t>(PyLong_AsLong(val_)));
  }

  Local<Boolean> Local<Value>::asBoolean() { throw Exception("can't cast value as Boolean"); }

  Local<Function> Local<Value>::asFunction() const
  {
    throw Exception("can't cast value as Function");
    //return Function::newFunction(val_);
  }

  Local<Array> Local<Value>::asArray() const
  {
    return Array::newArray(0);
  }

  Local<ByteBuffer> Local<Value>::asByteBuffer()
  {
    throw Exception("can't cast value as ByteBuffer");
  }

  Local<Object> Local<Value>::asObject() const
  {
    return Local<Object>{val_};
  }

  Local<Unsupported> Local<Value>::asUnsupported()
  {
    throw Exception("can't cast value as Unsupported");
  }

  bool Local<Value>::operator==(const Local<Value> &other) const
  {
    // TODO: nullptr vs None
    const auto lhs = val_;
    const auto rhs = other.val_;

    // nullptr == nullptr
    if (lhs == nullptr || rhs == nullptr)
    {
      return lhs == rhs;
    }

    return PyObject_RichCompareBool(lhs, rhs, Py_EQ);
  }

  Local<String> Local<Value>::describe() const {TEMPLATE_NOT_IMPLEMENTED()}

  Local<Value> Local<Object>::get(const Local<String> &key)
  {
    return {};
  }

  void Local<Object>::set(const Local<String> &key,
                          const Local<Value> &value)
  {
  }

  void Local<Object>::remove(const Local<class String> &key)
  {
  }

  bool Local<Object>::has(const Local<class String> &key) { return true; }

  bool Local<Object>::instanceOf(const Local<class Value> &type) { return false; }

  std::vector<Local<String>> Local<Object>::getKeys() { return {}; }

  float Local<Number>::toFloat() const { return static_cast<float>(toDouble()); }

  double Local<Number>::toDouble() const
  {
    return PyLong_AsDouble(val_);
  }

  int32_t Local<Number>::toInt32() const { return static_cast<int32_t>(toDouble()); }

  int64_t Local<Number>::toInt64() const { return static_cast<int64_t>(toDouble()); }

  bool Local<Boolean>::value() { return false; }

  Local<Value> Local<Function>::callImpl([[maybe_unused]] const Local<Value> &thiz, const size_t size,
                                         const Local<Value> *args) const
  {
    // PyObject* self = thiz.isObject() ? py_interop::toPy(thiz) : nullptr;
    // TODO: selfs
    PyObject *ret = nullptr;
    if (!PyCallable_Check(py_interop::asPy(*this)))
    {
      PyErr_Format(PyExc_TypeError,
                   "attribute of type '%.200s' is not callable",
                   Py_TYPE(py_interop::asPy(*this))->tp_name);
      return Local<Value>(ret);
    }
    // args to tuple
    if (size == 0)
    {
      ret = PyObject_CallNoArgs(py_interop::asPy(*this));
    }
    else if (size == 1)
    {
      if (PyTuple_Check(py_interop::asPy(args[0])))
      {
        ret = PyObject_CallOneArg(py_interop::asPy(*this), py_interop::asPy(args[0]));
      }
      else
      {
        
      }
    }
    else
    {
      PyObject *tuple = PyTuple_New(static_cast<Py_ssize_t>(size));
      py_backend::checkException();
      for (size_t i = 0; i < size; ++i)
      {
        PyTuple_SetItem(tuple, static_cast<Py_ssize_t>(i), py_interop::toPy(args[i]));
        py_backend::checkException();
      }
      if (!PyTuple_Check(tuple))
      {
        
      }
      ret = PyObject_Call(py_interop::asPy(*this), tuple, PyDict_New());
    }

    py_backend::checkException();
    return Local<Value>(ret);
  }

  size_t Local<Array>::size() { return 0; }

  Local<Value> Local<Array>::get(size_t index) { return {}; }

  void Local<Array>::set(size_t index, const Local<Value> &value)
  {
  }

  void Local<Array>::add(const Local<Value> &value) const { set(size(), value); }

  void Local<Array>::clear()
  {
  }

  ByteBuffer::Type Local<ByteBuffer>::getType() { return ByteBuffer::Type::KFloat32; }

  bool Local<ByteBuffer>::isShared() { return true; }

  void Local<ByteBuffer>::commit()
  {
  }

  void Local<ByteBuffer>::sync()
  {
  }

  size_t Local<ByteBuffer>::byteLength() { return 0; }

  void *Local<ByteBuffer>::getRawBytes() { return nullptr; }

  std::shared_ptr<void> Local<ByteBuffer>::getRawBytesShared() { return {}; }
} // namespace script
