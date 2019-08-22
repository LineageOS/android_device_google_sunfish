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
#define LOG_TAG "android.hardware.vibrator@1.3-tests.sunfish"

#include <android-base/file.h>
#include <gtest/gtest.h>

#include <fstream>

#include "Hardware.h"

using namespace ::testing;

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_3 {
namespace implementation {

class HwCalTest : public Test {
  protected:
    static constexpr uint32_t Q_DEFAULT = 15.5f * (1 << 16);
    static constexpr std::array<uint32_t, 6> V_DEFAULT = {60, 70, 80, 90, 100, 76};

  public:
    void SetUp() override { setenv("CALIBRATION_FILEPATH", mCalFile.path, true); }

  private:
    static void pack(std::ostream &stream, const uint32_t &value, std::string lpad,
                     std::string rpad) {
        stream << lpad << value << rpad;
    }

    template <typename T, typename std::array<T, 0>::size_type N>
    static void pack(std::ostream &stream, const std::array<T, N> &value, std::string lpad,
                     std::string rpad) {
        for (auto &entry : value) {
            pack(stream, entry, lpad, rpad);
        }
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

TEST_F(HwCalTest, f0_measured) {
    uint32_t expect = std::rand();
    uint32_t actual = ~expect;

    write("f0_measured", expect);

    createHwCal();

    EXPECT_TRUE(mHwCal->getF0(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, f0_missing) {
    uint32_t actual;

    createHwCal();

    EXPECT_FALSE(mHwCal->getF0(&actual));
}

TEST_F(HwCalTest, redc_measured) {
    uint32_t expect = std::rand();
    uint32_t actual = ~expect;

    write("redc_measured", expect);

    createHwCal();

    EXPECT_TRUE(mHwCal->getRedc(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, redc_missing) {
    uint32_t actual;

    createHwCal();

    EXPECT_FALSE(mHwCal->getRedc(&actual));
}

TEST_F(HwCalTest, q_measured) {
    uint32_t expect = std::rand();
    uint32_t actual = ~expect;

    write("q_measured", expect);

    createHwCal();

    EXPECT_TRUE(mHwCal->getQ(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, q_index) {
    uint8_t value = std::rand();
    uint32_t expect = value * 1.5f * (1 << 16) + 2.0f * (1 << 16);
    uint32_t actual = ~expect;

    write("q_index", value);

    createHwCal();

    EXPECT_TRUE(mHwCal->getQ(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, q_missing) {
    uint32_t expect = Q_DEFAULT;
    uint32_t actual = ~expect;

    createHwCal();

    EXPECT_TRUE(mHwCal->getQ(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, q_nofile) {
    uint32_t expect = Q_DEFAULT;
    uint32_t actual = ~expect;

    write("q_measured", actual);
    unlink();

    createHwCal();

    EXPECT_TRUE(mHwCal->getQ(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, v_levels) {
    std::array<uint32_t, 6> expect;
    std::array<uint32_t, 6> actual;

    std::transform(expect.begin(), expect.end(), actual.begin(), [](uint32_t &e) {
        e = std::rand();
        return ~e;
    });

    write("v_levels", expect);

    createHwCal();

    EXPECT_TRUE(mHwCal->getVolLevels(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, v_missing) {
    std::array<uint32_t, 6> expect = V_DEFAULT;
    std::array<uint32_t, 6> actual;

    std::transform(expect.begin(), expect.end(), actual.begin(), [](uint32_t &e) { return ~e; });

    createHwCal();

    EXPECT_TRUE(mHwCal->getVolLevels(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, v_short) {
    std::array<uint32_t, 6> expect = V_DEFAULT;
    std::array<uint32_t, 6> actual;

    std::transform(expect.begin(), expect.end(), actual.begin(), [](uint32_t &e) { return ~e; });

    write("v_levels", std::array<uint32_t, expect.size() - 1>());

    createHwCal();

    EXPECT_TRUE(mHwCal->getVolLevels(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, v_long) {
    std::array<uint32_t, 6> expect = V_DEFAULT;
    std::array<uint32_t, 6> actual;

    std::transform(expect.begin(), expect.end(), actual.begin(), [](uint32_t &e) { return ~e; });

    write("v_levels", std::array<uint32_t, expect.size() + 1>());

    createHwCal();

    EXPECT_TRUE(mHwCal->getVolLevels(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, v_nofile) {
    std::array<uint32_t, 6> expect = V_DEFAULT;
    std::array<uint32_t, 6> actual;

    std::transform(expect.begin(), expect.end(), actual.begin(), [](uint32_t &e) { return ~e; });

    write("v_levels", actual);
    unlink();

    createHwCal();

    EXPECT_TRUE(mHwCal->getVolLevels(&actual));
    EXPECT_EQ(expect, actual);
}

TEST_F(HwCalTest, multiple) {
    uint32_t f0Expect = std::rand();
    uint32_t f0Actual = ~f0Expect;
    uint32_t redcExpect = std::rand();
    uint32_t redcActual = ~redcExpect;
    uint32_t qExpect = std::rand();
    uint32_t qActual = ~qExpect;
    std::array<uint32_t, 6> volExpect;
    std::array<uint32_t, 6> volActual;

    std::transform(volExpect.begin(), volExpect.end(), volActual.begin(), [](uint32_t &e) {
        e = std::rand();
        return ~e;
    });

    write("f0_measured", f0Expect);
    write("redc_measured", redcExpect);
    write("q_measured", qExpect);
    write("v_levels", volExpect);

    createHwCal();

    EXPECT_TRUE(mHwCal->getF0(&f0Actual));
    EXPECT_EQ(f0Expect, f0Actual);
    EXPECT_TRUE(mHwCal->getRedc(&redcActual));
    EXPECT_EQ(redcExpect, redcActual);
    EXPECT_TRUE(mHwCal->getQ(&qActual));
    EXPECT_EQ(qExpect, qActual);
    EXPECT_TRUE(mHwCal->getVolLevels(&volActual));
    EXPECT_EQ(volExpect, volActual);
}

TEST_F(HwCalTest, trimming) {
    uint32_t f0Expect = std::rand();
    uint32_t f0Actual = ~f0Expect;
    uint32_t redcExpect = std::rand();
    uint32_t redcActual = ~redcExpect;
    uint32_t qExpect = std::rand();
    uint32_t qActual = ~qExpect;
    std::array<uint32_t, 6> volExpect;
    std::array<uint32_t, 6> volActual;

    std::transform(volExpect.begin(), volExpect.end(), volActual.begin(), [](uint32_t &e) {
        e = std::rand();
        return ~e;
    });

    write("f0_measured", f0Expect, " \t", "\t ");
    write("redc_measured", redcExpect, " \t", "\t ");
    write("q_measured", qExpect, " \t", "\t ");
    write("v_levels", volExpect, " \t", "\t ");

    createHwCal();

    EXPECT_TRUE(mHwCal->getF0(&f0Actual));
    EXPECT_EQ(f0Expect, f0Actual);
    EXPECT_TRUE(mHwCal->getRedc(&redcActual));
    EXPECT_EQ(redcExpect, redcActual);
    EXPECT_TRUE(mHwCal->getQ(&qActual));
    EXPECT_EQ(qExpect, qActual);
    EXPECT_TRUE(mHwCal->getVolLevels(&volActual));
    EXPECT_EQ(volExpect, volActual);
}

}  // namespace implementation
}  // namespace V1_3
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
