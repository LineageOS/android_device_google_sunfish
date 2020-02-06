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

#include <android-base/unique_fd.h>
#include <log/log.h>
#include <sys/epoll.h>
#include <utils/Trace.h>

#include <list>
#include <map>
#include <sstream>
#include <string>

#include "utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

using ::android::base::unique_fd;

class HwApiBase {
  private:
    using NamesMap = std::map<const std::ios *, std::string>;

    class RecordInterface {
      public:
        virtual std::string toString(const NamesMap &names) = 0;
        virtual ~RecordInterface() {}
    };
    template <typename T>
    class Record : public RecordInterface {
      public:
        Record(const char *func, const T &value, const std::ios *stream)
            : mFunc(func), mValue(value), mStream(stream) {}
        std::string toString(const NamesMap &names) override;

      private:
        const char *mFunc;
        const T mValue;
        const std::ios *mStream;
    };
    using Records = std::list<std::unique_ptr<RecordInterface>>;

    static constexpr uint32_t RECORDS_SIZE = 32;

  public:
    HwApiBase();
    void debug(int fd);

  protected:
    template <typename T>
    void open(const std::string &name, T *stream);
    template <typename T>
    void openFull(const std::string &name, T *stream);
    bool has(const std::ios &stream);
    template <typename T>
    bool get(T *value, std::istream *stream);
    template <typename T>
    bool set(const T &value, std::ostream *stream);
    template <typename T>
    bool poll(const T &value, std::istream *stream);
    template <typename T>
    void record(const char *func, const T &value, const std::ios *stream);

  private:
    std::string mPathPrefix;
    NamesMap mNames;
    Records mRecords{RECORDS_SIZE};
    std::mutex mRecordsMutex;
};

#define HWAPI_RECORD(args...) HwApiBase::record(__FUNCTION__, ##args)

template <typename T>
void HwApiBase::open(const std::string &name, T *stream) {
    mNames[stream] = name;
    utils::openNoCreate(mPathPrefix + name, stream);
}

template <typename T>
void HwApiBase::openFull(const std::string &name, T *stream) {
    mNames[stream] = name;
    utils::openNoCreate(name, stream);
}

template <typename T>
bool HwApiBase::get(T *value, std::istream *stream) {
    ATRACE_NAME("HwApi::get");
    bool ret;
    stream->seekg(0);
    *stream >> *value;
    if (!(ret = !!*stream)) {
        ALOGE("Failed to read %s (%d): %s", mNames[stream].c_str(), errno, strerror(errno));
    }
    stream->clear();
    HWAPI_RECORD(*value, stream);
    return ret;
}

template <typename T>
bool HwApiBase::set(const T &value, std::ostream *stream) {
    ATRACE_NAME("HwApi::set");
    using utils::operator<<;
    bool ret;
    *stream << value << std::endl;
    if (!(ret = !!*stream)) {
        ALOGE("Failed to write %s (%d): %s", mNames[stream].c_str(), errno, strerror(errno));
        stream->clear();
    }
    HWAPI_RECORD(value, stream);
    return ret;
}

template <typename T>
bool HwApiBase::poll(const T &value, std::istream *stream) {
    ATRACE_NAME("HwApi::poll");
    auto path = mPathPrefix + mNames[stream];
    unique_fd fileFd{::open(path.c_str(), O_RDONLY)};
    unique_fd epollFd{epoll_create(1)};
    epoll_event event = {
        .events = EPOLLPRI | EPOLLET,
    };
    T actual;
    bool ret;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fileFd, &event)) {
        ALOGE("Failed to poll %s (%d): %s", mNames[stream].c_str(), errno, strerror(errno));
        return false;
    }

    while ((ret = get(&actual, stream)) && (actual != value)) {
        epoll_wait(epollFd, &event, 1, -1);
    }

    HWAPI_RECORD(value, stream);
    return ret;
}

template <typename T>
void HwApiBase::record(const char *func, const T &value, const std::ios *stream) {
    std::lock_guard<std::mutex> lock(mRecordsMutex);
    mRecords.emplace_back(std::make_unique<Record<T>>(func, value, stream));
    mRecords.pop_front();
}

template <typename T>
std::string HwApiBase::Record<T>::toString(const NamesMap &names) {
    using utils::operator<<;
    std::stringstream ret;

    ret << mFunc << " '" << names.at(mStream) << "' = '" << mValue << "'";

    return ret.str();
}

class HwCalBase {
  public:
    HwCalBase();
    void debug(int fd);

  protected:
    template <typename T>
    bool getProperty(const char *key, T *value, const T defval);
    template <typename T>
    bool getPersist(const char *key, T *value);

  private:
    std::string mPropertyPrefix;
    std::map<std::string, std::string> mCalData;
};

template <typename T>
bool HwCalBase::getProperty(const char *key, T *outval, const T defval) {
    ATRACE_NAME("HwCal::getProperty");
    *outval = utils::getProperty(mPropertyPrefix + key, defval);
    return true;
}

template <typename T>
bool HwCalBase::getPersist(const char *key, T *value) {
    ATRACE_NAME("HwCal::getPersist");
    auto it = mCalData.find(key);
    if (it == mCalData.end()) {
        ALOGE("Missing %s config!", key);
        return false;
    }
    std::stringstream stream{it->second};
    utils::unpack(stream, value);
    if (!stream || !stream.eof()) {
        ALOGE("Invalid %s config!", key);
        return false;
    }
    return true;
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
