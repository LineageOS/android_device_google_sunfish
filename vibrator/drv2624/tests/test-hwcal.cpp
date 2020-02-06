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

#include <android-base/file.h>
#include <android-base/properties.h>
#include <gtest/gtest.h>

#include <fstream>

#include "Hardware.h"

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

using ::android::base::SetProperty;
using ::android::base::WaitForProperty;

using ::testing::Test;

class HwCalTest : public Test {
  protected:
    static constexpr char PROPERTY_PREFIX[] = "test.vibrator.hal.";

    static constexpr uint32_t DEFAULT_LRA_PERIOD = 262;

    static constexpr uint32_t DEFAULT_FREQUENCY_SHIFT = 10;
    static constexpr uint32_t DEFAULT_VOLTAGE_MAX = 107;

    static constexpr uint32_t DEFAULT_CLICK_DURATION_MS = 6;
    static constexpr uint32_t DEFAULT_TICK_DURATION_MS = 2;
    static constexpr uint32_t DEFAULT_DOUBLE_CLICK_DURATION_MS = 135;
    static constexpr uint32_t DEFAULT_HEAVY_CLICK_DURATION_MS = 8;

  public:
    void SetUp() override {
        setenv("PROPERTY_PREFIX", PROPERTY_PREFIX, true);
        setenv("CALIBRATION_FILEPATH", mCalFile.path, true);
    }

  private:
    template <typename T>
    static void pack(std::ostream &stream, const T &value, std::string lpad, std::string rpad) {
        stream << lpad << value << rpad;
    }

  protected:
    void createHwCal() { mHwCal = std::make_unique<HwCal>(); }

    template <typename T>
    void write(const std::string key, const T &value, std::string lpad = " ",
               std::string rpad = "") {
        std::ofstream calfile{mCalFile.path, std::ios_base::app};
        calfile << key << ":";
        pack(calfile, value, lpad, rpad);
        calfile << std::endl;
    }

    void unlink() { ::unlink(mCalFile.path); }

  protected:
    std::unique_ptr<Vibrator::HwCal> mHwCal;
    TemporaryFile mCalFile;
};

TEST_F(HwCalTest, closeloop_present) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = std::rand();
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "closeloop.threshold", std::to_string(expect)));

    createHwCal();

    EXPECT_TRUE(mHwCal->getCloseLoopThreshold(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, closeloop_missing) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = UINT32_MAX;
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "closeloop.threshold", std::string()));

    createHwCal();

    EXPECT_TRUE(mHwCal->getCloseLoopThreshold(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, dynamicconfig_presentFalse) {
    std::string prefix{PROPERTY_PREFIX};
    bool expect = false;
    bool actual = !expect;

    EXPECT_TRUE(SetProperty(prefix + "config.dynamic", "0"));

    createHwCal();

    EXPECT_TRUE(mHwCal->getDynamicConfig(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, dynamicconfig_presentTrue) {
    std::string prefix{PROPERTY_PREFIX};
    bool expect = true;
    bool actual = !expect;

    EXPECT_TRUE(SetProperty(prefix + "config.dynamic", "1"));

    createHwCal();

    EXPECT_TRUE(mHwCal->getDynamicConfig(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, dynamicconfig_missing) {
    std::string prefix{PROPERTY_PREFIX};
    bool expect = false;
    bool actual = !expect;

    EXPECT_TRUE(SetProperty(prefix + "config.dynamic", std::string()));

    createHwCal();

    EXPECT_TRUE(mHwCal->getDynamicConfig(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, freqshift_present) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = std::rand();
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "long.frequency.shift", std::to_string(expect)));

    createHwCal();

    EXPECT_TRUE(mHwCal->getLongFrequencyShift(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, freqshift_missing) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = DEFAULT_FREQUENCY_SHIFT;
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "long.frequency.shift", std::string()));

    createHwCal();

    EXPECT_TRUE(mHwCal->getLongFrequencyShift(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, shortvolt_present) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = std::rand();
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "short.voltage", std::to_string(expect)));

    createHwCal();

    EXPECT_TRUE(mHwCal->getShortVoltageMax(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, shortvolt_missing) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = DEFAULT_VOLTAGE_MAX;
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "short.voltage", std::string()));

    createHwCal();

    EXPECT_TRUE(mHwCal->getShortVoltageMax(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, longvolt_present) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = std::rand();
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "long.voltage", std::to_string(expect)));

    createHwCal();

    EXPECT_TRUE(mHwCal->getLongVoltageMax(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, longvolt_missing) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = DEFAULT_VOLTAGE_MAX;
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "long.voltage", std::string()));

    createHwCal();

    EXPECT_TRUE(mHwCal->getLongVoltageMax(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, click_present) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = std::rand();
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "click.duration", std::to_string(expect)));

    createHwCal();

    EXPECT_TRUE(mHwCal->getClickDuration(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, click_missing) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = DEFAULT_CLICK_DURATION_MS;
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "click.duration", std::string()));

    createHwCal();

    EXPECT_TRUE(mHwCal->getClickDuration(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, tick_present) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = std::rand();
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "tick.duration", std::to_string(expect)));

    createHwCal();

    EXPECT_TRUE(mHwCal->getTickDuration(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, tick_missing) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = DEFAULT_TICK_DURATION_MS;
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "tick.duration", std::string()));

    createHwCal();

    EXPECT_TRUE(mHwCal->getTickDuration(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, doubleclick) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = DEFAULT_DOUBLE_CLICK_DURATION_MS;
    uint32_t actual = ~expect;

    createHwCal();

    EXPECT_TRUE(mHwCal->getDoubleClickDuration(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, heavyclick_present) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = std::rand();
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "heavyclick.duration", std::to_string(expect)));

    createHwCal();

    EXPECT_TRUE(mHwCal->getHeavyClickDuration(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, heavyclick_missing) {
    std::string prefix{PROPERTY_PREFIX};
    uint32_t expect = DEFAULT_HEAVY_CLICK_DURATION_MS;
    uint32_t actual = ~expect;

    EXPECT_TRUE(SetProperty(prefix + "heavyclick.duration", std::string()));

    createHwCal();

    EXPECT_TRUE(mHwCal->getHeavyClickDuration(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, autocal_present) {
    std::string expect = std::to_string(std::rand()) + " " + std::to_string(std::rand()) + " " +
                         std::to_string(std::rand());
    std::string actual = "";

    write("autocal", expect);

    createHwCal();

    EXPECT_TRUE(mHwCal->getAutocal(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, autocal_missing) {
    std::string actual;

    createHwCal();

    EXPECT_FALSE(mHwCal->getAutocal(&actual));
}

TEST_F(HwCalTest, lra_period_present) {
    uint32_t expect = std::rand();
    uint32_t actual = ~expect;

    write("lra_period", expect);

    createHwCal();

    EXPECT_TRUE(mHwCal->getLraPeriod(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, lra_period_missing) {
    uint32_t expect = DEFAULT_LRA_PERIOD;
    uint32_t actual = ~expect;

    createHwCal();

    EXPECT_TRUE(mHwCal->getLraPeriod(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, multiple) {
    std::string autocalExpect = std::to_string(std::rand()) + " " + std::to_string(std::rand()) +
                                " " + std::to_string(std::rand());
    std::string autocalActual = "";
    uint32_t lraPeriodExpect = std::rand();
    uint32_t lraPeriodActual = ~lraPeriodExpect;

    write("autocal", autocalExpect);
    write("lra_period", lraPeriodExpect);

    createHwCal();

    EXPECT_TRUE(mHwCal->getAutocal(&autocalActual));
    EXPECT_EQ(autocalExpect, autocalActual);
    EXPECT_TRUE(mHwCal->getLraPeriod(&lraPeriodActual));
    EXPECT_EQ(lraPeriodExpect, lraPeriodActual);
}

TEST_F(HwCalTest, trimming) {
    std::string autocalExpect = std::to_string(std::rand()) + " " + std::to_string(std::rand()) +
                                " " + std::to_string(std::rand());
    std::string autocalActual = "";
    uint32_t lraPeriodExpect = std::rand();
    uint32_t lraPeriodActual = ~lraPeriodExpect;

    write("autocal", autocalExpect, " \t", "\t ");
    write("lra_period", lraPeriodExpect, " \t", "\t ");

    createHwCal();

    EXPECT_TRUE(mHwCal->getAutocal(&autocalActual));
    EXPECT_EQ(autocalExpect, autocalActual);
    EXPECT_TRUE(mHwCal->getLraPeriod(&lraPeriodActual));
    EXPECT_EQ(lraPeriodExpect, lraPeriodActual);
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
