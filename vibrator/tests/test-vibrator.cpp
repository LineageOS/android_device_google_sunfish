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
#define LOG_TAG "PtsVibratorHalSunfishTestSuite"

#include <android-base/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Vibrator.h"
#include "mocks.h"
#include "types.h"
#include "utils.h"

using namespace ::testing;

namespace android {
namespace hardware {
namespace vibrator {
namespace V1_3 {
namespace implementation {

using ::android::hardware::vibrator::V1_0::EffectStrength;
using ::android::hardware::vibrator::V1_0::Status;

// Forward Declarations

static EffectQueue Queue(const QueueEffect &effect);
static EffectQueue Queue(const QueueDelay &delay);
template <typename T, typename U, typename... Args>
static EffectQueue Queue(const T &first, const U &second, Args... rest);

// Constants With Arbitrary Values

static constexpr std::array<EffectLevel, 6> V_LEVELS{40, 50, 60, 70, 80, 90};
static constexpr EffectDuration EFFECT_DURATION{15};

// Constants With Prescribed Values

static constexpr EffectIndex EFFECT_INDEX{2};
static constexpr EffectIndex QUEUE_INDEX{65534};

static constexpr EffectIndex GPIO_FALL_INDEX{EFFECT_INDEX};
static const EffectScale GPIO_FALL_SCALE{levelToScale(V_LEVELS[2])};
static constexpr EffectIndex GPIO_RISE_INDEX{EFFECT_INDEX};
static const EffectScale GPIO_RISE_SCALE{levelToScale(V_LEVELS[3])};

static const EffectScale ON_GLOBAL_SCALE{levelToScale(V_LEVELS[5])};
static const EffectIndex ON_EFFECT_INDEX{0};

static const std::map<EffectTuple, EffectScale> EFFECT_SCALE{
    {{Effect::CLICK, EffectStrength::LIGHT}, levelToScale(V_LEVELS[1])},
    {{Effect::CLICK, EffectStrength::MEDIUM}, levelToScale(V_LEVELS[2])},
    {{Effect::CLICK, EffectStrength::STRONG}, levelToScale(V_LEVELS[3])},
    {{Effect::TICK, EffectStrength::LIGHT}, levelToScale(V_LEVELS[1])},
    {{Effect::TICK, EffectStrength::MEDIUM}, levelToScale(V_LEVELS[1])},
    {{Effect::TICK, EffectStrength::STRONG}, levelToScale(V_LEVELS[1])},
    {{Effect::HEAVY_CLICK, EffectStrength::LIGHT}, levelToScale(V_LEVELS[2])},
    {{Effect::HEAVY_CLICK, EffectStrength::MEDIUM}, levelToScale(V_LEVELS[3])},
    {{Effect::HEAVY_CLICK, EffectStrength::STRONG}, levelToScale(V_LEVELS[4])},
    {{Effect::TEXTURE_TICK, EffectStrength::LIGHT}, levelToScale(V_LEVELS[0])},
    {{Effect::TEXTURE_TICK, EffectStrength::MEDIUM}, levelToScale(V_LEVELS[0])},
    {{Effect::TEXTURE_TICK, EffectStrength::STRONG}, levelToScale(V_LEVELS[0])},
};

static const std::map<EffectTuple, EffectQueue> EFFECT_QUEUE{
    {{Effect::DOUBLE_CLICK, EffectStrength::LIGHT},
     Queue(QueueEffect{EFFECT_INDEX, V_LEVELS[1]}, 100, QueueEffect{EFFECT_INDEX, V_LEVELS[2]})},
    {{Effect::DOUBLE_CLICK, EffectStrength::MEDIUM},
     Queue(QueueEffect{EFFECT_INDEX, V_LEVELS[2]}, 100, QueueEffect{EFFECT_INDEX, V_LEVELS[3]})},
    {{Effect::DOUBLE_CLICK, EffectStrength::STRONG},
     Queue(QueueEffect{EFFECT_INDEX, V_LEVELS[3]}, 100, QueueEffect{EFFECT_INDEX, V_LEVELS[4]})},
};

EffectQueue Queue(const QueueEffect &effect) {
    auto index = std::get<0>(effect);
    auto level = std::get<1>(effect);
    auto string = std::to_string(index) + "." + std::to_string(level);
    auto duration = EFFECT_DURATION;
    return {string, duration};
}

EffectQueue Queue(const QueueDelay &delay) {
    auto string = std::to_string(delay);
    return {string, delay};
}

template <typename T, typename U, typename... Args>
EffectQueue Queue(const T &first, const U &second, Args... rest) {
    auto head = Queue(first);
    auto tail = Queue(second, rest...);
    auto string = std::get<0>(head) + "," + std::get<0>(tail);
    auto duration = std::get<1>(head) + std::get<1>(tail);
    return {string, duration};
}

class VibratorTest : public Test, public WithParamInterface<EffectTuple> {
  public:
    void SetUp() override {
        std::unique_ptr<MockApi> mockapi;
        std::unique_ptr<MockCal> mockcal;

        createMock(&mockapi, &mockcal);
        createVibrator(std::move(mockapi), std::move(mockcal));
    }

    void TearDown() override { deleteVibrator(); }

  protected:
    void createMock(std::unique_ptr<MockApi> *mockapi, std::unique_ptr<MockCal> *mockcal) {
        *mockapi = std::make_unique<MockApi>();
        *mockcal = std::make_unique<MockCal>();

        mMockApi = mockapi->get();
        mMockCal = mockcal->get();

        ON_CALL(*mMockApi, destructor()).WillByDefault(Assign(&mMockApi, nullptr));

        ON_CALL(*mMockApi, getEffectDuration(_))
            .WillByDefault(
                DoAll(SetArgPointee<0>(msToCycles(EFFECT_DURATION)), ::testing::Return(true)));

        ON_CALL(*mMockCal, destructor()).WillByDefault(Assign(&mMockCal, nullptr));

        ON_CALL(*mMockCal, getVolLevels(_))
            .WillByDefault(DoAll(SetArgPointee<0>(V_LEVELS), ::testing::Return(true)));

        relaxMock(false);
    }

    void createVibrator(std::unique_ptr<MockApi> mockapi, std::unique_ptr<MockCal> mockcal,
                        bool relaxed = true) {
        std::vector<uint32_t> vlevels{std::begin(V_LEVELS), std::end(V_LEVELS)};
        if (relaxed) {
            relaxMock(true);
        }
        mVibrator = new Vibrator(std::move(mockapi), std::move(mockcal));
        if (relaxed) {
            relaxMock(false);
        }
    }

    void deleteVibrator(bool relaxed = true) {
        if (relaxed) {
            relaxMock(true);
        }
        mVibrator.clear();
    }

  private:
    void relaxMock(bool relax) {
        auto times = relax ? AnyNumber() : Exactly(0);

        Mock::VerifyAndClearExpectations(mMockApi);
        Mock::VerifyAndClearExpectations(mMockCal);

        EXPECT_CALL(*mMockApi, destructor()).Times(times);
        EXPECT_CALL(*mMockApi, setF0(_)).Times(times);
        EXPECT_CALL(*mMockApi, setRedc(_)).Times(times);
        EXPECT_CALL(*mMockApi, setQ(_)).Times(times);
        EXPECT_CALL(*mMockApi, setActivate(_)).Times(times);
        EXPECT_CALL(*mMockApi, setDuration(_)).Times(times);
        EXPECT_CALL(*mMockApi, getEffectDuration(_)).Times(times);
        EXPECT_CALL(*mMockApi, setEffectIndex(_)).Times(times);
        EXPECT_CALL(*mMockApi, setEffectQueue(_)).Times(times);
        EXPECT_CALL(*mMockApi, hasEffectScale()).Times(times);
        EXPECT_CALL(*mMockApi, setEffectScale(_)).Times(times);
        EXPECT_CALL(*mMockApi, setGlobalScale(_)).Times(times);
        EXPECT_CALL(*mMockApi, setState(_)).Times(times);
        EXPECT_CALL(*mMockApi, hasAspEnable()).Times(times);
        EXPECT_CALL(*mMockApi, getAspEnable(_)).Times(times);
        EXPECT_CALL(*mMockApi, setAspEnable(_)).Times(times);
        EXPECT_CALL(*mMockApi, setGpioFallIndex(_)).Times(times);
        EXPECT_CALL(*mMockApi, setGpioFallScale(_)).Times(times);
        EXPECT_CALL(*mMockApi, setGpioRiseIndex(_)).Times(times);
        EXPECT_CALL(*mMockApi, setGpioRiseScale(_)).Times(times);
        EXPECT_CALL(*mMockApi, debug(_)).Times(times);

        EXPECT_CALL(*mMockCal, destructor()).Times(times);
        EXPECT_CALL(*mMockCal, getF0(_)).Times(times);
        EXPECT_CALL(*mMockCal, getRedc(_)).Times(times);
        EXPECT_CALL(*mMockCal, getQ(_)).Times(times);
        EXPECT_CALL(*mMockCal, getVolLevels(_)).Times(times);
        EXPECT_CALL(*mMockApi, debug(_)).Times(times);
    }

  protected:
    MockApi *mMockApi;
    MockCal *mMockCal;
    sp<IVibrator> mVibrator;
};

TEST_F(VibratorTest, HwApi) {
    std::unique_ptr<MockApi> mockapi;
    std::unique_ptr<MockCal> mockcal;
    uint32_t f0Val = std::rand();
    uint32_t redcVal = std::rand();
    uint32_t qVal = std::rand();
    Expectation volGet;
    Sequence f0Seq, redcSeq, qSeq, volSeq, durSeq;

    EXPECT_CALL(*mMockApi, destructor()).WillOnce(DoDefault());
    EXPECT_CALL(*mMockCal, destructor()).WillOnce(DoDefault());

    deleteVibrator(false);

    createMock(&mockapi, &mockcal);

    EXPECT_CALL(*mMockCal, getF0(_))
        .InSequence(f0Seq)
        .WillOnce(DoAll(SetArgPointee<0>(f0Val), ::testing::Return(true)));
    EXPECT_CALL(*mMockApi, setF0(f0Val)).InSequence(f0Seq).WillOnce(::testing::Return(true));

    EXPECT_CALL(*mMockCal, getRedc(_))
        .InSequence(redcSeq)
        .WillOnce(DoAll(SetArgPointee<0>(redcVal), ::testing::Return(true)));
    EXPECT_CALL(*mMockApi, setRedc(redcVal)).InSequence(redcSeq).WillOnce(::testing::Return(true));

    EXPECT_CALL(*mMockCal, getQ(_))
        .InSequence(qSeq)
        .WillOnce(DoAll(SetArgPointee<0>(qVal), ::testing::Return(true)));
    EXPECT_CALL(*mMockApi, setQ(qVal)).InSequence(qSeq).WillOnce(::testing::Return(true));

    volGet = EXPECT_CALL(*mMockCal, getVolLevels(_)).WillOnce(DoDefault());

    EXPECT_CALL(*mMockApi, setState(true)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, setEffectIndex(EFFECT_INDEX))
        .InSequence(durSeq)
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, getEffectDuration(_)).InSequence(durSeq).WillOnce(DoDefault());

    EXPECT_CALL(*mMockApi, setGpioFallIndex(GPIO_FALL_INDEX)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, setGpioFallScale(GPIO_FALL_SCALE))
        .After(volGet)
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, setGpioRiseIndex(GPIO_RISE_INDEX)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, setGpioRiseScale(GPIO_RISE_SCALE))
        .After(volGet)
        .WillOnce(::testing::Return(true));

    createVibrator(std::move(mockapi), std::move(mockcal), false);
}

TEST_F(VibratorTest, on) {
    Sequence s1, s2, s3;
    uint16_t duration = std::rand() + 1;

    EXPECT_CALL(*mMockApi, setGlobalScale(ON_GLOBAL_SCALE))
        .InSequence(s1)
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, setEffectIndex(ON_EFFECT_INDEX))
        .InSequence(s2)
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, setDuration(Ge(duration)))
        .InSequence(s3)
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, setActivate(true))
        .InSequence(s1, s2, s3)
        .WillOnce(::testing::Return(true));

    EXPECT_EQ(Status::OK, mVibrator->on(duration));
}

TEST_F(VibratorTest, off) {
    EXPECT_CALL(*mMockApi, setActivate(false)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, setGlobalScale(0)).WillOnce(::testing::Return(true));

    EXPECT_EQ(Status::OK, mVibrator->off());
}

TEST_F(VibratorTest, supportsAmplitudeControl_supported) {
    EXPECT_CALL(*mMockApi, hasEffectScale()).WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, getAspEnable(_))
        .WillOnce(DoAll(SetArgPointee<0>(false), ::testing::Return(true)));

    EXPECT_EQ(true, mVibrator->supportsAmplitudeControl());
}

TEST_F(VibratorTest, supportsAmplitudeControl_unsupported1) {
    MockFunction<void()> either;

    EXPECT_CALL(*mMockApi, hasEffectScale())
        .Times(AtMost(1))
        .WillOnce(DoAll(InvokeWithoutArgs(&either, &MockFunction<void()>::Call),
                        ::testing::Return(false)));
    EXPECT_CALL(*mMockApi, getAspEnable(_))
        .Times(AtMost(1))
        .WillOnce(DoAll(InvokeWithoutArgs(&either, &MockFunction<void()>::Call),
                        SetArgPointee<0>(false), ::testing::Return(true)));
    EXPECT_CALL(either, Call()).Times(AtLeast(1));

    EXPECT_EQ(false, mVibrator->supportsAmplitudeControl());
}

TEST_F(VibratorTest, supportsAmplitudeControl_unsupported2) {
    MockFunction<void()> either;

    EXPECT_CALL(*mMockApi, hasEffectScale())
        .Times(AtMost(1))
        .WillOnce(DoAll(InvokeWithoutArgs(&either, &MockFunction<void()>::Call),
                        ::testing::Return(false)));
    EXPECT_CALL(*mMockApi, getAspEnable(_))
        .Times(AtMost(1))
        .WillOnce(DoAll(InvokeWithoutArgs(&either, &MockFunction<void()>::Call),
                        SetArgPointee<0>(true), ::testing::Return(true)));
    EXPECT_CALL(either, Call()).Times(AtLeast(1));

    EXPECT_EQ(false, mVibrator->supportsAmplitudeControl());
}

TEST_F(VibratorTest, supportsAmplitudeControl_unsupported3) {
    MockFunction<void()> either;

    EXPECT_CALL(*mMockApi, hasEffectScale())
        .Times(AtMost(1))
        .WillOnce(DoAll(InvokeWithoutArgs(&either, &MockFunction<void()>::Call),
                        ::testing::Return(true)));
    EXPECT_CALL(*mMockApi, getAspEnable(_))
        .Times(AtMost(1))
        .WillOnce(DoAll(InvokeWithoutArgs(&either, &MockFunction<void()>::Call),
                        SetArgPointee<0>(true), ::testing::Return(true)));
    EXPECT_CALL(either, Call()).Times(AtLeast(1));

    EXPECT_EQ(false, mVibrator->supportsAmplitudeControl());
}

TEST_F(VibratorTest, setAmplitude_supported) {
    Sequence s;
    EffectAmplitude amplitude = std::rand() + 1;

    EXPECT_CALL(*mMockApi, getAspEnable(_))
        .InSequence(s)
        .WillOnce(DoAll(SetArgPointee<0>(false), ::testing::Return(true)));
    EXPECT_CALL(*mMockApi, setEffectScale(amplitudeToScale(amplitude)))
        .InSequence(s)
        .WillOnce(::testing::Return(true));

    EXPECT_EQ(Status::OK, mVibrator->setAmplitude(amplitude));
}

TEST_F(VibratorTest, setAmplitude_unsupported) {
    EXPECT_CALL(*mMockApi, getAspEnable(_))
        .WillOnce(DoAll(SetArgPointee<0>(true), ::testing::Return(true)));

    EXPECT_EQ(Status::UNSUPPORTED_OPERATION, mVibrator->setAmplitude(1));
}

TEST_F(VibratorTest, supportsExternalControl_supported) {
    EXPECT_CALL(*mMockApi, hasAspEnable()).WillOnce(::testing::Return(true));

    EXPECT_EQ(true, mVibrator->supportsExternalControl());
}

TEST_F(VibratorTest, supportsExternalControl_unsupported) {
    EXPECT_CALL(*mMockApi, hasAspEnable()).WillOnce(::testing::Return(false));

    EXPECT_EQ(false, mVibrator->supportsExternalControl());
}

TEST_F(VibratorTest, setExternalControl_enable) {
    Sequence s;

    EXPECT_CALL(*mMockApi, setGlobalScale(ON_GLOBAL_SCALE))
        .InSequence(s)
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, setAspEnable(true)).InSequence(s).WillOnce(::testing::Return(true));

    EXPECT_EQ(Status::OK, mVibrator->setExternalControl(true));
}

TEST_F(VibratorTest, setExternalControl_disable) {
    EXPECT_CALL(*mMockApi, setAspEnable(false)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*mMockApi, setGlobalScale(0)).WillOnce(::testing::Return(true));

    EXPECT_EQ(Status::OK, mVibrator->setExternalControl(false));
}

TEST_P(VibratorTest, perform) {
    auto param = GetParam();
    auto effect = std::get<0>(param);
    auto strength = std::get<1>(param);
    auto scale = EFFECT_SCALE.find(param);
    auto queue = EFFECT_QUEUE.find(param);
    EffectDuration duration;

    if (scale != EFFECT_SCALE.end()) {
        Sequence s1, s2, s3;

        duration = EFFECT_DURATION;

        EXPECT_CALL(*mMockApi, setEffectIndex(EFFECT_INDEX))
            .InSequence(s1)
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mMockApi, setEffectScale(scale->second))
            .InSequence(s2)
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mMockApi, setDuration(Ge(duration)))
            .InSequence(s3)
            .WillOnce(::testing::Return(true));

        EXPECT_CALL(*mMockApi, setActivate(true))
            .InSequence(s1, s2, s3)
            .WillOnce(::testing::Return(true));
    } else if (queue != EFFECT_QUEUE.end()) {
        Sequence s1, s2, s3;

        duration = std::get<1>(queue->second);

        EXPECT_CALL(*mMockApi, setEffectIndex(QUEUE_INDEX))
            .InSequence(s1)
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mMockApi, setEffectQueue(std::get<0>(queue->second)))
            .InSequence(s2)
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mMockApi, setDuration(Ge(duration)))
            .InSequence(s3)
            .WillOnce(::testing::Return(true));

        EXPECT_CALL(*mMockApi, setActivate(true))
            .InSequence(s1, s2, s3)
            .WillOnce(::testing::Return(true));
    } else {
        duration = 0;
    }

    mVibrator->perform_1_3(effect, strength, [&](Status status, uint32_t lengthMs) {
        if (duration) {
            EXPECT_EQ(Status::OK, status);
            EXPECT_LE(duration, lengthMs);
        } else {
            EXPECT_EQ(Status::UNSUPPORTED_OPERATION, status);
            EXPECT_EQ(0, lengthMs);
        }
    });
}

INSTANTIATE_TEST_CASE_P(VibratorEffects, VibratorTest,
                        Combine(ValuesIn(hidl_enum_range<Effect>().begin(),
                                         hidl_enum_range<Effect>().end()),
                                ValuesIn(hidl_enum_range<EffectStrength>().begin(),
                                         hidl_enum_range<EffectStrength>().end())),
                        [](const testing::TestParamInfo<VibratorTest::ParamType> &info) {
                            auto effect = std::get<0>(info.param);
                            auto strength = std::get<1>(info.param);
                            return toString(effect) + "_" + toString(strength);
                        });

}  // namespace implementation
}  // namespace V1_3
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
