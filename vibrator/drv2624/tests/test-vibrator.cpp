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

#include <android-base/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Vibrator.h"
#include "mocks.h"
#include "types.h"
#include "utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::AnyOf;
using ::testing::Assign;
using ::testing::Combine;
using ::testing::DoAll;
using ::testing::DoDefault;
using ::testing::Exactly;
using ::testing::ExpectationSet;
using ::testing::Mock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgPointee;
using ::testing::SetArgReferee;
using ::testing::Test;
using ::testing::TestParamInfo;
using ::testing::ValuesIn;
using ::testing::WithParamInterface;

// Constants With Prescribed Values

static const std::map<EffectTuple, EffectSequence> EFFECT_SEQUENCES{
    {{Effect::CLICK, EffectStrength::LIGHT}, {"1 0", 2}},
    {{Effect::CLICK, EffectStrength::MEDIUM}, {"1 0", 0}},
    {{Effect::CLICK, EffectStrength::STRONG}, {"1 0", 0}},
    {{Effect::TICK, EffectStrength::LIGHT}, {"2 0", 2}},
    {{Effect::TICK, EffectStrength::MEDIUM}, {"2 0", 0}},
    {{Effect::TICK, EffectStrength::STRONG}, {"2 0", 0}},
    {{Effect::DOUBLE_CLICK, EffectStrength::LIGHT}, {"3 0", 2}},
    {{Effect::DOUBLE_CLICK, EffectStrength::MEDIUM}, {"3 0", 0}},
    {{Effect::DOUBLE_CLICK, EffectStrength::STRONG}, {"3 0", 0}},
    {{Effect::HEAVY_CLICK, EffectStrength::LIGHT}, {"4 0", 2}},
    {{Effect::HEAVY_CLICK, EffectStrength::MEDIUM}, {"4 0", 0}},
    {{Effect::HEAVY_CLICK, EffectStrength::STRONG}, {"4 0", 0}},
    {{Effect::TEXTURE_TICK, EffectStrength::LIGHT}, {"2 0", 2}},
    {{Effect::TEXTURE_TICK, EffectStrength::MEDIUM}, {"2 0", 0}},
    {{Effect::TEXTURE_TICK, EffectStrength::STRONG}, {"2 0", 0}},
};

static uint32_t freqPeriodFormula(uint32_t in) {
    return 1000000000 / (24615 * in);
}

template <typename... T>
class VibratorTestTemplate : public Test, public WithParamInterface<std::tuple<bool, T...>> {
  public:
    static auto GetDynamicConfig(typename VibratorTestTemplate::ParamType param) {
        return std::get<0>(param);
    }
    template <std::size_t I>
    static auto GetOtherParam(typename VibratorTestTemplate::ParamType param) {
        return std::get<I + 1>(param);
    }

    static auto PrintParam(const TestParamInfo<typename VibratorTestTemplate::ParamType> &info) {
        auto dynamic = GetDynamicConfig(info.param);
        return std::string() + (dynamic ? "Dynamic" : "Static") + "Config";
    }

    static auto MakeParam(bool dynamicConfig, T... others) {
        return std::make_tuple(dynamicConfig, others...);
    }

    void SetUp() override {
        std::unique_ptr<MockApi> mockapi;
        std::unique_ptr<MockCal> mockcal;

        mCloseLoopThreshold = std::rand();
        // ensure close-loop test is possible
        if (mCloseLoopThreshold == UINT32_MAX) {
            mCloseLoopThreshold--;
        }

        mShortLraPeriod = std::rand();
        if (getDynamicConfig()) {
            mLongFrequencyShift = std::rand();
            mLongLraPeriod =
                freqPeriodFormula(freqPeriodFormula(mShortLraPeriod) - mLongFrequencyShift);
            mShortVoltageMax = std::rand();
            mLongVoltageMax = std::rand();
        }

        mEffectDurations[Effect::CLICK] = std::rand();
        mEffectDurations[Effect::TICK] = std::rand();
        mEffectDurations[Effect::DOUBLE_CLICK] = std::rand();
        mEffectDurations[Effect::HEAVY_CLICK] = std::rand();
        mEffectDurations[Effect::TEXTURE_TICK] = mEffectDurations[Effect::TICK];

        createMock(&mockapi, &mockcal);
        createVibrator(std::move(mockapi), std::move(mockcal));
    }

    void TearDown() override { deleteVibrator(); }

  protected:
    auto getDynamicConfig() const { return GetDynamicConfig(VibratorTestTemplate::GetParam()); }

    void createMock(std::unique_ptr<MockApi> *mockapi, std::unique_ptr<MockCal> *mockcal) {
        *mockapi = std::make_unique<MockApi>();
        *mockcal = std::make_unique<MockCal>();

        mMockApi = mockapi->get();
        mMockCal = mockcal->get();

        ON_CALL(*mMockApi, destructor()).WillByDefault(Assign(&mMockApi, nullptr));
        ON_CALL(*mMockApi, setOlLraPeriod(_)).WillByDefault(Return(true));
        ON_CALL(*mMockApi, setActivate(_)).WillByDefault(Return(true));
        ON_CALL(*mMockApi, setDuration(_)).WillByDefault(Return(true));
        ON_CALL(*mMockApi, setMode(_)).WillByDefault(Return(true));
        ON_CALL(*mMockApi, setCtrlLoop(_)).WillByDefault(Return(true));
        ON_CALL(*mMockApi, setLraWaveShape(_)).WillByDefault(Return(true));
        ON_CALL(*mMockApi, setOdClamp(_)).WillByDefault(Return(true));

        ON_CALL(*mMockCal, destructor()).WillByDefault(Assign(&mMockCal, nullptr));
        ON_CALL(*mMockCal, getLraPeriod(_))
            .WillByDefault(DoAll(SetArgPointee<0>(mShortLraPeriod), Return(true)));
        ON_CALL(*mMockCal, getCloseLoopThreshold(_))
            .WillByDefault(DoAll(SetArgPointee<0>(mCloseLoopThreshold), Return(true)));
        ON_CALL(*mMockCal, getDynamicConfig(_))
            .WillByDefault(DoAll(SetArgPointee<0>(getDynamicConfig()), Return(true)));

        if (getDynamicConfig()) {
            ON_CALL(*mMockCal, getLongFrequencyShift(_))
                .WillByDefault(DoAll(SetArgPointee<0>(mLongFrequencyShift), Return(true)));
            ON_CALL(*mMockCal, getShortVoltageMax(_))
                .WillByDefault(DoAll(SetArgPointee<0>(mShortVoltageMax), Return(true)));
            ON_CALL(*mMockCal, getLongVoltageMax(_))
                .WillByDefault(DoAll(SetArgPointee<0>(mLongVoltageMax), Return(true)));
        }

        ON_CALL(*mMockCal, getClickDuration(_))
            .WillByDefault(DoAll(SetArgPointee<0>(mEffectDurations[Effect::CLICK]), Return(true)));
        ON_CALL(*mMockCal, getTickDuration(_))
            .WillByDefault(DoAll(SetArgPointee<0>(mEffectDurations[Effect::TICK]), Return(true)));
        ON_CALL(*mMockCal, getDoubleClickDuration(_))
            .WillByDefault(
                DoAll(SetArgPointee<0>(mEffectDurations[Effect::DOUBLE_CLICK]), Return(true)));
        ON_CALL(*mMockCal, getHeavyClickDuration(_))
            .WillByDefault(
                DoAll(SetArgPointee<0>(mEffectDurations[Effect::HEAVY_CLICK]), Return(true)));

        relaxMock(false);
    }

    void createVibrator(std::unique_ptr<MockApi> mockapi, std::unique_ptr<MockCal> mockcal,
                        bool relaxed = true) {
        if (relaxed) {
            relaxMock(true);
        }
        mVibrator = ndk::SharedRefBase::make<Vibrator>(std::move(mockapi), std::move(mockcal));
        if (relaxed) {
            relaxMock(false);
        }
    }

    void deleteVibrator(bool relaxed = true) {
        if (relaxed) {
            relaxMock(true);
        }
        mVibrator.reset();
    }

    void relaxMock(bool relax) {
        auto times = relax ? AnyNumber() : Exactly(0);

        Mock::VerifyAndClearExpectations(mMockApi);
        Mock::VerifyAndClearExpectations(mMockCal);

        EXPECT_CALL(*mMockApi, destructor()).Times(times);
        EXPECT_CALL(*mMockApi, setAutocal(_)).Times(times);
        EXPECT_CALL(*mMockApi, setOlLraPeriod(_)).Times(times);
        EXPECT_CALL(*mMockApi, setActivate(_)).Times(times);
        EXPECT_CALL(*mMockApi, setDuration(_)).Times(times);
        EXPECT_CALL(*mMockApi, setState(_)).Times(times);
        EXPECT_CALL(*mMockApi, hasRtpInput()).Times(times);
        EXPECT_CALL(*mMockApi, setRtpInput(_)).Times(times);
        EXPECT_CALL(*mMockApi, setMode(_)).Times(times);
        EXPECT_CALL(*mMockApi, setSequencer(_)).Times(times);
        EXPECT_CALL(*mMockApi, setScale(_)).Times(times);
        EXPECT_CALL(*mMockApi, setCtrlLoop(_)).Times(times);
        EXPECT_CALL(*mMockApi, setLpTriggerEffect(_)).Times(times);
        EXPECT_CALL(*mMockApi, setLraWaveShape(_)).Times(times);
        EXPECT_CALL(*mMockApi, setOdClamp(_)).Times(times);
        EXPECT_CALL(*mMockApi, debug(_)).Times(times);

        EXPECT_CALL(*mMockCal, destructor()).Times(times);
        EXPECT_CALL(*mMockCal, getAutocal(_)).Times(times);
        EXPECT_CALL(*mMockCal, getLraPeriod(_)).Times(times);
        EXPECT_CALL(*mMockCal, getCloseLoopThreshold(_)).Times(times);
        EXPECT_CALL(*mMockCal, getDynamicConfig(_)).Times(times);
        EXPECT_CALL(*mMockCal, getLongFrequencyShift(_)).Times(times);
        EXPECT_CALL(*mMockCal, getShortVoltageMax(_)).Times(times);
        EXPECT_CALL(*mMockCal, getLongVoltageMax(_)).Times(times);
        EXPECT_CALL(*mMockCal, getClickDuration(_)).Times(times);
        EXPECT_CALL(*mMockCal, getTickDuration(_)).Times(times);
        EXPECT_CALL(*mMockCal, getDoubleClickDuration(_)).Times(times);
        EXPECT_CALL(*mMockCal, getHeavyClickDuration(_)).Times(times);
        EXPECT_CALL(*mMockCal, debug(_)).Times(times);
    }

  protected:
    MockApi *mMockApi;
    MockCal *mMockCal;
    std::shared_ptr<IVibrator> mVibrator;

    EffectDuration mCloseLoopThreshold;
    uint32_t mLongFrequencyShift;
    uint32_t mShortLraPeriod;
    uint32_t mLongLraPeriod;
    uint32_t mShortVoltageMax;
    uint32_t mLongVoltageMax;
    std::map<Effect, EffectDuration> mEffectDurations;
};

using BasicTest = VibratorTestTemplate<>;

TEST_P(BasicTest, Constructor) {
    std::unique_ptr<MockApi> mockapi;
    std::unique_ptr<MockCal> mockcal;
    std::string autocalVal = std::to_string(std::rand()) + " " + std::to_string(std::rand()) +
                             " " + std::to_string(std::rand());
    Sequence autocalSeq, lraPeriodSeq;

    EXPECT_CALL(*mMockApi, destructor()).WillOnce(DoDefault());
    EXPECT_CALL(*mMockCal, destructor()).WillOnce(DoDefault());

    deleteVibrator(false);

    createMock(&mockapi, &mockcal);

    EXPECT_CALL(*mMockApi, setState(true)).WillOnce(Return(true));

    EXPECT_CALL(*mMockCal, getAutocal(_))
        .InSequence(autocalSeq)
        .WillOnce(DoAll(SetArgReferee<0>(autocalVal), Return(true)));
    EXPECT_CALL(*mMockApi, setAutocal(autocalVal)).InSequence(autocalSeq).WillOnce(DoDefault());

    EXPECT_CALL(*mMockCal, getLraPeriod(_)).InSequence(lraPeriodSeq).WillOnce(DoDefault());

    EXPECT_CALL(*mMockCal, getCloseLoopThreshold(_)).WillOnce(DoDefault());
    EXPECT_CALL(*mMockCal, getDynamicConfig(_)).WillOnce(DoDefault());

    if (getDynamicConfig()) {
        EXPECT_CALL(*mMockCal, getLongFrequencyShift(_)).WillOnce(DoDefault());
        EXPECT_CALL(*mMockCal, getShortVoltageMax(_)).WillOnce(DoDefault());
        EXPECT_CALL(*mMockCal, getLongVoltageMax(_)).WillOnce(DoDefault());
    } else {
        EXPECT_CALL(*mMockApi, setOlLraPeriod(mShortLraPeriod))
            .InSequence(lraPeriodSeq)
            .WillOnce(DoDefault());
    }

    EXPECT_CALL(*mMockCal, getClickDuration(_)).WillOnce(DoDefault());
    EXPECT_CALL(*mMockCal, getTickDuration(_)).WillOnce(DoDefault());
    EXPECT_CALL(*mMockCal, getDoubleClickDuration(_)).WillOnce(DoDefault());
    EXPECT_CALL(*mMockCal, getHeavyClickDuration(_)).WillOnce(DoDefault());

    EXPECT_CALL(*mMockApi, setLpTriggerEffect(1)).WillOnce(Return(true));

    createVibrator(std::move(mockapi), std::move(mockcal), false);
}

TEST_P(BasicTest, on) {
    EffectDuration duration = std::rand();
    ExpectationSet e;

    e += EXPECT_CALL(*mMockApi, setCtrlLoop(_)).WillOnce(DoDefault());
    e += EXPECT_CALL(*mMockApi, setMode("rtp")).WillOnce(DoDefault());
    e += EXPECT_CALL(*mMockApi, setDuration(duration)).WillOnce(DoDefault());

    if (getDynamicConfig()) {
        e += EXPECT_CALL(*mMockApi, setLraWaveShape(0)).WillOnce(DoDefault());
        e += EXPECT_CALL(*mMockApi, setOdClamp(mLongVoltageMax)).WillOnce(DoDefault());
        e += EXPECT_CALL(*mMockApi, setOlLraPeriod(mLongLraPeriod)).WillOnce(DoDefault());
    }

    EXPECT_CALL(*mMockApi, setActivate(true)).After(e).WillOnce(DoDefault());

    EXPECT_EQ(EX_NONE, mVibrator->on(duration, nullptr).getExceptionCode());
}

TEST_P(BasicTest, on_openLoop) {
    EffectDuration duration = mCloseLoopThreshold;

    relaxMock(true);

    EXPECT_CALL(*mMockApi, setCtrlLoop(true)).WillOnce(DoDefault());

    EXPECT_EQ(EX_NONE, mVibrator->on(duration, nullptr).getExceptionCode());
}

TEST_P(BasicTest, on_closeLoop) {
    EffectDuration duration = mCloseLoopThreshold + 1;

    relaxMock(true);

    EXPECT_CALL(*mMockApi, setCtrlLoop(false)).WillOnce(DoDefault());

    EXPECT_EQ(EX_NONE, mVibrator->on(duration, nullptr).getExceptionCode());
}

TEST_P(BasicTest, off) {
    EXPECT_CALL(*mMockApi, setActivate(false)).WillOnce(DoDefault());

    EXPECT_EQ(EX_NONE, mVibrator->off().getExceptionCode());
}

TEST_P(BasicTest, supportsAmplitudeControl_supported) {
    EXPECT_CALL(*mMockApi, hasRtpInput()).WillOnce(Return(true));

    int32_t capabilities;
    EXPECT_TRUE(mVibrator->getCapabilities(&capabilities).isOk());
    EXPECT_GT(capabilities & IVibrator::CAP_AMPLITUDE_CONTROL, 0);
}

TEST_P(BasicTest, supportsAmplitudeControl_unsupported) {
    EXPECT_CALL(*mMockApi, hasRtpInput()).WillOnce(Return(false));

    int32_t capabilities;
    EXPECT_TRUE(mVibrator->getCapabilities(&capabilities).isOk());
    EXPECT_EQ(capabilities & IVibrator::CAP_AMPLITUDE_CONTROL, 0);
}

TEST_P(BasicTest, setAmplitude) {
    EffectAmplitude amplitude = static_cast<float>(std::rand()) / RAND_MAX ?: 1.0f;

    EXPECT_CALL(*mMockApi, setRtpInput(amplitudeToRtpInput(amplitude))).WillOnce(Return(true));

    EXPECT_EQ(EX_NONE, mVibrator->setAmplitude(amplitude).getExceptionCode());
}

TEST_P(BasicTest, supportsExternalControl_unsupported) {
    EXPECT_CALL(*mMockApi, hasRtpInput()).WillOnce(Return(false));

    int32_t capabilities;
    EXPECT_TRUE(mVibrator->getCapabilities(&capabilities).isOk());
    EXPECT_EQ(capabilities & IVibrator::CAP_EXTERNAL_CONTROL, 0);
}

TEST_P(BasicTest, setExternalControl_enable) {
    EXPECT_EQ(EX_UNSUPPORTED_OPERATION, mVibrator->setExternalControl(true).getExceptionCode());
}

TEST_P(BasicTest, setExternalControl_disable) {
    EXPECT_EQ(EX_UNSUPPORTED_OPERATION, mVibrator->setExternalControl(false).getExceptionCode());
}

INSTANTIATE_TEST_CASE_P(VibratorTests, BasicTest,
                        ValuesIn({BasicTest::MakeParam(false), BasicTest::MakeParam(true)}),
                        BasicTest::PrintParam);

class EffectsTest : public VibratorTestTemplate<EffectTuple> {
  public:
    static auto GetEffectTuple(ParamType param) { return GetOtherParam<0>(param); }

    static auto PrintParam(const TestParamInfo<ParamType> &info) {
        auto prefix = VibratorTestTemplate::PrintParam(info);
        auto tuple = GetEffectTuple(info.param);
        auto effect = std::get<0>(tuple);
        auto strength = std::get<1>(tuple);
        return prefix + "_" + toString(effect) + "_" + toString(strength);
    }

  protected:
    auto getEffectTuple() const { return GetEffectTuple(GetParam()); }
};

TEST_P(EffectsTest, perform) {
    auto tuple = getEffectTuple();
    auto effect = std::get<0>(tuple);
    auto strength = std::get<1>(tuple);
    auto seqIter = EFFECT_SEQUENCES.find(tuple);
    auto durIter = mEffectDurations.find(effect);
    EffectDuration duration;

    if (seqIter != EFFECT_SEQUENCES.end() && durIter != mEffectDurations.end()) {
        auto sequence = std::get<0>(seqIter->second);
        auto scale = std::get<1>(seqIter->second);
        ExpectationSet e;

        duration = durIter->second;

        e += EXPECT_CALL(*mMockApi, setSequencer(sequence)).WillOnce(Return(true));
        e += EXPECT_CALL(*mMockApi, setScale(scale)).WillOnce(Return(true));
        e += EXPECT_CALL(*mMockApi, setCtrlLoop(1)).WillOnce(DoDefault());
        e += EXPECT_CALL(*mMockApi, setMode("waveform")).WillOnce(DoDefault());
        e += EXPECT_CALL(*mMockApi, setDuration(duration)).WillOnce(DoDefault());

        if (getDynamicConfig()) {
            e += EXPECT_CALL(*mMockApi, setLraWaveShape(1)).WillOnce(DoDefault());
            e += EXPECT_CALL(*mMockApi, setOdClamp(mShortVoltageMax)).WillOnce(DoDefault());
            e += EXPECT_CALL(*mMockApi, setOlLraPeriod(mShortLraPeriod)).WillOnce(DoDefault());
        }

        EXPECT_CALL(*mMockApi, setActivate(true)).After(e).WillOnce(DoDefault());
    } else {
        duration = 0;
    }

    int32_t lengthMs;
    ndk::ScopedAStatus status = mVibrator->perform(effect, strength, nullptr, &lengthMs);
    if (duration) {
        EXPECT_EQ(EX_NONE, status.getExceptionCode());
        EXPECT_LE(duration, lengthMs);
    } else {
        EXPECT_EQ(EX_UNSUPPORTED_OPERATION, status.getExceptionCode());
    }
}

INSTANTIATE_TEST_CASE_P(VibratorTests, EffectsTest,
                        Combine(ValuesIn({false, true}),
                                Combine(ValuesIn(ndk::enum_range<Effect>().begin(),
                                                 ndk::enum_range<Effect>().end()),
                                        ValuesIn(ndk::enum_range<EffectStrength>().begin(),
                                                 ndk::enum_range<EffectStrength>().end()))),
                        EffectsTest::PrintParam);

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
