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

#include "../../src/Exception.h"
#include "../../src/Reference.h"
#include "../../src/Scope.h"
#include "../../src/Value.h"
#include "PyHelper.hpp"

using script::py_interop;
using script::py_backend::checkException;

namespace script {

template <typename T>
Local<T> checkAndMakeLocal(PyObject* ref) {
  return py_interop::makeLocal<T>(checkException(ref));
}

// for python this creates an empty dict
Local<Object> Object::newObject() { return checkAndMakeLocal<Object>(PyDict_New()); }

Local<Object> Object::newObjectImpl(const Local<Value>& type, size_t size,
                                    const Local<Value>* args) {
  //TEMPLATE_NOT_IMPLEMENTED()
 /* PyObject* args = Py_BuildValue("(ii)", 28, 103);
  PyObject* pRet = PyObject_CallObject(type, args );
  Py_DECREF(args);*/
  return Local<Object>{nullptr};
}

Local<String> String::newString(const char* utf8) {
  return checkAndMakeLocal<String>(PyBytes_FromString(utf8));
}

Local<String> String::newString(std::string_view utf8) {
  return checkAndMakeLocal<String>(
      PyBytes_FromStringAndSize(utf8.data(), static_cast<Py_ssize_t>(utf8.length())));
}

Local<String> String::newString(const std::string& utf8) {
  return checkAndMakeLocal<String>(
      PyBytes_FromStringAndSize(utf8.c_str(), static_cast<Py_ssize_t>(utf8.length())));
}

#if defined(__cpp_char8_t)

Local<String> String::newString(const char8_t* utf8) {
  return newString(reinterpret_cast<const char*>(utf8));
}

Local<String> String::newString(std::u8string_view utf8) {
  return newString(std::string_view(reinterpret_cast<const char*>(utf8.data()), utf8.length()));
}

Local<String> String::newString(const std::u8string& utf8) { return newString(utf8.c_str()); }

#endif

Local<Number> Number::newNumber(float value) { return newNumber(static_cast<double>(value)); }

Local<Number> Number::newNumber(double value) {
  return checkAndMakeLocal<Number>(PyLong_FromDouble(value));
}

Local<Number> Number::newNumber(int32_t value) {
  return checkAndMakeLocal<Number>(PyLong_FromLong(static_cast<long>(value)));
}

Local<Number> Number::newNumber(int64_t value) {
  return checkAndMakeLocal<Number>(PyLong_FromLongLong(static_cast<long long>(value)));
}

Local<Boolean> Boolean::newBoolean(bool value) {
  return checkAndMakeLocal<Boolean>(PyBool_FromLong(value));
}

namespace {
constexpr const char* kFunctionDataName = "_ScriptX_function_data";

struct FunctionData {
  FunctionCallback function;
  py_backend::PyEngine* engine = nullptr;
};

}  // namespace

Local<Function> Function::newFunction(script::FunctionCallback callback) {
  auto callback_ins = std::make_unique<FunctionData>();
  callback_ins->engine = EngineScope::currentEngineAs<py_backend::PyEngine>();
  callback_ins->function = std::move(callback);

  PyMethodDef method;
  method.ml_name = "ScriptX_native_method";
  method.ml_flags = METH_O;
  method.ml_doc = "ScriptX Function::newFunction";
  method.ml_meth = [](PyObject* self, PyObject* args) -> PyObject* {
    if (const auto ptr = PyCapsule_GetPointer(self, kFunctionDataName); ptr == nullptr) {
      ::PyErr_SetString(PyExc_TypeError, "invalid 'self' for native method");
    } else {
      const auto data = static_cast<FunctionData*>(ptr);
      try {
        const auto ret = data->function(py_interop::makeArguments(nullptr, self, args));
        return py_interop::toPy(ret);
      } catch (Exception& e) {
        py_backend::rethrowException(e);
      }
    }
    return nullptr;
  };

  const auto ctx = PyCapsule_New(callback_ins.get(), kFunctionDataName, [](PyObject* cap) {
    const auto ptr = PyCapsule_GetPointer(cap, kFunctionDataName);
    delete static_cast<FunctionData*>(ptr);
  });
  py_backend::checkException(ctx);
  callback_ins.reset();

  PyObject* closure = PyCFunction_New(&method, ctx);
  Py_XDECREF(ctx);
  py_backend::checkException(closure);

  return Local<Function>(closure);
}

Local<Array> Array::newArray(size_t size) {
  return checkAndMakeLocal<Array>(PyList_New(static_cast<Py_ssize_t>(size)));
}

Local<Array> Array::newArrayImpl(size_t size, const Local<Value>* args) {
  TEMPLATE_NOT_IMPLEMENTED()
}

Local<ByteBuffer> ByteBuffer::newByteBuffer(size_t size) { TEMPLATE_NOT_IMPLEMENTED()}

Local<script::ByteBuffer> ByteBuffer::newByteBuffer(void* nativeBuffer, size_t size) {
  TEMPLATE_NOT_IMPLEMENTED()
}

Local<ByteBuffer> ByteBuffer::newByteBuffer([[maybe_unused]] const std::shared_ptr<void>&
                                            nativeBuffer, size_t size) {
  TEMPLATE_NOT_IMPLEMENTED()
}

}  // namespace script