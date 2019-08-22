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
#ifndef ANDROID_HARDWARE_VIBRATOR_HARDWARE_H
#define ANDROID_HARDWARE_VIBRATOR_HARDWARE_H

#include "Vibrator.h"
#include "utils.h"

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_3 {
namespace implementation {

class HwApi : public Vibrator::HwApi {
  public:
    HwApi();
    bool setF0(uint32_t value) override { return set(value, mF0); }
    bool setRedc(uint32_t value) override { return set(value, mRedc); }
    bool setQ(uint32_t value) override { return set(value, mQ); }
    bool setActivate(bool value) override { return set(value, mActivate); }
    bool setDuration(uint32_t value) override { return set(value, mDuration); }
    bool getEffectDuration(uint32_t *value) override { return get(value, mEffectDuration); }
    bool setEffectIndex(uint32_t value) override { return set(value, mEffectIndex); }
    bool setEffectQueue(std::string value) override { return set(value, mEffectQueue); }
    bool hasEffectScale() override { return has(mEffectScale); }
    bool setEffectScale(uint32_t value) override { return set(value, mEffectScale); }
    bool setGlobalScale(uint32_t value) override { return set(value, mGlobalScale); }
    bool setState(bool value) override { return set(value, mState); }
    bool hasAspEnable() override { return has(mAspEnable); }
    bool getAspEnable(bool *value) override { return get(value, mAspEnable); }
    bool setAspEnable(bool value) override { return set(value, mAspEnable); }
    bool setGpioFallIndex(uint32_t value) override { return set(value, mGpioFallIndex); }
    bool setGpioFallScale(uint32_t value) override { return set(value, mGpioFallScale); }
    bool setGpioRiseIndex(uint32_t value) override { return set(value, mGpioRiseIndex); }
    bool setGpioRiseScale(uint32_t value) override { return set(value, mGpioRiseScale); }
    void debug(int fd) override;

  private:
    template <typename T>
    bool has(T &stream);
    template <typename T, typename U>
    bool get(T *value, U &stream);
    template <typename T, typename U>
    bool set(const T &value, U &stream);

  private:
    std::map<void *, std::string> mNames;
    std::ofstream mF0;
    std::ofstream mRedc;
    std::ofstream mQ;
    std::ofstream mActivate;
    std::ofstream mDuration;
    std::ifstream mEffectDuration;
    std::ofstream mEffectIndex;
    std::ofstream mEffectQueue;
    std::ofstream mEffectScale;
    std::ofstream mGlobalScale;
    std::ofstream mState;
    std::fstream mAspEnable;
    std::ofstream mGpioFallIndex;
    std::ofstream mGpioFallScale;
    std::ofstream mGpioRiseIndex;
    std::ofstream mGpioRiseScale;
};

class HwCal : public Vibrator::HwCal {
  private:
    static constexpr char F0_CONFIG[] = "f0_measured";
    static constexpr char REDC_CONFIG[] = "redc_measured";
    static constexpr char Q_CONFIG[] = "q_measured";
    static constexpr char Q_INDEX[] = "q_index";
    static constexpr char VOLTAGES_CONFIG[] = "v_levels";

    static constexpr uint32_t Q_FLOAT_TO_FIXED = 1 << 16;
    static constexpr float Q_INDEX_TO_FLOAT = 1.5f;
    static constexpr uint32_t Q_INDEX_TO_FIXED = Q_INDEX_TO_FLOAT * Q_FLOAT_TO_FIXED;
    static constexpr uint32_t Q_INDEX_OFFSET = 2.0f * Q_FLOAT_TO_FIXED;

    static constexpr uint32_t Q_DEFAULT = 15.5 * Q_FLOAT_TO_FIXED;
    static constexpr std::array<uint32_t, 6> V_LEVELS_DEFAULT = {60, 70, 80, 90, 100, 76};

  public:
    HwCal();
    bool getF0(uint32_t *value) override { return get(F0_CONFIG, value); }
    bool getRedc(uint32_t *value) override { return get(REDC_CONFIG, value); }
    bool getQ(uint32_t *value) override {
        if (get(Q_CONFIG, value)) {
            return true;
        }
        if (get(Q_INDEX, value)) {
            *value = *value * Q_INDEX_TO_FIXED + Q_INDEX_OFFSET;
            return true;
        }
        *value = Q_DEFAULT;
        return true;
    }
    bool getVolLevels(std::array<uint32_t, 6> *value) override {
        if (get(VOLTAGES_CONFIG, value)) {
            return true;
        }
        *value = V_LEVELS_DEFAULT;
        return true;
    }
    void debug(int fd) override;

  private:
    template <typename T>
    bool get(const char *key, T *value);

  private:
    std::map<std::string, std::string> mCalData;
};

}  // namespace implementation
}  // namespace V1_3
}  // namespace vibrator
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_VIBRATOR_HARDWARE_H
