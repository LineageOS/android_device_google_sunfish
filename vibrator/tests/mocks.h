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
#ifndef ANDROID_HARDWARE_VIBRATOR_TEST_MOCKS_H
#define ANDROID_HARDWARE_VIBRATOR_TEST_MOCKS_H

#include "Vibrator.h"

class MockApi : public ::android::hardware::vibrator::V1_3::implementation::Vibrator::HwApi {
  public:
    MOCK_METHOD0(destructor, void());
    MOCK_METHOD1(setF0, bool(uint32_t value));
    MOCK_METHOD1(setRedc, bool(uint32_t value));
    MOCK_METHOD1(setQ, bool(uint32_t value));
    MOCK_METHOD1(setActivate, bool(bool value));
    MOCK_METHOD1(setDuration, bool(uint32_t value));
    MOCK_METHOD1(getEffectDuration, bool(uint32_t *value));
    MOCK_METHOD1(setEffectIndex, bool(uint32_t value));
    MOCK_METHOD1(setEffectQueue, bool(std::string value));
    MOCK_METHOD0(hasEffectScale, bool());
    MOCK_METHOD1(setEffectScale, bool(uint32_t value));
    MOCK_METHOD1(setGlobalScale, bool(uint32_t value));
    MOCK_METHOD1(setState, bool(bool value));
    MOCK_METHOD0(hasAspEnable, bool());
    MOCK_METHOD1(getAspEnable, bool(bool *value));
    MOCK_METHOD1(setAspEnable, bool(bool value));
    MOCK_METHOD1(setGpioFallIndex, bool(uint32_t value));
    MOCK_METHOD1(setGpioFallScale, bool(uint32_t value));
    MOCK_METHOD1(setGpioRiseIndex, bool(uint32_t value));
    MOCK_METHOD1(setGpioRiseScale, bool(uint32_t value));
    MOCK_METHOD1(debug, void(int fd));

    ~MockApi() override { destructor(); };
};

class MockCal : public ::android::hardware::vibrator::V1_3::implementation::Vibrator::HwCal {
  public:
    MOCK_METHOD0(destructor, void());
    MOCK_METHOD1(getF0, bool(uint32_t *value));
    MOCK_METHOD1(getRedc, bool(uint32_t *value));
    MOCK_METHOD1(getQ, bool(uint32_t *value));
    MOCK_METHOD1(getVolLevels, bool(std::array<uint32_t, 6> *value));
    MOCK_METHOD1(debug, void(int fd));

    ~MockCal() override { destructor(); };
};

#endif  // ANDROID_HARDWARE_VIBRATOR_TEST_MOCKS_H
