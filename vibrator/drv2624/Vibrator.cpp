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

#include "Vibrator.h"

#include <cutils/properties.h>
#include <hardware/hardware.h>
#include <hardware/vibrator.h>
#include <log/log.h>
#include <utils/Trace.h>

#include <cinttypes>
#include <cmath>
#include <fstream>
#include <iostream>

#include "utils.h"

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_3 {
namespace implementation {

static constexpr int8_t MAX_RTP_INPUT = 127;
static constexpr int8_t MIN_RTP_INPUT = 0;

static constexpr char RTP_MODE[] = "rtp";
static constexpr char WAVEFORM_MODE[] = "waveform";

// Use effect #1 in the waveform library for CLICK effect
static constexpr char WAVEFORM_CLICK_EFFECT_SEQ[] = "1 0";

// Use effect #2 in the waveform library for TICK effect
static constexpr char WAVEFORM_TICK_EFFECT_SEQ[] = "2 0";

// Use effect #3 in the waveform library for DOUBLE_CLICK effect
static constexpr char WAVEFORM_DOUBLE_CLICK_EFFECT_SEQ[] = "3 0";

// Use effect #4 in the waveform library for HEAVY_CLICK effect
static constexpr char WAVEFORM_HEAVY_CLICK_EFFECT_SEQ[] = "4 0";

static std::uint32_t freqPeriodFormula(std::uint32_t in) {
    return 1000000000 / (24615 * in);
}

using utils::toUnderlying;

using Status = ::android::hardware::vibrator::V1_0::Status;
using EffectStrength = ::android::hardware::vibrator::V1_0::EffectStrength;

Vibrator::Vibrator(std::unique_ptr<HwApi> hwapi, std::unique_ptr<HwCal> hwcal)
    : mHwApi(std::move(hwapi)), mHwCal(std::move(hwcal)) {
    std::string autocal;
    uint32_t lraPeriod;
    bool dynamicConfig;

    if (!mHwApi->setState(true)) {
        ALOGE("Failed to set state (%d): %s", errno, strerror(errno));
    }

    if (mHwCal->getAutocal(&autocal)) {
        mHwApi->setAutocal(autocal);
    }
    mHwCal->getLraPeriod(&lraPeriod);

    mHwCal->getCloseLoopThreshold(&mCloseLoopThreshold);
    mHwCal->getDynamicConfig(&dynamicConfig);

    if (dynamicConfig) {
        uint32_t longFreqencyShift;
        uint32_t shortVoltageMax, longVoltageMax;

        mHwCal->getLongFrequencyShift(&longFreqencyShift);
        mHwCal->getShortVoltageMax(&shortVoltageMax);
        mHwCal->getLongVoltageMax(&longVoltageMax);

        mEffectConfig.reset(new VibrationConfig({
            .shape = WaveShape::SINE,
            .odClamp = shortVoltageMax,
            .olLraPeriod = lraPeriod,
        }));
        mSteadyConfig.reset(new VibrationConfig({
            .shape = WaveShape::SQUARE,
            .odClamp = longVoltageMax,
            // 1. Change long lra period to frequency
            // 2. Get frequency': subtract the frequency shift from the frequency
            // 3. Get final long lra period after put the frequency' to formula
            .olLraPeriod = freqPeriodFormula(freqPeriodFormula(lraPeriod) - longFreqencyShift),
        }));
    } else {
        mHwApi->setOlLraPeriod(lraPeriod);
    }

    mHwCal->getClickDuration(&mClickDuration);
    mHwCal->getTickDuration(&mTickDuration);
    mHwCal->getDoubleClickDuration(&mDoubleClickDuration);
    mHwCal->getHeavyClickDuration(&mHeavyClickDuration);

    // This enables effect #1 from the waveform library to be triggered by SLPI
    // while the AP is in suspend mode
    if (!mHwApi->setLpTriggerEffect(1)) {
        ALOGW("Failed to set LP trigger mode (%d): %s", errno, strerror(errno));
    }
}

Return<Status> Vibrator::on(uint32_t timeoutMs, const char mode[],
                            const std::unique_ptr<VibrationConfig> &config) {
    LoopControl loopMode = LoopControl::OPEN;

    // Open-loop mode is used for short click for over-drive
    // Close-loop mode is used for long notification for stability
    if (mode == RTP_MODE && timeoutMs > mCloseLoopThreshold) {
        loopMode = LoopControl::CLOSE;
    }

    mHwApi->setCtrlLoop(toUnderlying(loopMode));
    if (!mHwApi->setDuration(timeoutMs)) {
        ALOGE("Failed to set duration (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }

    mHwApi->setMode(mode);
    if (config != nullptr) {
        mHwApi->setLraWaveShape(toUnderlying(config->shape));
        mHwApi->setOdClamp(config->odClamp);
        mHwApi->setOlLraPeriod(config->olLraPeriod);
    }

    if (!mHwApi->setActivate(1)) {
        ALOGE("Failed to activate (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }

    return Status::OK;
}

// Methods from ::android::hardware::vibrator::V1_2::IVibrator follow.
Return<Status> Vibrator::on(uint32_t timeoutMs) {
    ATRACE_NAME("Vibrator::on");
    return on(timeoutMs, RTP_MODE, mSteadyConfig);
}

Return<Status> Vibrator::off() {
    ATRACE_NAME("Vibrator::off");
    if (!mHwApi->setActivate(0)) {
        ALOGE("Failed to turn vibrator off (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

Return<bool> Vibrator::supportsAmplitudeControl() {
    ATRACE_NAME("Vibrator::supportsAmplitudeControl");
    return (mHwApi->hasRtpInput() ? true : false);
}

Return<Status> Vibrator::setAmplitude(uint8_t amplitude) {
    ATRACE_NAME("Vibrator::setAmplitude");
    if (amplitude == 0) {
        return Status::BAD_VALUE;
    }

    int32_t rtp_input =
        std::round((amplitude - 1) / 254.0 * (MAX_RTP_INPUT - MIN_RTP_INPUT) + MIN_RTP_INPUT);

    if (!mHwApi->setRtpInput(rtp_input)) {
        ALOGE("Failed to set amplitude (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }

    return Status::OK;
}

// Methods from ::android::hardware::vibrator::V1_3::IVibrator follow.

Return<bool> Vibrator::supportsExternalControl() {
    ATRACE_NAME("Vibrator::supportsExternalControl");
    return false;
}

Return<Status> Vibrator::setExternalControl(bool enabled) {
    ATRACE_NAME("Vibrator::setExternalControl");
    ALOGE("Not support in DRV2624 solution, %d", enabled);
    return Status::UNSUPPORTED_OPERATION;
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

    dprintf(fd, "  Close Loop Thresh: %" PRIu32 "\n", mCloseLoopThreshold);
    if (mSteadyConfig) {
        dprintf(fd, "  Steady Shape: %" PRIu32 "\n", mSteadyConfig->shape);
        dprintf(fd, "  Steady OD Clamp: %" PRIu32 "\n", mSteadyConfig->odClamp);
        dprintf(fd, "  Steady OL LRA Period: %" PRIu32 "\n", mSteadyConfig->olLraPeriod);
    }
    if (mEffectConfig) {
        dprintf(fd, "  Effect Shape: %" PRIu32 "\n", mEffectConfig->shape);
        dprintf(fd, "  Effect OD Clamp: %" PRIu32 "\n", mEffectConfig->odClamp);
        dprintf(fd, "  Effect OL LRA Period: %" PRIu32 "\n", mEffectConfig->olLraPeriod);
    }
    dprintf(fd, "  Click Duration: %" PRIu32 "\n", mClickDuration);
    dprintf(fd, "  Tick Duration: %" PRIu32 "\n", mTickDuration);
    dprintf(fd, "  Double Click Duration: %" PRIu32 "\n", mDoubleClickDuration);
    dprintf(fd, "  Heavy Click Duration: %" PRIu32 "\n", mHeavyClickDuration);

    dprintf(fd, "\n");

    mHwApi->debug(fd);

    dprintf(fd, "\n");

    mHwCal->debug(fd);

    fsync(fd);
    return Void();
}

static uint8_t convertEffectStrength(EffectStrength strength) {
    uint8_t scale;

    switch (strength) {
        case EffectStrength::LIGHT:
            scale = 2;  // 50%
            break;
        case EffectStrength::MEDIUM:
        case EffectStrength::STRONG:
            scale = 0;  // 100%
            break;
    }

    return scale;
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

template <typename T>
Return<void> Vibrator::performWrapper(T effect, EffectStrength strength, perform_cb _hidl_cb) {
    ATRACE_NAME("Vibrator::performWrapper");
    auto validEffectRange = hidl_enum_range<T>();
    if (effect < *validEffectRange.begin() || effect > *std::prev(validEffectRange.end())) {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
        return Void();
    }
    auto validStrengthRange = hidl_enum_range<EffectStrength>();
    if (strength < *validStrengthRange.begin() || strength > *std::prev(validStrengthRange.end())) {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
        return Void();
    }
    return performEffect(static_cast<Effect>(effect), strength, _hidl_cb);
}

Return<void> Vibrator::performEffect(Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    Status status = Status::OK;
    uint32_t timeMS;

    switch (effect) {
        case Effect::TEXTURE_TICK:
            mHwApi->setSequencer(WAVEFORM_TICK_EFFECT_SEQ);
            timeMS = mTickDuration;
            break;
        case Effect::CLICK:
            mHwApi->setSequencer(WAVEFORM_CLICK_EFFECT_SEQ);
            timeMS = mClickDuration;
            break;
        case Effect::DOUBLE_CLICK:
            mHwApi->setSequencer(WAVEFORM_DOUBLE_CLICK_EFFECT_SEQ);
            timeMS = mDoubleClickDuration;
            break;
        case Effect::TICK:
            mHwApi->setSequencer(WAVEFORM_TICK_EFFECT_SEQ);
            timeMS = mTickDuration;
            break;
        case Effect::HEAVY_CLICK:
            mHwApi->setSequencer(WAVEFORM_HEAVY_CLICK_EFFECT_SEQ);
            timeMS = mHeavyClickDuration;
            break;
        default:
            _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
            return Void();
    }
    mHwApi->setScale(convertEffectStrength(strength));
    on(timeMS, WAVEFORM_MODE, mEffectConfig);
    _hidl_cb(status, timeMS);
    return Void();
}

}  // namespace implementation
}  // namespace V1_3
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
