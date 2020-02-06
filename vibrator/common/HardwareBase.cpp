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

#include "HardwareBase.h"

#include <cutils/properties.h>
#include <log/log.h>

#include <fstream>
#include <sstream>

#include "utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

HwApiBase::HwApiBase() {
    mPathPrefix = std::getenv("HWAPI_PATH_PREFIX") ?: "";
    if (mPathPrefix.empty()) {
        ALOGE("Failed get HWAPI path prefix!");
    }
}

bool HwApiBase::has(const std::ios &stream) {
    return !!stream;
}

void HwApiBase::debug(int fd) {
    dprintf(fd, "Kernel:\n");

    for (auto &entry : utils::pathsFromEnv("HWAPI_DEBUG_PATHS", mPathPrefix)) {
        auto &path = entry.first;
        auto &stream = entry.second;
        std::string line;

        dprintf(fd, "  %s:\n", path.c_str());
        while (std::getline(stream, line)) {
            dprintf(fd, "    %s\n", line.c_str());
        }
    }

    mRecordsMutex.lock();
    dprintf(fd, "  Records:\n");
    for (auto &r : mRecords) {
        if (r == nullptr) {
            continue;
        }
        dprintf(fd, "    %s\n", r->toString(mNames).c_str());
    }
    mRecordsMutex.unlock();
}

HwCalBase::HwCalBase() {
    std::ifstream calfile;
    auto propertyPrefix = std::getenv("PROPERTY_PREFIX");

    if (propertyPrefix != NULL) {
        mPropertyPrefix = std::string(propertyPrefix);
    } else {
        ALOGE("Failed get property prefix!");
    }

    utils::fileFromEnv("CALIBRATION_FILEPATH", &calfile);

    for (std::string line; std::getline(calfile, line);) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::istringstream is_line(line);
        std::string key, value;
        if (std::getline(is_line, key, ':') && std::getline(is_line, value)) {
            mCalData[utils::trim(key)] = utils::trim(value);
        }
    }
}

void HwCalBase::debug(int fd) {
    std::ifstream stream;
    std::string path;
    std::string line;
    struct context {
        HwCalBase *obj;
        int fd;
    } context{this, fd};

    dprintf(fd, "Properties:\n");

    property_list(
        [](const char *key, const char *value, void *cookie) {
            struct context *context = static_cast<struct context *>(cookie);
            HwCalBase *obj = context->obj;
            int fd = context->fd;
            const std::string expect{obj->mPropertyPrefix};
            const std::string actual{key, std::min(strlen(key), expect.size())};
            if (actual == expect) {
                dprintf(fd, "  %s:\n", key);
                dprintf(fd, "    %s\n", value);
            }
        },
        &context);

    dprintf(fd, "\n");

    dprintf(fd, "Persist:\n");

    utils::fileFromEnv("CALIBRATION_FILEPATH", &stream, &path);

    dprintf(fd, "  %s:\n", path.c_str());
    while (std::getline(stream, line)) {
        dprintf(fd, "    %s\n", line.c_str());
    }
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
