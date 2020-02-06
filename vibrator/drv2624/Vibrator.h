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
#pragma once

#include <aidl/android/hardware/vibrator/BnVibrator.h>

#include <fstream>

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

class Vibrator : public BnVibrator {
  public:
    // APIs for interfacing with the kernel driver.
    class HwApi {
      public:
        virtual ~HwApi() = default;
        // Stores the COMP, BEMF, and GAIN calibration values to use.
        //   <COMP> <BEMF> <GAIN>
        virtual bool setAutocal(std::string value) = 0;
        // Stores the open-loop LRA frequency to be used.
        virtual bool setOlLraPeriod(uint32_t value) = 0;
        // Activates/deactivates the vibrator for durations specified by
        // setDuration().
        virtual bool setActivate(bool value) = 0;
        // Specifies the vibration duration in milliseconds.
        virtual bool setDuration(uint32_t value) = 0;
        // Specifies the active state of the vibrator
        // (true = enabled, false = disabled).
        virtual bool setState(bool value) = 0;
        // Reports whether setRtpInput() is supported.
        virtual bool hasRtpInput() = 0;
        // Specifies the playback amplitude of the haptic waveforms in RTP mode.
        // Negative numbers indicates braking.
        virtual bool setRtpInput(int8_t value) = 0;
        // Specifies the mode of operation.
        //   rtp        - RTP Mode
        //   waveform   - Waveform Sequencer Mode
        //   diag       - Diagnostics Routine
        //   autocal    - Automatic Level Calibration Routine
        virtual bool setMode(std::string value) = 0;
        // Specifies a waveform sequence in index-count pairs.
        //   <index-1> <count-1> [<index-2> <cound-2> ...]
        virtual bool setSequencer(std::string value) = 0;
        // Specifies the scaling of effects in Waveform mode.
        //   0 - 100%
        //   1 - 75%
        //   2 - 50%
        //   3 - 25%
        virtual bool setScale(uint8_t value) = 0;
        // Selects either closed loop or open loop mode.
        // (true = open, false = closed).
        virtual bool setCtrlLoop(bool value) = 0;
        // Specifies waveform index to be played in low-power trigger mode.
        //   0  - Disabled
        //   1+ - Waveform Index
        virtual bool setLpTriggerEffect(uint32_t value) = 0;
        // Specifies which shape to use for driving the LRA when in open loop
        // mode.
        //   0 - Square Wave
        //   1 - Sine Wave
        virtual bool setLraWaveShape(uint32_t value) = 0;
        // Specifies the maximum voltage for automatic overdrive and automatic
        // braking periods.
        virtual bool setOdClamp(uint32_t value) = 0;
        // Get battery temperature sensor value
        virtual bool getUsbTemp(int32_t *value) = 0;
        // Emit diagnostic information to the given file.
        virtual void debug(int fd) = 0;
    };

    // APIs for obtaining calibration/configuration data from persistent memory.
    class HwCal {
      public:
        virtual ~HwCal() = default;
        // Obtains the COMP, BEMF, and GAIN calibration values to use.
        virtual bool getAutocal(std::string *value) = 0;
        // Obtains the open-loop LRA frequency to be used.
        virtual bool getLraPeriod(uint32_t *value) = 0;
        // Obtains the effect coeffs to calculate the target voltage
        virtual bool getEffectCoeffs(std::array<float, 4> *value) = 0;
        // Obtain the max steady G value
        virtual bool getSteadyAmpMax(float *value) = 0;
        // Obtains threshold in ms, above which close-loop should be used.
        virtual bool getCloseLoopThreshold(uint32_t *value) = 0;
        // Obtains dynamic/static configuration choice.
        virtual bool getDynamicConfig(bool *value) = 0;
        // Obtains LRA frequency shift for long (steady) vibrations.
        virtual bool getLongFrequencyShift(uint32_t *value) = 0;
        // Obtains maximum voltage for short (effect) vibrations
        virtual bool getShortVoltageMax(uint32_t *value) = 0;
        // Obtains maximum voltage for long (steady) vibrations
        virtual bool getLongVoltageMax(uint32_t *value) = 0;
        // Obtains the duration for the click effect
        virtual bool getClickDuration(uint32_t *value) = 0;
        // Obtains the duration for the tick effect
        virtual bool getTickDuration(uint32_t *value) = 0;
        // Obtains the duration for the double-click effect
        virtual bool getDoubleClickDuration(uint32_t *value) = 0;
        // Obtains the duration for the heavy-click effect
        virtual bool getHeavyClickDuration(uint32_t *value) = 0;
        // Obtains the wave shape for effect haptics
        virtual bool getEffectShape(uint32_t *value) = 0;
        // Obtains the wave shape for steady vibration
        virtual bool getSteadyShape(uint32_t *value) = 0;
        // Obtains the trigger effect support
        virtual bool getTriggerEffectSupport(uint32_t *value) = 0;
        // Emit diagnostic information to the given file.
        virtual void debug(int fd) = 0;
    };

  private:
    enum class LoopControl : bool {
        CLOSE = false,
        OPEN = true,
    };

    enum class WaveShape : uint32_t {
        SQUARE = 0,
        SINE = 1,
    };

    struct VibrationConfig {
        WaveShape shape;
        uint32_t *odClamp;
        uint32_t olLraPeriod;
    };

    enum OdClampOffset : uint32_t {
        TEXTURE_TICK,
        TICK,
        CLICK,
        HEAVY_CLICK,
    };

  public:
    Vibrator(std::unique_ptr<HwApi> hwapi, std::unique_ptr<HwCal> hwcal);

    ndk::ScopedAStatus getCapabilities(int32_t *_aidl_return) override;
    ndk::ScopedAStatus off() override;
    ndk::ScopedAStatus on(int32_t timeoutMs,
                          const std::shared_ptr<IVibratorCallback> &callback) override;
    ndk::ScopedAStatus perform(Effect effect, EffectStrength strength,
                               const std::shared_ptr<IVibratorCallback> &callback,
                               int32_t *_aidl_return) override;
    ndk::ScopedAStatus getSupportedEffects(std::vector<Effect> *_aidl_return) override;
    ndk::ScopedAStatus setAmplitude(float amplitude) override;
    ndk::ScopedAStatus setExternalControl(bool enabled) override;
    ndk::ScopedAStatus getCompositionDelayMax(int32_t *maxDelayMs);
    ndk::ScopedAStatus getCompositionSizeMax(int32_t *maxSize);
    ndk::ScopedAStatus getSupportedPrimitives(std::vector<CompositePrimitive> *supported) override;
    ndk::ScopedAStatus getPrimitiveDuration(CompositePrimitive primitive,
                                            int32_t *durationMs) override;
    ndk::ScopedAStatus compose(const std::vector<CompositeEffect> &composite,
                               const std::shared_ptr<IVibratorCallback> &callback) override;
    ndk::ScopedAStatus getSupportedAlwaysOnEffects(std::vector<Effect> *_aidl_return) override;
    ndk::ScopedAStatus alwaysOnEnable(int32_t id, Effect effect, EffectStrength strength) override;
    ndk::ScopedAStatus alwaysOnDisable(int32_t id) override;

    binder_status_t dump(int fd, const char **args, uint32_t numArgs) override;

  private:
    ndk::ScopedAStatus on(uint32_t timeoutMs, const char mode[],
                          const std::unique_ptr<VibrationConfig> &config, const int8_t volOffset);
    ndk::ScopedAStatus performEffect(Effect effect, EffectStrength strength, int32_t *outTimeMs);

    std::unique_ptr<HwApi> mHwApi;
    std::unique_ptr<HwCal> mHwCal;
    uint32_t mCloseLoopThreshold;
    std::unique_ptr<VibrationConfig> mSteadyConfig;
    std::unique_ptr<VibrationConfig> mEffectConfig;
    uint32_t mClickDuration;
    uint32_t mTickDuration;
    uint32_t mDoubleClickDuration;
    uint32_t mHeavyClickDuration;
    std::array<uint32_t, 5> mEffectTargetOdClamp;
    uint32_t mSteadyTargetOdClamp;
    uint32_t mSteadyOlLraPeriod;
    uint32_t mSteadyOlLraPeriodShift;
    bool mDynamicConfig;
};

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
