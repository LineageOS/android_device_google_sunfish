/* * Copyright (C) 2019 The Android Open Source Project *
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

#include "benchmark/benchmark.h"

#include <android-base/file.h>
#include <android-base/properties.h>
#include <cutils/fs.h>

#include "Hardware.h"
#include "Vibrator.h"

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

using ::android::base::SetProperty;

class VibratorBench : public benchmark::Fixture {
  private:
    static constexpr const char *FILE_NAMES[]{
        "device/autocal",
        "device/ol_lra_period",
        "activate",
        "duration",
        "state",
        "device/rtp_input",
        "device/mode",
        "device/set_sequencer",
        "device/scale",
        "device/ctrl_loop",
        "device/lp_trigger_effect",
        "device/lra_wave_shape",
        "device/od_clamp",
    };
    static constexpr char PROPERTY_PREFIX[] = "test.vibrator.hal.";

  public:
    void SetUp(::benchmark::State &state) override {
        auto prefix = std::filesystem::path(mFilesDir.path) / "";

        setenv("HWAPI_PATH_PREFIX", prefix.c_str(), true);

        for (auto n : FILE_NAMES) {
            const auto name = std::filesystem::path(n);
            const auto path = std::filesystem::path(mFilesDir.path) / name;

            fs_mkdirs(path.c_str(), S_IRWXU);
            symlink("/dev/null", path.c_str());
        }

        setenv("PROPERTY_PREFIX", PROPERTY_PREFIX, true);

        SetProperty(std::string() + PROPERTY_PREFIX + "config.dynamic", getDynamicConfig(state));

        mVibrator = ndk::SharedRefBase::make<Vibrator>(HwApi::Create(), std::make_unique<HwCal>());
    }

    static void DefaultConfig(benchmark::internal::Benchmark *b) {
        b->Unit(benchmark::kMicrosecond);
    }

    static void DefaultArgs(benchmark::internal::Benchmark *b) {
        b->ArgNames({"DynamicConfig"});
        b->Args({false});
        b->Args({true});
    }

  protected:
    std::string getDynamicConfig(const ::benchmark::State &state) const {
        return std::to_string(state.range(0));
    }

    auto getOtherArg(const ::benchmark::State &state, std::size_t index) const {
        return state.range(index + 1);
    }

  protected:
    TemporaryDir mFilesDir;
    std::shared_ptr<IVibrator> mVibrator;
};

#define BENCHMARK_WRAPPER(fixt, test, code)                           \
    BENCHMARK_DEFINE_F(fixt, test)                                    \
    /* NOLINTNEXTLINE */                                              \
    (benchmark::State & state){code} BENCHMARK_REGISTER_F(fixt, test) \
        ->Apply(fixt::DefaultConfig)                                  \
        ->Apply(fixt::DefaultArgs)

BENCHMARK_WRAPPER(VibratorBench, on, {
    uint32_t duration = std::rand() ?: 1;

    for (auto _ : state) {
        mVibrator->on(duration, nullptr);
    }
});

BENCHMARK_WRAPPER(VibratorBench, off, {
    for (auto _ : state) {
        mVibrator->off();
    }
});

BENCHMARK_WRAPPER(VibratorBench, setAmplitude, {
    uint8_t amplitude = std::rand() ?: 1;

    for (auto _ : state) {
        mVibrator->setAmplitude(amplitude);
    }
});

BENCHMARK_WRAPPER(VibratorBench, setExternalControl_enable, {
    for (auto _ : state) {
        mVibrator->setExternalControl(true);
    }
});

BENCHMARK_WRAPPER(VibratorBench, setExternalControl_disable, {
    for (auto _ : state) {
        mVibrator->setExternalControl(false);
    }
});

BENCHMARK_WRAPPER(VibratorBench, getCapabilities, {
    int32_t capabilities;

    for (auto _ : state) {
        mVibrator->getCapabilities(&capabilities);
    }
});

class VibratorEffectsBench : public VibratorBench {
  public:
    static void DefaultArgs(benchmark::internal::Benchmark *b) {
        b->ArgNames({"DynamicConfig", "Effect", "Strength"});
        for (const auto &dynamic : {false, true}) {
            for (const auto &effect : ndk::enum_range<Effect>()) {
                for (const auto &strength : ndk::enum_range<EffectStrength>()) {
                    b->Args({dynamic, static_cast<long>(effect), static_cast<long>(strength)});
                }
            }
        }
    }

  protected:
    auto getEffect(const ::benchmark::State &state) const {
        return static_cast<Effect>(getOtherArg(state, 0));
    }

    auto getStrength(const ::benchmark::State &state) const {
        return static_cast<EffectStrength>(getOtherArg(state, 1));
    }
};

BENCHMARK_WRAPPER(VibratorEffectsBench, perform, {
    Effect effect = getEffect(state);
    EffectStrength strength = getStrength(state);
    int32_t lengthMs;

    ndk::ScopedAStatus status = mVibrator->perform(effect, strength, nullptr, &lengthMs);

    if (status.getExceptionCode() == EX_UNSUPPORTED_OPERATION) {
        return;
    }

    for (auto _ : state) {
        mVibrator->perform(effect, strength, nullptr, &lengthMs);
    }
});

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl

BENCHMARK_MAIN();
