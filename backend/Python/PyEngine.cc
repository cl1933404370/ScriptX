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

#include "PyEngine.h"

#include "../../src/Utils.h"

namespace script::py_backend {
PyEngine::PyEngine(std::shared_ptr<utils::MessageQueue> queue)
  : queue_(queue ? std::move(queue) : std::make_shared<utils::MessageQueue>()) {
  Py_Initialize();
}

PyEngine::PyEngine()
  : PyEngine(nullptr) {
}

PyEngine::~PyEngine() = default;

void PyEngine::destroy() noexcept { destroyUserData(); }

Local<Value> PyEngine::get(const Local<String>& key) { return {}; }

void PyEngine::set(const Local<String>& key, const Local<Value>& value) {
}

Local<Value> PyEngine::eval(const Local<String>& script) { return eval(script, Local<Value>()); }

Local<Value> PyEngine::eval(const Local<String>& script, const Local<String>& sourceFile) {
  return eval(script, sourceFile.asValue());
}

Local<Value> PyEngine::eval(const Local<String>& script, const Local<Value>& sourceFile) {
  Tracer trace(this, "PyEngine::eval");
  const auto source_string_holder = script.toString();
  std::string source_file_name;
  if (sourceFile.isString()) {
    source_file_name = sourceFile.asString().toString();
  }
  if (source_file_name.empty()) {
    source_file_name = "unknown.py";
  }
  if (!Py_IsInitialized())
    Py_Initialize();
  try {
    const char* s = const_cast<char*>(source_string_holder.c_str());
    const auto main = PyModule_New("SCRIPTX");
    PyObject* main_dict = PyModule_GetDict(PyImport_AddModule("__main__"));
    PyModule_AddObject(main, "__builtins__", PyDict_GetItemString(main_dict, "__builtins__"));
    const auto scope = PyObject_GetAttrString(main, "__dict__");
    if (const auto run_return = PyRun_String(s, Py_file_input, scope, scope);
      run_return == nullptr) {
      PyErr_Print();
      PyErr_Clear();
      Py_DECREF(run_return);
      return Local<Value>{};
    } else {
      Py_DECREF(run_return);
    }
    PyObject* functions = PyModule_GetDict(main);
    PyObject* aa7 = PyList_GetItem(PyDict_Values(functions), PyDict_Size(functions) - 1);
    //https://pythonextensionpatterns.readthedocs.io/en/latest/cpp_and_cpython.html
    if (!(PyErr_Occurred() == nullptr || !aa7)) {
      return Local<Value>{};
    }
    if (!PyCallable_Check(aa7)) {
      PyErr_Print();
      PyErr_Clear();
      return Local<Value>{};
    }
    //todo don't no how to use
    //Py_Finalize();
    return Local<Value>{aa7};
  } catch (...) {
    PyErr_Print();
    return Local<Value>{};
  }
}

std::shared_ptr<utils::MessageQueue> PyEngine::messageQueue() { return queue_; }

void PyEngine::gc() {
}

void PyEngine::adjustAssociatedMemory(int64_t count) {
}

ScriptLanguage PyEngine::getLanguageType() { return ScriptLanguage::kPython; }

std::string PyEngine::getEngineVersion() { return Py_GetVersion(); }

bool PyEngine::isDestroying() const { return false; }
} // namespace script::py_backend
