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
#include "utils.h"

#include <cutils/properties.h>
#include <hardware/hardware.h>
#include <hardware/vibrator.h>
#include <log/log.h>
#include <utils/Trace.h>

#include <cinttypes>
#include <cmath>
#include <fstream>
#include <iostream>

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

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

// UT team design those target G values
static constexpr std::array<float, 5> EFFECT_TARGET_G = {0.15, 0.15, 0.27, 0.43, 0.57};
static constexpr std::array<float, 3> STEADY_TARGET_G = {1.2, 1.145, 0.905};

#define FLOAT_EPS 1e-6

// Temperature protection upper bound 10°C and lower bound 5°C
static constexpr int32_t TEMP_UPPER_BOUND = 10000;
static constexpr int32_t TEMP_LOWER_BOUND = 5000;
// Steady vibration's voltage in lower bound guarantee
static uint32_t STEADY_VOLTAGE_LOWER_BOUND = 90;  // 1.8 Vpeak

static std::uint32_t freqPeriodFormula(std::uint32_t in) {
    return 1000000000 / (24615 * in);
}

static std::uint32_t convertLevelsToOdClamp(float voltageLevel, uint32_t lraPeriod) {
    float odClamp;

    odClamp = voltageLevel /
              ((21.32 / 1000.0) *
               sqrt(1.0 - (static_cast<float>(freqPeriodFormula(lraPeriod)) * 8.0 / 10000.0)));

    return round(odClamp);
}

static float targetGToVlevelsUnderLinearEquation(std::array<float, 4> inputCoeffs, float targetG) {
    // Implement linear equation to get voltage levels, f(x) = ax + b
    // 0 to 3.2 is our valid output
    float outPutVal = 0.0f;
    outPutVal = (targetG - inputCoeffs[1]) / inputCoeffs[0];
    if ((outPutVal > FLOAT_EPS) && (outPutVal <= 3.2)) {
        return outPutVal;
    } else {
        return 0.0f;
    }
}

static float targetGToVlevelsUnderCubicEquation(std::array<float, 4> inputCoeffs, float targetG) {
    // Implement cubic equation to get voltage levels, f(x) = ax^3 + bx^2 + cx + d
    // 0 to 3.2 is our valid output
    float AA = 0.0f, BB = 0.0f, CC = 0.0f, Delta = 0.0f;
    float Y1 = 0.0f, Y2 = 0.0f, K = 0.0f, T = 0.0f, sita = 0.0f;
    float outPutVal = 0.0f;
    float oneHalf = 1.0 / 2.0, oneThird = 1.0 / 3.0;
    float cosSita = 0.0f, sinSitaSqrt3 = 0.0f, sqrtA = 0.0f;

    AA = inputCoeffs[1] * inputCoeffs[1] - 3.0 * inputCoeffs[0] * inputCoeffs[2];
    BB = inputCoeffs[1] * inputCoeffs[2] - 9.0 * inputCoeffs[0] * (inputCoeffs[3] - targetG);
    CC = inputCoeffs[2] * inputCoeffs[2] - 3.0 * inputCoeffs[1] * (inputCoeffs[3] - targetG);

    Delta = BB * BB - 4.0 * AA * CC;

    // There are four discriminants in Shengjin formula.
    // https://zh.wikipedia.org/wiki/%E4%B8%89%E6%AC%A1%E6%96%B9%E7%A8%8B#%E7%9B%9B%E9%87%91%E5%85%AC%E5%BC%8F%E6%B3%95
    if ((fabs(AA) <= FLOAT_EPS) && (fabs(BB) <= FLOAT_EPS)) {
        // Case 1: A = B = 0
        outPutVal = -inputCoeffs[1] / (3 * inputCoeffs[0]);
        if ((outPutVal > FLOAT_EPS) && (outPutVal <= 3.2)) {
            return outPutVal;
        }
        return 0.0f;
    } else if (Delta > FLOAT_EPS) {
        // Case 2: Delta > 0
        Y1 = AA * inputCoeffs[1] + 3.0 * inputCoeffs[0] * (-BB + pow(Delta, oneHalf)) / 2.0;
        Y2 = AA * inputCoeffs[1] + 3.0 * inputCoeffs[0] * (-BB - pow(Delta, oneHalf)) / 2.0;

        if ((Y1 < -FLOAT_EPS) && (Y2 > FLOAT_EPS)) {
            return (-inputCoeffs[1] + pow(-Y1, oneThird) - pow(Y2, oneThird)) /
                   (3.0 * inputCoeffs[0]);
        } else if ((Y1 > FLOAT_EPS) && (Y2 < -FLOAT_EPS)) {
            return (-inputCoeffs[1] - pow(Y1, oneThird) + pow(-Y2, oneThird)) /
                   (3.0 * inputCoeffs[0]);
        } else if ((Y1 < -FLOAT_EPS) && (Y2 < -FLOAT_EPS)) {
            return (-inputCoeffs[1] + pow(-Y1, oneThird) + pow(-Y2, oneThird)) /
                   (3.0 * inputCoeffs[0]);
        } else {
            return (-inputCoeffs[1] - pow(Y1, oneThird) - pow(Y2, oneThird)) /
                   (3.0 * inputCoeffs[0]);
        }
        return 0.0f;
    } else if (Delta < -FLOAT_EPS) {
        // Case 3: Delta < 0
        T = (2 * AA * inputCoeffs[1] - 3 * inputCoeffs[0] * BB) / (2 * AA * sqrt(AA));
        sita = acos(T);
        cosSita = cos(sita / 3);
        sinSitaSqrt3 = sqrt(3.0) * sin(sita / 3);
        sqrtA = sqrt(AA);

        outPutVal = (-inputCoeffs[1] - 2 * sqrtA * cosSita) / (3 * inputCoeffs[0]);
        if ((outPutVal > FLOAT_EPS) && (outPutVal <= 3.2)) {
            return outPutVal;
        }
        outPutVal = (-inputCoeffs[1] + sqrtA * (cosSita + sinSitaSqrt3)) / (3 * inputCoeffs[0]);
        if ((outPutVal > FLOAT_EPS) && (outPutVal <= 3.2)) {
            return outPutVal;
        }
        outPutVal = (-inputCoeffs[1] + sqrtA * (cosSita - sinSitaSqrt3)) / (3 * inputCoeffs[0]);
        if ((outPutVal > FLOAT_EPS) && (outPutVal <= 3.2)) {
            return outPutVal;
        }
        return 0.0f;
    } else if (Delta <= FLOAT_EPS) {
        // Case 4: Delta = 0
        K = BB / AA;
        outPutVal = (-inputCoeffs[1] / inputCoeffs[0] + K);
        if ((outPutVal > FLOAT_EPS) && (outPutVal <= 3.2)) {
            return outPutVal;
        }
        outPutVal = (-K / 2);
        if ((outPutVal > FLOAT_EPS) && (outPutVal <= 3.2)) {
            return outPutVal;
        }
        return 0.0f;
    } else {
        // Exception handling
        return 0.0f;
    }
}

using utils::toUnderlying;

Vibrator::Vibrator(std::unique_ptr<HwApi> hwapi, std::unique_ptr<HwCal> hwcal)
    : mHwApi(std::move(hwapi)), mHwCal(std::move(hwcal)) {
    std::string autocal;
    uint32_t lraPeriod = 0, lpTrigSupport = 0;
    bool hasEffectCoeffs = false;
    std::array<float, 4> effectCoeffs = {0};

    if (!mHwApi->setState(true)) {
        ALOGE("Failed to set state (%d): %s", errno, strerror(errno));
    }

    if (mHwCal->getAutocal(&autocal)) {
        mHwApi->setAutocal(autocal);
    }
    mHwCal->getLraPeriod(&lraPeriod);

    mHwCal->getCloseLoopThreshold(&mCloseLoopThreshold);
    mHwCal->getDynamicConfig(&mDynamicConfig);

    if (mDynamicConfig) {
        uint8_t i = 0;
        float tempVolLevel = 0.0f;
        float tempAmpMax = 0.0f;
        uint32_t longFreqencyShift = 0;
        uint32_t shortVoltageMax = 0, longVoltageMax = 0;
        uint32_t shape = 0;

        mHwCal->getLongFrequencyShift(&longFreqencyShift);
        mHwCal->getShortVoltageMax(&shortVoltageMax);
        mHwCal->getLongVoltageMax(&longVoltageMax);

        hasEffectCoeffs = mHwCal->getEffectCoeffs(&effectCoeffs);
        for (i = 0; i < 5; i++) {
            if (hasEffectCoeffs) {
                // Use linear approach to get the target voltage levels
                if ((effectCoeffs[2] == 0) && (effectCoeffs[3] == 0)) {
                    tempVolLevel =
                        targetGToVlevelsUnderLinearEquation(effectCoeffs, EFFECT_TARGET_G[i]);
                    mEffectTargetOdClamp[i] = convertLevelsToOdClamp(tempVolLevel, lraPeriod);
                } else {
                    // Use cubic approach to get the target voltage levels
                    tempVolLevel =
                        targetGToVlevelsUnderCubicEquation(effectCoeffs, EFFECT_TARGET_G[i]);
                    mEffectTargetOdClamp[i] = convertLevelsToOdClamp(tempVolLevel, lraPeriod);
                }
            } else {
                mEffectTargetOdClamp[i] = shortVoltageMax;
            }
        }
        // Add a boundary protection for level 5 only, since
        // some devices might not be able to reach the maximum target G
        if ((mEffectTargetOdClamp[4] <= 0) || (mEffectTargetOdClamp[4] > 161)) {
            mEffectTargetOdClamp[4] = shortVoltageMax;
        }

        mHwCal->getEffectShape(&shape);
        mEffectConfig.reset(new VibrationConfig({
            .shape = (shape == UINT32_MAX) ? WaveShape::SINE : static_cast<WaveShape>(shape),
            .odClamp = &mEffectTargetOdClamp[0],
            .olLraPeriod = lraPeriod,
        }));

        mSteadyTargetOdClamp = longVoltageMax;
        if ((mHwCal->getSteadyAmpMax(&tempAmpMax)) && (tempAmpMax > STEADY_TARGET_G[0])) {
            tempVolLevel = round((STEADY_TARGET_G[0] / tempAmpMax) * longVoltageMax);
            mSteadyTargetOdClamp = (tempVolLevel < STEADY_VOLTAGE_LOWER_BOUND)
                                       ? STEADY_VOLTAGE_LOWER_BOUND
                                       : tempVolLevel;
        }
        mHwCal->getSteadyShape(&shape);
        mSteadyConfig.reset(new VibrationConfig({
            .shape = (shape == UINT32_MAX) ? WaveShape::SQUARE : static_cast<WaveShape>(shape),
            .odClamp = &mSteadyTargetOdClamp,
            .olLraPeriod = lraPeriod,
        }));
        mSteadyOlLraPeriod = lraPeriod;
        // 1. Change long lra period to frequency
        // 2. Get frequency': subtract the frequency shift from the frequency
        // 3. Get final long lra period after put the frequency' to formula
        mSteadyOlLraPeriodShift =
            freqPeriodFormula(freqPeriodFormula(lraPeriod) - longFreqencyShift);
    } else {
        mHwApi->setOlLraPeriod(lraPeriod);
    }

    mHwCal->getClickDuration(&mClickDuration);
    mHwCal->getTickDuration(&mTickDuration);
    mHwCal->getDoubleClickDuration(&mDoubleClickDuration);
    mHwCal->getHeavyClickDuration(&mHeavyClickDuration);

    // This enables effect #1 from the waveform library to be triggered by SLPI
    // while the AP is in suspend mode
    // For default setting, we will enable this feature if that project did not
    // set the lptrigger config
    mHwCal->getTriggerEffectSupport(&lpTrigSupport);
    if (!mHwApi->setLpTriggerEffect(lpTrigSupport)) {
        ALOGW("Failed to set LP trigger mode (%d): %s", errno, strerror(errno));
    }
}

ndk::ScopedAStatus Vibrator::getCapabilities(int32_t *_aidl_return) {
    ATRACE_NAME("Vibrator::getCapabilities");
    int32_t ret = 0;
    if (mHwApi->hasRtpInput()) {
        ret |= IVibrator::CAP_AMPLITUDE_CONTROL;
    }
    *_aidl_return = ret;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::on(uint32_t timeoutMs, const char mode[],
                                const std::unique_ptr<VibrationConfig> &config,
                                const int8_t volOffset) {
    LoopControl loopMode = LoopControl::OPEN;

    // Open-loop mode is used for short click for over-drive
    // Close-loop mode is used for long notification for stability
    if (mode == RTP_MODE && timeoutMs > mCloseLoopThreshold) {
        loopMode = LoopControl::CLOSE;
    }

    mHwApi->setCtrlLoop(toUnderlying(loopMode));
    if (!mHwApi->setDuration(timeoutMs)) {
        ALOGE("Failed to set duration (%d): %s", errno, strerror(errno));
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    mHwApi->setMode(mode);
    if (config != nullptr) {
        mHwApi->setLraWaveShape(toUnderlying(config->shape));
        mHwApi->setOdClamp(config->odClamp[volOffset]);
        mHwApi->setOlLraPeriod(config->olLraPeriod);
    }

    if (!mHwApi->setActivate(1)) {
        ALOGE("Failed to activate (%d): %s", errno, strerror(errno));
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::on(int32_t timeoutMs,
                                const std::shared_ptr<IVibratorCallback> &callback) {
    ATRACE_NAME("Vibrator::on");
    if (callback) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    if (mDynamicConfig) {
        int usbTemp = 0;
        mHwApi->getUsbTemp(&usbTemp);
        if (usbTemp > TEMP_UPPER_BOUND) {
            mSteadyConfig->odClamp = &mSteadyTargetOdClamp;
            mSteadyConfig->olLraPeriod = mSteadyOlLraPeriod;
        } else if (usbTemp < TEMP_LOWER_BOUND) {
            mSteadyConfig->odClamp = &STEADY_VOLTAGE_LOWER_BOUND;
            mSteadyConfig->olLraPeriod = mSteadyOlLraPeriodShift;
        }
    }

    return on(timeoutMs, RTP_MODE, mSteadyConfig, 0);
}

ndk::ScopedAStatus Vibrator::off() {
    ATRACE_NAME("Vibrator::off");
    if (!mHwApi->setActivate(0)) {
        ALOGE("Failed to turn vibrator off (%d): %s", errno, strerror(errno));
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setAmplitude(float amplitude) {
    ATRACE_NAME("Vibrator::setAmplitude");
    if (amplitude <= 0.0f || amplitude > 1.0f) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    int32_t rtp_input = std::round(amplitude * (MAX_RTP_INPUT - MIN_RTP_INPUT) + MIN_RTP_INPUT);

    if (!mHwApi->setRtpInput(rtp_input)) {
        ALOGE("Failed to set amplitude (%d): %s", errno, strerror(errno));
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setExternalControl(bool enabled) {
    ATRACE_NAME("Vibrator::setExternalControl");
    ALOGE("Not support in DRV2624 solution, %d", enabled);
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

binder_status_t Vibrator::dump(int fd, const char **args, uint32_t numArgs) {
    if (fd < 0) {
        ALOGE("Called debug() with invalid fd.");
        return STATUS_OK;
    }

    (void)args;
    (void)numArgs;

    dprintf(fd, "AIDL:\n");

    dprintf(fd, "  Close Loop Thresh: %" PRIu32 "\n", mCloseLoopThreshold);
    if (mSteadyConfig) {
        dprintf(fd, "  Steady Shape: %" PRIu32 "\n", mSteadyConfig->shape);
        dprintf(fd, "  Steady OD Clamp: %" PRIu32 "\n", mSteadyConfig->odClamp[0]);
        dprintf(fd, "  Steady OL LRA Period: %" PRIu32 "\n", mSteadyConfig->olLraPeriod);
    }
    if (mEffectConfig) {
        dprintf(fd, "  Effect Shape: %" PRIu32 "\n", mEffectConfig->shape);
        dprintf(fd,
                "  Effect OD Clamp: %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32 "\n",
                mEffectConfig->odClamp[0], mEffectConfig->odClamp[1], mEffectConfig->odClamp[2],
                mEffectConfig->odClamp[3], mEffectConfig->odClamp[4]);
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
    return STATUS_OK;
}

ndk::ScopedAStatus Vibrator::getSupportedEffects(std::vector<Effect> *_aidl_return) {
    *_aidl_return = {Effect::TEXTURE_TICK, Effect::TICK, Effect::CLICK, Effect::HEAVY_CLICK,
                     Effect::DOUBLE_CLICK};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::perform(Effect effect, EffectStrength strength,
                                     const std::shared_ptr<IVibratorCallback> &callback,
                                     int32_t *_aidl_return) {
    ATRACE_NAME("Vibrator::perform");
    ndk::ScopedAStatus status;

    if (callback) {
        status = ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    } else {
        status = performEffect(effect, strength, _aidl_return);
    }

    if (!status.isOk()) {
        *_aidl_return = 0;
    }

    return status;
}

ndk::ScopedAStatus Vibrator::performEffect(Effect effect, EffectStrength strength,
                                           int32_t *outTimeMs) {
    ndk::ScopedAStatus status;
    uint32_t timeMS = 0;
    int8_t volOffset;

    switch (strength) {
        case EffectStrength::LIGHT:
            volOffset = 0;
            break;
        case EffectStrength::MEDIUM:
            volOffset = 1;
            break;
        case EffectStrength::STRONG:
            volOffset = 1;
            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    switch (effect) {
        case Effect::TEXTURE_TICK:
            mHwApi->setSequencer(WAVEFORM_TICK_EFFECT_SEQ);
            timeMS = mTickDuration;
            volOffset = TEXTURE_TICK;
            break;
        case Effect::CLICK:
            mHwApi->setSequencer(WAVEFORM_CLICK_EFFECT_SEQ);
            timeMS = mClickDuration;
            volOffset += CLICK;
            break;
        case Effect::DOUBLE_CLICK:
            mHwApi->setSequencer(WAVEFORM_DOUBLE_CLICK_EFFECT_SEQ);
            timeMS = mDoubleClickDuration;
            volOffset += CLICK;
            break;
        case Effect::TICK:
            mHwApi->setSequencer(WAVEFORM_TICK_EFFECT_SEQ);
            timeMS = mTickDuration;
            volOffset += TICK;
            break;
        case Effect::HEAVY_CLICK:
            mHwApi->setSequencer(WAVEFORM_HEAVY_CLICK_EFFECT_SEQ);
            timeMS = mHeavyClickDuration;
            volOffset += HEAVY_CLICK;
            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    status = on(timeMS, WAVEFORM_MODE, mEffectConfig, volOffset);
    if (!status.isOk()) {
        return status;
    }

    *outTimeMs = timeMS;

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getSupportedAlwaysOnEffects(std::vector<Effect> * /*_aidl_return*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::alwaysOnEnable(int32_t /*id*/, Effect /*effect*/,
                                            EffectStrength /*strength*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}
ndk::ScopedAStatus Vibrator::alwaysOnDisable(int32_t /*id*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getCompositionDelayMax(int32_t * /*maxDelayMs*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getCompositionSizeMax(int32_t * /*maxSize*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getSupportedPrimitives(
        std::vector<CompositePrimitive> * /*supported*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::getPrimitiveDuration(CompositePrimitive /*primitive*/,
                                                  int32_t * /*durationMs*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Vibrator::compose(const std::vector<CompositeEffect> & /*composite*/,
                                     const std::shared_ptr<IVibratorCallback> & /*callback*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
