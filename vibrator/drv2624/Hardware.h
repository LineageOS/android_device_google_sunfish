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

#include "../common/HardwareBase.h"
#include "Vibrator.h"

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

class HwApi : public Vibrator::HwApi, private HwApiBase {
  public:
    static std::unique_ptr<HwApi> Create() {
        auto hwapi = std::unique_ptr<HwApi>(new HwApi());
        // the following streams are required
        if (!hwapi->mActivate.is_open() || !hwapi->mDuration.is_open() ||
            !hwapi->mState.is_open()) {
            return nullptr;
        }
        return hwapi;
    }

    bool setAutocal(std::string value) override { return set(value, &mAutocal); }
    bool setOlLraPeriod(uint32_t value) override { return set(value, &mOlLraPeriod); }
    bool setActivate(bool value) override { return set(value, &mActivate); }
    bool setDuration(uint32_t value) override { return set(value, &mDuration); }
    bool setState(bool value) override { return set(value, &mState); }
    bool hasRtpInput() override { return has(mRtpInput); }
    bool setRtpInput(int8_t value) override { return set(value, &mRtpInput); }
    bool setMode(std::string value) override { return set(value, &mMode); }
    bool setSequencer(std::string value) override { return set(value, &mSequencer); }
    bool setScale(uint8_t value) override { return set(value, &mScale); }
    bool setCtrlLoop(bool value) override { return set(value, &mCtrlLoop); }
    bool setLpTriggerEffect(uint32_t value) override { return set(value, &mLpTrigger); }
    bool setLraWaveShape(uint32_t value) override { return set(value, &mLraWaveShape); }
    bool setOdClamp(uint32_t value) override { return set(value, &mOdClamp); }
    bool getUsbTemp(int32_t *value) override { return get(value, &mUsbTemp); }
    void debug(int fd) override { HwApiBase::debug(fd); }

  private:
    HwApi() {
        open("device/autocal", &mAutocal);
        open("device/ol_lra_period", &mOlLraPeriod);
        open("activate", &mActivate);
        open("duration", &mDuration);
        open("state", &mState);
        open("device/rtp_input", &mRtpInput);
        open("device/mode", &mMode);
        open("device/set_sequencer", &mSequencer);
        open("device/scale", &mScale);
        open("device/ctrl_loop", &mCtrlLoop);
        open("device/lp_trigger_effect", &mLpTrigger);
        open("device/lra_wave_shape", &mLraWaveShape);
        open("device/od_clamp", &mOdClamp);
        // TODO: for future new architecture: b/149610125
        openFull("/sys/devices/virtual/thermal/tz-by-name/usbc-therm-monitor/temp", &mUsbTemp);
    }

  private:
    std::ofstream mAutocal;
    std::ofstream mOlLraPeriod;
    std::ofstream mActivate;
    std::ofstream mDuration;
    std::ofstream mState;
    std::ofstream mRtpInput;
    std::ofstream mMode;
    std::ofstream mSequencer;
    std::ofstream mScale;
    std::ofstream mCtrlLoop;
    std::ofstream mLpTrigger;
    std::ofstream mLraWaveShape;
    std::ofstream mOdClamp;
    std::ifstream mUsbTemp;
};

class HwCal : public Vibrator::HwCal, private HwCalBase {
  private:
    static constexpr char AUTOCAL_CONFIG[] = "autocal";
    static constexpr char LRA_PERIOD_CONFIG[] = "lra_period";
    static constexpr char EFFECT_COEFF_CONFIG[] = "haptic_coefficient";
    static constexpr char STEADY_AMP_MAX_CONFIG[] = "vibration_amp_max";

    static constexpr uint32_t WAVEFORM_CLICK_EFFECT_MS = 6;
    static constexpr uint32_t WAVEFORM_TICK_EFFECT_MS = 2;
    static constexpr uint32_t WAVEFORM_DOUBLE_CLICK_EFFECT_MS = 144;
    static constexpr uint32_t WAVEFORM_HEAVY_CLICK_EFFECT_MS = 8;

    static constexpr uint32_t DEFAULT_LRA_PERIOD = 262;
    static constexpr uint32_t DEFAULT_FREQUENCY_SHIFT = 10;
    static constexpr uint32_t DEFAULT_VOLTAGE_MAX = 107;  // 2.15V;
    static constexpr uint32_t DEFAULT_LP_TRIGGER_SUPPORT = 1;

  public:
    HwCal() {}

    bool getAutocal(std::string *value) override { return getPersist(AUTOCAL_CONFIG, value); }
    bool getLraPeriod(uint32_t *value) override {
        if (getPersist(LRA_PERIOD_CONFIG, value)) {
            return true;
        }
        *value = DEFAULT_LRA_PERIOD;
        return true;
    }
    bool getEffectCoeffs(std::array<float, 4> *value) override {
        if (getPersist(EFFECT_COEFF_CONFIG, value)) {
            return true;
        }
        return false;
    }
    bool getSteadyAmpMax(float *value) override {
        if (getPersist(STEADY_AMP_MAX_CONFIG, value)) {
            return true;
        }
        return false;
    }
    bool getCloseLoopThreshold(uint32_t *value) override {
        return getProperty("closeloop.threshold", value, UINT32_MAX);
        return true;
    }
    bool getDynamicConfig(bool *value) override {
        return getProperty("config.dynamic", value, false);
    }
    bool getLongFrequencyShift(uint32_t *value) override {
        return getProperty("long.frequency.shift", value, DEFAULT_FREQUENCY_SHIFT);
    }
    bool getShortVoltageMax(uint32_t *value) override {
        return getProperty("short.voltage", value, DEFAULT_VOLTAGE_MAX);
    }
    bool getLongVoltageMax(uint32_t *value) override {
        return getProperty("long.voltage", value, DEFAULT_VOLTAGE_MAX);
    }
    bool getClickDuration(uint32_t *value) override {
        return getProperty("click.duration", value, WAVEFORM_CLICK_EFFECT_MS);
    }
    bool getTickDuration(uint32_t *value) override {
        return getProperty("tick.duration", value, WAVEFORM_TICK_EFFECT_MS);
    }
    bool getDoubleClickDuration(uint32_t *value) override {
        *value = WAVEFORM_DOUBLE_CLICK_EFFECT_MS;
        return true;
    }
    bool getHeavyClickDuration(uint32_t *value) override {
        return getProperty("heavyclick.duration", value, WAVEFORM_HEAVY_CLICK_EFFECT_MS);
    }
    bool getEffectShape(uint32_t *value) override {
        return getProperty("effect.shape", value, UINT32_MAX);
    }
    bool getSteadyShape(uint32_t *value) override {
        return getProperty("steady.shape", value, UINT32_MAX);
    }
    bool getTriggerEffectSupport(uint32_t *value) override {
        return getProperty("lptrigger", value, DEFAULT_LP_TRIGGER_SUPPORT);
    }
    void debug(int fd) override { HwCalBase::debug(fd); }
};

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
