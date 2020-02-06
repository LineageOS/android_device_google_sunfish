/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <fstream>
#include <map>
#include <sstream>

#include <android-base/macros.h>
#include <android-base/properties.h>
#include <log/log.h>

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {
namespace utils {

template <typename T>
class Is_Iterable {
  private:
    template <typename U>
    static std::true_type test(typename U::iterator *u);

    template <typename U>
    static std::false_type test(U *u);

  public:
    static const bool value = decltype(test<T>(0))::value;
};

template <typename T, bool B>
using Enable_If_Iterable = std::enable_if_t<Is_Iterable<T>::value == B>;

template <typename T, typename U = void>
using Enable_If_Signed = std::enable_if_t<std::is_signed_v<T>, U>;

template <typename T, typename U = void>
using Enable_If_Unsigned = std::enable_if_t<std::is_unsigned_v<T>, U>;

// override for default behavior of printing as a character
inline std::ostream &operator<<(std::ostream &stream, const int8_t value) {
    return stream << +value;
}
// override for default behavior of printing as a character
inline std::ostream &operator<<(std::ostream &stream, const uint8_t value) {
    return stream << +value;
}

template <typename T>
inline auto toUnderlying(const T value) {
    return static_cast<std::underlying_type_t<T>>(value);
}

template <typename T>
inline Enable_If_Iterable<T, true> unpack(std::istream &stream, T *value) {
    for (auto &entry : *value) {
        stream >> entry;
    }
}

template <typename T>
inline Enable_If_Iterable<T, false> unpack(std::istream &stream, T *value) {
    stream >> *value;
}

template <>
inline void unpack<std::string>(std::istream &stream, std::string *value) {
    *value = std::string(std::istreambuf_iterator(stream), {});
    stream.setstate(std::istream::eofbit);
}

template <typename T>
inline Enable_If_Signed<T, T> getProperty(const std::string &key, const T def) {
    return ::android::base::GetIntProperty(key, def);
}

template <typename T>
inline Enable_If_Unsigned<T, T> getProperty(const std::string &key, const T def) {
    return ::android::base::GetUintProperty(key, def);
}

template <>
inline bool getProperty<bool>(const std::string &key, const bool def) {
    return ::android::base::GetBoolProperty(key, def);
}

template <typename T>
static void openNoCreate(const std::string &file, T *outStream) {
    auto mode = std::is_base_of_v<std::ostream, T> ? std::ios_base::out : std::ios_base::in;

    // Force 'in' mode to prevent file creation
    outStream->open(file, mode | std::ios_base::in);
    if (!*outStream) {
        ALOGE("Failed to open %s (%d): %s", file.c_str(), errno, strerror(errno));
    }
}

template <typename T>
static void fileFromEnv(const char *env, T *outStream, std::string *outName = nullptr) {
    auto file = std::getenv(env);

    if (file == nullptr) {
        ALOGE("Failed get env %s", env);
        return;
    }

    if (outName != nullptr) {
        *outName = std::string(file);
    }

    openNoCreate(file, outStream);
}

static ATTRIBUTE_UNUSED auto pathsFromEnv(const char *env, const std::string &prefix = "") {
    std::map<std::string, std::ifstream> ret;
    auto value = std::getenv(env);

    if (value == nullptr) {
        return ret;
    }

    std::istringstream paths{value};
    std::string path;

    while (paths >> path) {
        ret[path].open(prefix + path);
    }

    return ret;
}

static ATTRIBUTE_UNUSED std::string trim(const std::string &str,
                                         const std::string &whitespace = " \t") {
    const auto str_begin = str.find_first_not_of(whitespace);
    if (str_begin == std::string::npos) {
        return "";
    }

    const auto str_end = str.find_last_not_of(whitespace);
    const auto str_range = str_end - str_begin + 1;

    return str.substr(str_begin, str_range);
}

}  // namespace utils
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
