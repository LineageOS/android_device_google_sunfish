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
#ifndef ANDROID_HARDWARE_VIBRATOR_V1_3_VIBRATOR_H
#define ANDROID_HARDWARE_VIBRATOR_V1_3_VIBRATOR_H

#include <android/hardware/vibrator/1.3/IVibrator.h>
#include <hidl/Status.h>

#include <fstream>

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_3 {
namespace implementation {

class Vibrator : public IVibrator {
  public:
    // APIs for interfacing with the kernel driver.
    class HwApi {
      public:
        virtual ~HwApi() = default;
        // Stores the LRA resonant frequency to be used for PWLE playback
        // and click compensation.
        virtual bool setF0(uint32_t value) = 0;
        // Stores the LRA series resistance to be used for click
        // compensation.
        virtual bool setRedc(uint32_t value) = 0;
        // Stores the LRA Q factor to be used for Q-dependent waveform
        // selection.
        virtual bool setQ(uint32_t value) = 0;
        // Activates/deactivates the vibrator for durations specified by
        // setDuration().
        virtual bool setActivate(bool value) = 0;
        // Specifies the vibration duration in milliseconds.
        virtual bool setDuration(uint32_t value) = 0;
        // Reports the duration of the waveform selected by
        // setEffectIndex(), measured in 48-kHz periods.
        virtual bool getEffectDuration(uint32_t *value) = 0;
        // Selects the waveform associated with vibration calls from
        // the Android vibrator HAL.
        virtual bool setEffectIndex(uint32_t value) = 0;
        // Specifies an array of waveforms, delays, and repetition markers to
        // generate complex waveforms.
        virtual bool setEffectQueue(std::string value) = 0;
        // Reports whether setEffectScale() is supported.
        virtual bool hasEffectScale() = 0;
        // Indicates the number of 0.125-dB steps of attenuation to apply to
        // waveforms triggered in response to vibration calls from the
        // Android vibrator HAL.
        virtual bool setEffectScale(uint32_t value) = 0;
        // Indicates the number of 0.125-dB steps of attenuation to apply to
        // any output waveform (additive to all other set*Scale()
        // controls).
        virtual bool setGlobalScale(uint32_t value) = 0;
        // Specifies the active state of the vibrator
        // (true = enabled, false= disabled).
        virtual bool setState(bool value) = 0;
        // Reports whether getAspEnable()/setAspEnable() is supported.
        virtual bool hasAspEnable() = 0;
        // Enables/disables ASP playback.
        virtual bool getAspEnable(bool *value) = 0;
        // Reports enabled/disabled state of ASP playback.
        virtual bool setAspEnable(bool value) = 0;
        // Selects the waveform associated with a GPIO1 falling edge.
        virtual bool setGpioFallIndex(uint32_t value) = 0;
        // Indicates the number of 0.125-dB steps of attenuation to apply to
        // waveforms triggered in response to a GPIO1 falling edge.
        virtual bool setGpioFallScale(uint32_t value) = 0;
        // Selects the waveform associated with a GPIO1 rising edge.
        virtual bool setGpioRiseIndex(uint32_t value) = 0;
        // Indicates the number of 0.125-dB steps of attenuation to apply to
        // waveforms triggered in response to a GPIO1 rising edge.
        virtual bool setGpioRiseScale(uint32_t value) = 0;
        // Emit diagnostic information to the given file.
        virtual void debug(int fd) = 0;
    };

    // APIs for obtaining calibration/configuration data from persistent memory.
    class HwCal {
      public:
        virtual ~HwCal() = default;
        // Obtains the LRA resonant frequency to be used for PWLE playback
        // and click compensation.
        virtual bool getF0(uint32_t *value) = 0;
        // Obtains the LRA series resistance to be used for click
        // compensation.
        virtual bool getRedc(uint32_t *value) = 0;
        // Obtains the LRA Q factor to be used for Q-dependent waveform
        // selection.
        virtual bool getQ(uint32_t *value) = 0;
        // Obtains the discreet voltage levels to be applied for the various
        // waveforms, in units of 1%.
        virtual bool getVolLevels(std::array<uint32_t, 6> *value) = 0;
        // Emit diagnostic information to the given file.
        virtual void debug(int fd) = 0;
    };

  public:
    Vibrator(std::unique_ptr<HwApi> hwapi, std::unique_ptr<HwCal> hwcal);

    // Methods from ::android::hardware::vibrator::V1_0::IVibrator follow.
    using Status = ::android::hardware::vibrator::V1_0::Status;
    Return<Status> on(uint32_t timeoutMs) override;
    Return<Status> off() override;
    Return<bool> supportsAmplitudeControl() override;
    Return<Status> setAmplitude(uint8_t amplitude) override;

    // Methods from ::android::hardware::vibrator::V1_3::IVibrator follow.
    Return<bool> supportsExternalControl() override;
    Return<Status> setExternalControl(bool enabled) override;

    using EffectStrength = ::android::hardware::vibrator::V1_0::EffectStrength;
    Return<void> perform(V1_0::Effect effect, EffectStrength strength,
                         perform_cb _hidl_cb) override;
    Return<void> perform_1_1(V1_1::Effect_1_1 effect, EffectStrength strength,
                             perform_cb _hidl_cb) override;
    Return<void> perform_1_2(V1_2::Effect effect, EffectStrength strength,
                             perform_cb _hidl_cb) override;
    Return<void> perform_1_3(Effect effect, EffectStrength strength, perform_cb _hidl_cb) override;

    // Methods from ::android.hidl.base::V1_0::IBase follow.
    Return<void> debug(const hidl_handle &handle, const hidl_vec<hidl_string> &options) override;

  private:
    Return<Status> on(uint32_t timeoutMs, uint32_t effectIndex);
    template <typename T>
    Return<void> performWrapper(T effect, EffectStrength strength, perform_cb _hidl_cb);
    // set 'amplitude' based on an arbitrary scale determined by 'maximum'
    Return<Status> setEffectAmplitude(uint8_t amplitude, uint8_t maximum);
    Return<Status> setGlobalAmplitude(bool set);
    // 'simple' effects are those precompiled and loaded into the controller
    Return<Status> getSimpleDetails(Effect effect, EffectStrength strength, uint32_t *outTimeMs,
                                    uint32_t *outVolLevel);
    // 'compound' effects are those composed by stringing multiple 'simple' effects
    Return<Status> getCompoundDetails(Effect effect, EffectStrength strength, uint32_t *outTimeMs,
                                      uint32_t *outVolLevel, std::string *outEffectQueue);
    Return<Status> setEffectQueue(const std::string &effectQueue);
    Return<void> performEffect(Effect effect, EffectStrength strength, perform_cb _hidl_cb);
    bool isUnderExternalControl();
    std::unique_ptr<HwApi> mHwApi;
    std::unique_ptr<HwCal> mHwCal;
    std::array<uint32_t, 6> mVolLevels;
    uint32_t mSimpleEffectDuration;
};

}  // namespace implementation
}  // namespace V1_3
}  // namespace vibrator
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_VIBRATOR_V1_3_VIBRATOR_H
