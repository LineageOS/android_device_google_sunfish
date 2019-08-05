/*
 * Copyright (C) 2017 The Android Open Source Project
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

#define LOG_TAG "android.hardware.vibrator@1.3-service.sunfish"

#include <log/log.h>

#include <cutils/properties.h>
#include <hardware/hardware.h>
#include <hardware/vibrator.h>

#include "Vibrator.h"

#include <cinttypes>
#include <cmath>
#include <fstream>
#include <iostream>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_3 {
namespace implementation {

using Status = ::android::hardware::vibrator::V1_0::Status;
using EffectStrength = ::android::hardware::vibrator::V1_0::EffectStrength;

static constexpr uint32_t WAVEFORM_SIMPLE_EFFECT_INDEX = 2;

static constexpr uint32_t WAVEFORM_TEXTURE_TICK_EFFECT_LEVEL = 0;

static constexpr uint32_t WAVEFORM_TICK_EFFECT_LEVEL = 1;

static constexpr uint32_t WAVEFORM_CLICK_EFFECT_LEVEL = 2;

static constexpr uint32_t WAVEFORM_HEAVY_CLICK_EFFECT_LEVEL = 3;

static constexpr uint32_t WAVEFORM_DOUBLE_CLICK_SILENCE_MS = 100;

static constexpr uint32_t WAVEFORM_LONG_VIBRATION_EFFECT_INDEX = 0;

static constexpr uint32_t WAVEFORM_TRIGGER_QUEUE_INDEX = 65534;

static constexpr uint32_t VOLTAGE_GLOBAL_SCALE_LEVEL = 5;
static constexpr uint8_t VOLTAGE_SCALE_MAX = 100;

static constexpr int8_t MAX_COLD_START_LATENCY_MS = 6; // I2C Transaction + DSP Return-From-Standby
static constexpr int8_t MAX_PAUSE_TIMING_ERROR_MS = 1; // ALERT Irq Handling

static constexpr float AMP_ATTENUATE_STEP_SIZE = 0.125f;
static constexpr float EFFECT_FREQUENCY_KHZ = 48.0f;

static uint8_t amplitudeToScale(uint8_t amplitude, uint8_t maximum) {
    return std::round((-20 * std::log10(amplitude / static_cast<float>(maximum))) /
                      (AMP_ATTENUATE_STEP_SIZE));
}

Vibrator::Vibrator(std::unique_ptr<HwApi> hwapi, std::unique_ptr<HwCal> hwcal)
    : mHwApi(std::move(hwapi)), mHwCal(std::move(hwcal)) {
    uint32_t caldata;
    uint32_t effectDuration;

    if (!mHwApi->setState(true)) {
        ALOGE("Failed to set state (%d): %s", errno, strerror(errno));
    }

    if (mHwCal->getF0(&caldata)) {
        mHwApi->setF0(caldata);
    }
    if (mHwCal->getRedc(&caldata)) {
        mHwApi->setRedc(caldata);
    }
    if (mHwCal->getQ(&caldata)) {
        mHwApi->setQ(caldata);
    }
    mHwCal->getVolLevels(&mVolLevels);

    mHwApi->setEffectIndex(WAVEFORM_SIMPLE_EFFECT_INDEX);
    mHwApi->getEffectDuration(&effectDuration);

    mSimpleEffectDuration = std::ceil(effectDuration / EFFECT_FREQUENCY_KHZ);

    const uint32_t scaleFall =
        amplitudeToScale(mVolLevels[WAVEFORM_CLICK_EFFECT_LEVEL], VOLTAGE_SCALE_MAX);
    const uint32_t scaleRise =
        amplitudeToScale(mVolLevels[WAVEFORM_HEAVY_CLICK_EFFECT_LEVEL], VOLTAGE_SCALE_MAX);

    mHwApi->setGpioFallIndex(WAVEFORM_SIMPLE_EFFECT_INDEX);
    mHwApi->setGpioFallScale(scaleFall);
    mHwApi->setGpioRiseIndex(WAVEFORM_SIMPLE_EFFECT_INDEX);
    mHwApi->setGpioRiseScale(scaleRise);
}

Return<Status> Vibrator::on(uint32_t timeoutMs, uint32_t effectIndex) {
    mHwApi->setEffectIndex(effectIndex);
    mHwApi->setDuration(timeoutMs);
    mHwApi->setActivate(1);

    return Status::OK;
}

// Methods from ::android::hardware::vibrator::V1_1::IVibrator follow.
Return<Status> Vibrator::on(uint32_t timeoutMs) {
    if (MAX_COLD_START_LATENCY_MS <= UINT32_MAX - timeoutMs) {
        timeoutMs += MAX_COLD_START_LATENCY_MS;
    }
    setGlobalAmplitude(true);
    return on(timeoutMs, WAVEFORM_LONG_VIBRATION_EFFECT_INDEX);
}

Return<Status> Vibrator::off() {
    setGlobalAmplitude(false);
    if (!mHwApi->setActivate(0)) {
        ALOGE("Failed to turn vibrator off (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

Return<bool> Vibrator::supportsAmplitudeControl() {
    return !isUnderExternalControl() && mHwApi->hasEffectScale();
}

Return<Status> Vibrator::setAmplitude(uint8_t amplitude) {
    if (!amplitude) {
        return Status::BAD_VALUE;
    }

    if (!isUnderExternalControl()) {
        return setEffectAmplitude(amplitude, UINT8_MAX);
    } else {
        return Status::UNSUPPORTED_OPERATION;
    }
}

Return<Status> Vibrator::setEffectAmplitude(uint8_t amplitude, uint8_t maximum) {
    int32_t scale = amplitudeToScale(amplitude, maximum);

    if (!mHwApi->setEffectScale(scale)) {
        ALOGE("Failed to set effect amplitude (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }

    return Status::OK;
}

Return<Status> Vibrator::setGlobalAmplitude(bool set) {
    uint8_t amplitude = set ? mVolLevels[VOLTAGE_GLOBAL_SCALE_LEVEL] : VOLTAGE_SCALE_MAX;
    int32_t scale = amplitudeToScale(amplitude, VOLTAGE_SCALE_MAX);

    if (!mHwApi->setGlobalScale(scale)) {
        ALOGE("Failed to set global amplitude (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }

    return Status::OK;
}

// Methods from ::android::hardware::vibrator::V1_3::IVibrator follow.

Return<bool> Vibrator::supportsExternalControl() {
    return (mHwApi->hasAspEnable() ? true : false);
}

Return<Status> Vibrator::setExternalControl(bool enabled) {
    setGlobalAmplitude(enabled);

    if (!mHwApi->setAspEnable(enabled)) {
        ALOGE("Failed to set external control (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

bool Vibrator::isUnderExternalControl() {
    bool isAspEnabled;
    mHwApi->getAspEnable(&isAspEnabled);
    return isAspEnabled;
}

// Methods from ::android.hidl.base::V1_0::IBase follow.

Return<void> Vibrator::debug(const hidl_handle &handle,
                             const hidl_vec<hidl_string> & /* options */) {
    if (handle == nullptr || handle->numFds < 1 || handle->data[0] < 0) {
        ALOGE("Called debug() with invalid fd.");
        return Void();
    }

    int fd = handle->data[0];

    dprintf(fd, "HIDL:\n");

    dprintf(fd, "  Voltage Levels:");
    for (auto v : mVolLevels) {
        dprintf(fd, " %" PRIu32, v);
    }
    dprintf(fd, "\n");

    dprintf(fd, "  Effect Duration: %" PRIu32 "\n", mSimpleEffectDuration);

    dprintf(fd, "\n");

    mHwApi->debug(fd);

    dprintf(fd, "\n");

    mHwCal->debug(fd);

    fsync(fd);
    return Void();
}

template <typename T>
Return<void> Vibrator::performWrapper(T effect, EffectStrength strength, perform_cb _hidl_cb) {
    auto validRange = hidl_enum_range<T>();
    if (effect < *validRange.begin() || effect > *std::prev(validRange.end())) {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
        return Void();
    }
    return performEffect(static_cast<Effect>(effect), strength, _hidl_cb);
}

Return<void> Vibrator::perform(V1_0::Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    return performWrapper(effect, strength, _hidl_cb);
}

Return<void> Vibrator::perform_1_1(V1_1::Effect_1_1 effect, EffectStrength strength,
                                   perform_cb _hidl_cb) {
    return performWrapper(effect, strength, _hidl_cb);
}

Return<void> Vibrator::perform_1_2(V1_2::Effect effect, EffectStrength strength,
                                   perform_cb _hidl_cb) {
    return performWrapper(effect, strength, _hidl_cb);
}

Return<void> Vibrator::perform_1_3(Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    return performWrapper(effect, strength, _hidl_cb);
}

Return<Status> Vibrator::getSimpleDetails(Effect effect, EffectStrength strength,
                                          uint32_t *outTimeMs, uint32_t *outVolLevel) {
    uint32_t timeMs;
    uint32_t volLevel;
    uint32_t volIndex;
    int8_t volOffset;

    switch (strength) {
        case EffectStrength::LIGHT:
            volOffset = -1;
            break;
        case EffectStrength::MEDIUM:
            volOffset = 0;
            break;
        case EffectStrength::STRONG:
            volOffset = 1;
            break;
        default:
            return Status::UNSUPPORTED_OPERATION;
    }

    switch (effect) {
        case Effect::TEXTURE_TICK:
            volIndex = WAVEFORM_TEXTURE_TICK_EFFECT_LEVEL;
            volOffset = 0;
            break;
        case Effect::TICK:
            volIndex = WAVEFORM_TICK_EFFECT_LEVEL;
            volOffset = 0;
            break;
        case Effect::CLICK:
            volIndex = WAVEFORM_CLICK_EFFECT_LEVEL;
            break;
        case Effect::HEAVY_CLICK:
            volIndex = WAVEFORM_HEAVY_CLICK_EFFECT_LEVEL;
            break;
        default:
            return Status::UNSUPPORTED_OPERATION;
    }

    volLevel = mVolLevels[volIndex + volOffset];
    timeMs = mSimpleEffectDuration + MAX_COLD_START_LATENCY_MS;

    *outTimeMs = timeMs;
    *outVolLevel = volLevel;

    return Status::OK;
}

Return<Status> Vibrator::getCompoundDetails(Effect effect, EffectStrength strength,
                                            uint32_t *outTimeMs, uint32_t * /*outVolLevel*/,
                                            std::string *outEffectQueue) {
    Status status;
    uint32_t timeMs;
    std::ostringstream effectBuilder;
    uint32_t thisTimeMs;
    uint32_t thisVolLevel;

    switch (effect) {
        case Effect::DOUBLE_CLICK:
            timeMs = 0;

            status = getSimpleDetails(Effect::CLICK, strength, &thisTimeMs, &thisVolLevel);
            if (status != Status::OK) {
                return status;
            }
            effectBuilder << WAVEFORM_SIMPLE_EFFECT_INDEX << "." << thisVolLevel;
            timeMs += thisTimeMs;

            effectBuilder << ",";

            effectBuilder << WAVEFORM_DOUBLE_CLICK_SILENCE_MS;
            timeMs += WAVEFORM_DOUBLE_CLICK_SILENCE_MS + MAX_PAUSE_TIMING_ERROR_MS;

            effectBuilder << ",";

            status = getSimpleDetails(Effect::HEAVY_CLICK, strength, &thisTimeMs, &thisVolLevel);
            if (status != Status::OK) {
                return status;
            }
            effectBuilder << WAVEFORM_SIMPLE_EFFECT_INDEX << "." << thisVolLevel;
            timeMs += thisTimeMs;

            break;
        default:
            return Status::UNSUPPORTED_OPERATION;
    }

    *outTimeMs = timeMs;
    *outEffectQueue = effectBuilder.str();

    return Status::OK;
}

Return<Status> Vibrator::setEffectQueue(const std::string &effectQueue) {
    if (!mHwApi->setEffectQueue(effectQueue)) {
        ALOGE("Failed to write \"%s\" to effect queue (%d): %s", effectQueue.c_str(), errno,
              strerror(errno));
        return Status::UNKNOWN_ERROR;
    }

    return Status::OK;
}

Return<void> Vibrator::performEffect(Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    Status status = Status::OK;
    uint32_t timeMs = 0;
    uint32_t effectIndex;
    uint32_t volLevel;
    std::string effectQueue;

    switch (effect) {
        case Effect::TEXTURE_TICK:
            // fall-through
        case Effect::TICK:
            // fall-through
        case Effect::CLICK:
            // fall-through
        case Effect::HEAVY_CLICK:
            status = getSimpleDetails(effect, strength, &timeMs, &volLevel);
            break;
        case Effect::DOUBLE_CLICK:
            status = getCompoundDetails(effect, strength, &timeMs, &volLevel, &effectQueue);
            break;
        default:
            status = Status::UNSUPPORTED_OPERATION;
            break;
    }
    if (status != Status::OK) {
        goto exit;
    }

    if (!effectQueue.empty()) {
        status = setEffectQueue(effectQueue);
        if (status != Status::OK) {
            goto exit;
        }
        effectIndex = WAVEFORM_TRIGGER_QUEUE_INDEX;
    } else {
        setEffectAmplitude(volLevel, VOLTAGE_SCALE_MAX);
        effectIndex = WAVEFORM_SIMPLE_EFFECT_INDEX;
    }

    on(timeMs, effectIndex);

exit:

    _hidl_cb(status, timeMs);

    return Void();
}

}  // namespace implementation
}  // namespace V1_3
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
