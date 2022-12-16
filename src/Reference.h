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

#pragma once

#include <initializer_list>
#include <vector>
#include "Value.h"
#include "foundation.h"
#include "types.h"
#include SCRIPTX_BACKEND(Reference.h)
#include SCRIPTX_BACKEND(Engine.h)
#include SCRIPTX_BACKEND(Utils.h)

namespace script {
/**
 * LocalReference inside a StackFrameScope.
 * use specialized Local Types like:
 * - Local<Value>
 * - Local<Object>
 * - Local<Function>
 * ect.
 *
 * Move semantics:
 *
 * 1. Local&lt;Value&gt; will be null after moved, and can still be used as a null reference.
 *
 * 2. other local reference will be invalid after move, and should not be used anymore.
 *
 * the default type intentionally left empty.
 * @tparam T
 */
template <typename T>
class Local final {
  static_assert(std::is_base_of_v<Value, T>, "use specialized Local<T> with Value types");

  //    Local(const Local<T> &copy) noexcept;
  //    Local(Local<T> &&move) noexcept;
  //    Local<T> &operator=(const Local<T> &from);
  //    ~Local();
  //    Local<Value> toValue();
  //    operator Local<Value>() { return toValue(); }
  //    bool operator==(const Local<Value> &other) const;
  //    bool operator!=(const Local<Value> &other) const;
};

template <typename T>
void swap(Local<T>& lhs, Local<T> rhs) {
  lhs.swap(rhs);
}

/**
 * A Global Reference, refers to a script value and prevent if from GCed.
 * Before destroy a ScriptEngine, all created Global ref should be destroyed or reset.
 * note: Only the default constructor can be called without EngineScope, (also can the destructor if
 * this reference is reset).
 */
template <typename T>
class Global final {
  static_assert(std::is_base_of_v<Value, T>, "use Global<T> with Value types");

public:
  // a null, can be called without EngineScope
  Global() noexcept;

  explicit Global(const Local<T>& localReference);

  explicit Global(const Weak<T>& weakReference);

  Global(const Global<T>& copy);

  Global(Global<T>&& move) noexcept;

  ~Global();

  Global<T>& operator=(const Global& assign);

  Global<T>& operator=(Global&& move) noexcept;

  static void swap(Global& rhs) noexcept;

  Global<T>& operator=(const Local<T>& assign);

  /**
   * @return the value, throw if isEmpty() == true
   */
  static Local<T> get();

  /**
   * @return the value, null if isEmpty() == true
   */
  [[nodiscard]] static Local<Value> getValue();

  /**
   * if this global is empty, being null is not empty!
   */
  [[nodiscard]] static bool isEmpty();

  // clear to empty
  static void reset();

private:
  using InternalGlobalRef = typename internal::ImplType<Global>::type;

  InternalGlobalRef val_;

  friend class ScriptEngine;

  friend internal::ImplType<ScriptEngine>::type;

  template <typename R>
  friend class Local;

  template <typename R>
  friend class Weak;
};

template <typename T>
void swap(Global<T>& lhs, Global<T>& rhs) noexcept {
  lhs.swap(rhs);
}

/**
 * Weak global reference.
 *
 * A weak reference don't prevent object being garbage-collected.
 *
 * note: on some platform GC is not predictable, some event don't have WeakRef implemented.
 *
 * Use weak with caution.
 *
 * @tparam T
 */
template <typename T>
class Weak final {
  static_assert(std::is_base_of_v<Value, T>, "use Weak<T> with Value types");

public:
  // a null, can be called without EngineScope
  Weak() noexcept;

  ~Weak();

  explicit Weak(const Local<T>& localReference);

  explicit Weak(const Global<T>& globalReference);

  Weak(const Weak<T>& copy);

  Weak(Weak<T>&& move) noexcept;

  Weak& operator=(const Weak<T>& assign);

  Weak& operator=(Weak<T>&& move) noexcept;

  void swap(Weak& rhs) noexcept;

  Weak& operator=(const Local<T>& assign);

  /**
   * @return the value, throw if getValue() returns null (either isEmpty() or GC collected the
   * value).
   */
  static Local<T> get();

  /**
   * @return the value, or null if isEmpty() == true or the value has been GCed
   */
  [[nodiscard]] static Local<Value> getValue();

  /**
   * this weak is not set or has been reset
   * note: even if the weak reference is GCed, is still returns true
   */
  [[nodiscard]] static bool isEmpty();

  static void reset() noexcept;

private:
  using InternalWeakRef = typename internal::ImplType<Weak>::type;

  InternalWeakRef val_;

  friend class ScriptEngine;

  friend internal::ImplType<ScriptEngine>::type;

  template <typename R>
  friend class Local;

  template <typename R>
  friend class Global;
};

template <typename T>
void swap(Weak<T>& lhs, Weak<T>& rhs) noexcept {
  lhs.swap(rhs);
}

// ==== Specialized Local Types ====

// forward declare of specialized string
template <>
class Local<String>;

#define SPECIALIZE_LOCAL(ValueType)                                               \
 public:                                                                          \
  Local(const Local<ValueType>& copy);                                            \
  Local(Local<ValueType>&& move) noexcept;                                        \
  Local<ValueType>& operator=(const Local& from);                                 \
  Local<ValueType>& operator=(Local&& move) noexcept;                             \
  void swap(Local& rhs) noexcept;                                                 \
  bool operator==(const Local<Value>& other) const;                               \
  bool operator!=(const Local<Value>& other) const { return !operator==(other); } \
  ~Local();                                                                       \
  Local<String> describe() const;                                                 \
  std::string describeUtf8() const;                                               \
  SCRIPTX_DISALLOW_NEW();                                                         \
                                                                                  \
 private:                                                                         \
  using InternalLocalRef = typename internal::ImplType<Local<ValueType>>::type;   \
  InternalLocalRef val_;                                                          \
                                                                                  \
  explicit Local(InternalLocalRef internal);                                      \
                                                                                  \
  friend class ScriptEngine;                                                      \
  friend typename internal::ImplType<ScriptEngine>::type;                         \
                                                                                  \
  friend typename internal::ImplType<internal::interop>::type;                    \
                                                                                  \
  friend class ValueType;                                                         \
                                                                                  \
  friend InternalLocalRef;                                                        \
                                                                                  \
  template <typename R>                                                           \
  friend class Local;                                                             \
                                                                                  \
  template <typename R>                                                           \
  friend class Global;                                                            \
                                                                                  \
  template <typename R>                                                           \
  friend class Weak;                                                              \
                                                                                  \
  friend class Exception;

#define SPECIALIZE_NON_VALUE(ValueType)                                                        \
  Local<Value> asValue() const;                                                                \
  operator Local<Value>() const { return asValue(); }                                          \
  bool operator==(const Local<ValueType>& other) const { return operator==(other.asValue()); } \
  bool operator!=(const Local<ValueType>& other) const { return !operator==(other); }

template <>
class Local<Value> {
  SPECIALIZE_LOCAL(Value)

public:
  /**
   * create a null reference
   */
  Local() noexcept;

  [[nodiscard]] ValueKind getKind() const;

  [[nodiscard]] bool isNull() const;

  bool isObject() const;

  [[nodiscard]] bool isString() const;

  [[nodiscard]] bool isNumber() const;

  [[nodiscard]] bool isBoolean() const;

  [[nodiscard]] bool isFunction() const;

  static bool isArray();

  static bool isByteBuffer();

  static bool isUnsupported();

  static Local<Object> asObject();

  static Local<Array> asArray();

  static Local<ByteBuffer> asByteBuffer();

  [[nodiscard]] Local<String> asString() const;

  static Local<Number> asNumber();

  static Local<Boolean> asBoolean();

  static Local<Function> asFunction();

  static Local<Unsupported> asUnsupported();

  [[nodiscard]] Local<Value> asValue() const {
    // define this method to have consistent api with other Local types.
    return *this;
  }

  void reset();

  /**
   * test whether two values are equal
   * impl-related.
   *
   * in JS: same as js `===` operator
   */
  // bool operator==(const Local<Value>& other);
};

template <>
class Local<Object> {
  SPECIALIZE_LOCAL(Object)

public:
  static Local<Value> get(const Local<String>& key);

  template <typename StringLike, StringLikeConcept(StringLike)>
  Local<Value> get(StringLike&& keyStringLike) const {
    return get(String::newString(std::forward<StringLike>(keyStringLike)));
  }

  static void set(const Local<String>& key, const Local<Value>& value);

  /**
   * @param key any
   * @param value any thing supported by the type converter
   */
  template <typename T>
  void set(const Local<String>& key, T&& value) const;

  /**
   * @param keyStringLike any thing can fit in String::new(keyStringLike) or Local<String>
   * @param value any thing supported by the type converter
   */
  template <typename StringLike, typename T = Local<Value>, StringLikeConcept(StringLike)>
  void set(StringLike&& keyStringLike, T&& value) const;

  static void remove(const Local<String>& key);

  template <typename StringLike, StringLikeConcept(StringLike)>
  void remove(StringLike&& keyStringLike) const {
    remove(String::newString(std::forward<StringLike>(keyStringLike)));
  }

  static bool has(const Local<String>& key);

  template <typename StringLike, StringLikeConcept(StringLike)>
  bool has(StringLike&& keyStringLike) const {
    return has(String::newString(std::forward<StringLike>(keyStringLike)));
  }

  /**
   * @return this instanceof type
   */
  static bool instanceOf(const Local<Value>& type);

  /**
   * @return all keys to enumerate properties of this object
   */
  static std::vector<Local<String>> getKeys();

  /**
   * @return all keys to enumerate properties of this object
   */
  static std::vector<std::string> getKeyNames();

  SPECIALIZE_NON_VALUE(Object)
};

template <>
class Local<String> {
  SPECIALIZE_LOCAL(String)

public:
  /**
   * avoid memory copy, provides better performance.
   * especially useful when requires string_view or c string
   *
   * \code
   * auto holder = s.toStringHolder();
   * printf("%s", holder.c_str());
   * \endcode
   *
   * \see StringHolder
   */
  [[nodiscard]] StringHolder toStringHolder() const;

  [[nodiscard]] std::string toString() const;

#ifdef __cpp_char8_t

  [[nodiscard]] std::u8string toU8string() const;

#endif

  SPECIALIZE_NON_VALUE(String)

private:
  friend class StringHolder;
};

template <>
class Local<Number> {
  SPECIALIZE_LOCAL(Number)

public:
  static int32_t toInt32();

  static int64_t toInt64();

  static float toFloat();

  static double toDouble();

  SPECIALIZE_NON_VALUE(Number)
};

template <>
class Local<Boolean> {
  SPECIALIZE_LOCAL(Boolean)

public:
  static bool value();

  SPECIALIZE_NON_VALUE(Boolean)
};

template <>
class Local<Function> {
  SPECIALIZE_LOCAL(Function)

public:
  /**
   * @param thiz the receiver of the function
   * @param args function arguments
   * @return function return value
   */
  [[nodiscard]] Local<Value> call(const Local<Value>& thiz, const std::vector<Local<Value>>& args) const {
    return callImpl(thiz, args.size(), args.data());
  }

  /**
   * @param thiz the receiver of the function, default to null
   * @param args function arguments
   * @return function return value
   */
  [[nodiscard]] Local<Value> call(const Local<Value>& thiz,
                                  const std::initializer_list<Local<Value>>& args) const {
    return callImpl(thiz, args.size(), args.begin());
  }

  /**
   * typesafe variadic template helper method
   * @tparam T MUST BE local reference, ie: Local<Type>. or supported raw C++ type to convert.
   * @param thiz the receiver of the function, default to null
   * @param args function arguments
   * @return function return value
   */
  template <typename... T>
  [[nodiscard]] Local<Value> call(const Local<Value>& thiz, T&&... args) const;

  /**
   * helper function to call with null thiz(receiver) and no arguments.
   */
  [[nodiscard]] Local<Value> call() const { return call({}); }

  /**
   * create a C++ function wrapping a Local<Function>
   *
   * \code
   * // example:
   * Local<Function> add = ...
   * auto func = add.wrapper<int(int,int)>();
   * func(1, 2) == 3;
   * \endcode
   *
   * @tparam FuncType function signature, like "int(int, int)" or "std::string(const char*, int)"
   * @return a std::function
   * @param thiz the receiver of the function, default to null
   * @note the returned std::function holds a Global<Function> to the function and receiver
   * reference, and will be auto released when destroy engine, after that, the returned
   * std::function is not valid anymore.
   */
  // implemented in Native.hpp
  template <typename FuncType>
  std::function<FuncType> wrapper(const Local<Value>& thiz = {}) const;

  SPECIALIZE_NON_VALUE(Function)

private:
  Local<Value> callImpl(const Local<Value>& thiz, size_t size, const Local<Value>* args) const;
};

template <>
class Local<Array> {
  SPECIALIZE_LOCAL(Array)

public:
  static size_t size();

  static Local<Value> get(size_t index);

  static void set(size_t index, const Local<Value>& value);

  /**
   * @param index index
   * @param value any thing supported by the type converter
   */
  template <typename T>
  void set(size_t index, T&& value) const;

  void add(const Local<Value>& value) const;

  static void clear();

  SPECIALIZE_NON_VALUE(Array)
};

/**
 * ByteBuffer represents a chunk of memory that you can exchange data between ScriptEngine and C++.
 * For JavaScript language these are ArrayBuffer, or any TypedArray.
 *
 * There are two kinds of ByteBuffer.
 *
 * 1. shared.
 * 2. non-shared.
 *
 * For instance, when we have a "data = new ArrayBuffer()" in js,
 * and passed to C++ as "Local<ByteBuffer> bytes".
 *
 * Shared means "data" and "bytes" can point to the same memory address, so any write on "data" can
 * be seen on "bytes", and vice-versa.
 *
 * Non-shared means we can't get the pointer of "data" to the memory chunk(usually the current
 * backend not support this), so we have to make a copy of it. In this case, we need two more
 * operations: "sync" to copy data form "data" to "bytes" and "commit" to copy from "bytes" to
 * "data".
 *
 * For now, V8 and JavaScriptCore backend support full shared, that is all ByteBuffer are shared.
 * For Wasm back end it's different, please read the doc.
 *
 * But you can always use a shared byte buffer as no non-shared, "commit" & "sync" on it would be
 * noop.
 */
template <>
class Local<ByteBuffer> {
  SPECIALIZE_LOCAL(ByteBuffer)

public:
  /**
   * get the underlying raw bytes
   *
   * @return the raw pointer, caller does NOT OWN this pointer!
   * And it's only valid during the Local<ByteArray> lifecycle.
   *
   * This is a cheaper call (depends on implementation)
   * compared to getRawBytesShared(), use this one when you
   * just want to get the raw bytes temporarily.
   */
  static void* getRawBytes();

  /**
   * get the underlying raw bytes
   * @return the raw pointer, caller OWN the shared_ptr,
   * and the shared_ptr stay valid until the engine is destroyed.
   *
   * note:
   *
   * 1. caller should release the shared_ptr before destroying ScriptEngine.
   *
   * 2. getRawBytes() == getRawBytesShared().get()
   *
   * 3. getRawBytesShared() maybe more expensive compared to getRawBytes() depends on
   * implementations.
   */
  static std::shared_ptr<void> getRawBytesShared();

  /**
   * @return size of raw bytes, in unit of byte
   */
  static size_t byteLength();

  /**
   * @return count of elements (uint8, uint32, etc).
   * use getType() to fetch element type.
   */
  [[nodiscard]] static size_t elementCount() { return byteLength() / ByteBuffer::getTypeSize(getType()); }

  /**
   * @return the content type
   */
  static ByteBuffer::Type getType();

  /**
   * If the ByteBuffer is a shared ByteBuffer.
   * Read class doc for more detail about shared & non-shared.
   */
  static bool isShared();

  /**
   * If the ByteBuffer is non-shared, copy data from C++ to Script.
   * For shared ByteBuffer, this is a noop.
   */
  static void commit();

  /**
   * If the ByteBuffer is non-shared, copy data from Script to C++.
   * For shared ByteBuffer, this is a noop.
   */
  static void sync();

  SPECIALIZE_NON_VALUE(ByteBuffer)
};

template <>
class Local<Unsupported> {
  SPECIALIZE_LOCAL(Unsupported)

public:
  SPECIALIZE_NON_VALUE(Unsupported)
};

#undef SPECIALIZE_LOCAL
#undef SPECIALIZE_NON_VALUE
} // namespace script
