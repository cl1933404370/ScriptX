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

Local<Object> Object::newObjectImpl(const Local<Value>& type, const size_t size,
                                    const Local<Value>* args) {
  // TEMPLATE_NOT_IMPLEMENTED()
  PyObject* pValue = nullptr;
  PyObject* pArgs = PyTuple_New(static_cast<Py_ssize_t>(size));
  for (size_t i = 0; i < size; ++i) {
    switch (Local<Value> value = *args; value.getKind()) {
      case ValueKind::kString: {
        auto ss = value.asString().toString();
        auto tt1 = ss.c_str();
        pValue = PyBytes_FromString(tt1);
        if (!PyBytes_Check(pValue)) {
          assert(0);
        }
      }
      break;
      case ValueKind::kNumber: {
        const long aa = static_cast<long>(value.asNumber().toInt64());
        pValue = PyLong_FromLong(aa);
        if (!PyNumber_Check(pValue)) {
          assert(0);
        }
      }
      break;
      case ValueKind::kNull:
        break;
      case ValueKind::kObject:
        break;
      case ValueKind::kBoolean:
        break;
      case ValueKind::kFunction:
        break;
      case ValueKind::kArray:
        break;
      case ValueKind::kByteBuffer:
        break;
      case ValueKind::kUnsupported:
        break;
      default:
        break;
    }
    if (!pValue) {
      Py_DECREF(pArgs);
      [[maybe_unused]] int a = fprintf(stderr, "Cannot convert argument\n");
      return Local<Object>{nullptr};
    }
    PyTuple_SetItem(pArgs, static_cast<Py_ssize_t>(i), pValue);
    ++args;
  }
  const auto copyFun = type.asObject().val_;
  PyObject* classValue = PyObject_CallObject(copyFun, pArgs);
  Py_DECREF(pArgs);
  Py_IncRef(copyFun);
  if (classValue != nullptr) {
    // Py_DECREF(pValue);
  } else {
    PyErr_Print();
    [[maybe_unused]] int a = fprintf(stderr, "Call failed\n");
    return Local<Object>{nullptr};
  }
  return Local<Object>{classValue};
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
  return checkAndMakeLocal<Number>(PyLong_FromLong(value));
}

Local<Number> Number::newNumber(int64_t value) {
  return checkAndMakeLocal<Number>(PyLong_FromLongLong(value));
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
} // namespace

Local<Function> Function::newFunction(FunctionCallback callback) {
  auto callback_ins = std::make_unique<FunctionData>();
  callback_ins->engine = EngineScope::currentEngineAs<py_backend::PyEngine>();
  callback_ins->function = std::move(callback);

  PyMethodDef method;
  method.ml_name = "ScriptX_native_method";
  method.ml_flags = METH_O;
  method.ml_doc = "ScriptX Function::newFunction";
  method.ml_meth = [](PyObject* self, PyObject* args) -> PyObject* {
    if (const auto ptr = PyCapsule_GetPointer(self, kFunctionDataName); ptr == nullptr) {
      PyErr_SetString(PyExc_TypeError, "invalid 'self' for native method");
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
  checkException(ctx);
  callback_ins.reset();

  PyObject* closure = PyCFunction_New(&method, ctx);
  Py_XDECREF(ctx);
  checkException(closure);

  return Local<Function>(closure);
}

Local<Array> Array::newArray(size_t size) {
  return checkAndMakeLocal<Array>(PyList_New(static_cast<Py_ssize_t>(size)));
}

Local<Array> Array::newArrayImpl(size_t size, const Local<Value>* args) {
  TEMPLATE_NOT_IMPLEMENTED()
}

Local<ByteBuffer> ByteBuffer::newByteBuffer(size_t size) { TEMPLATE_NOT_IMPLEMENTED() }

Local<ByteBuffer> ByteBuffer::newByteBuffer(void* nativeBuffer,
                                            size_t size) { TEMPLATE_NOT_IMPLEMENTED() }

Local<ByteBuffer> ByteBuffer::newByteBuffer(
    [[maybe_unused]] const std::shared_ptr<void>& nativeBuffer, size_t size) {
  TEMPLATE_NOT_IMPLEMENTED()
}
} // namespace script
