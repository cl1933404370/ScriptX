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

#include <ScriptX/ScriptX.h>

namespace script {

StringHolder::StringHolder(const Local<String>&string) {
  internalHolder_.string =
      PyBytes_AsString(string.val_);
}

StringHolder::~StringHolder() = default;

size_t StringHolder::length() const { return internalHolder_.len; }

const char *StringHolder::c_str() const { return internalHolder_.string; }

std::string_view StringHolder::stringView() const { return internalHolder_.string; }

std::string StringHolder::string() const { return internalHolder_.string; }

#if defined(__cpp_char8_t)
// NOLINTNEXTLINE(clang-analyzer-cplusplus.InnerPointer)
std::u8string StringHolder::u8string() const {
  const std::u8string str = {c_u8str(), length()};
  return str;
}

std::u8string_view StringHolder::u8stringView() const {
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.InnerPointer)
  const std::u8string_view temp  = {c_u8str(), length()};
  return temp;
}

const char8_t *StringHolder::c_u8str() const { return reinterpret_cast<const char8_t *>(c_str()); }
#endif

}  // namespace script
