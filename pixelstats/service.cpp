/*
 * Copyright (C) 2018 The Android Open Source Project
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

#define LOG_TAG "pixelstats"

#include <android-base/logging.h>
#include <utils/StrongPointer.h>

#include <pixelstats/DropDetect.h>
#include <pixelstats/SysfsCollector.h>
#include <pixelstats/UeventListener.h>

using android::sp;
using android::hardware::google::pixel::DropDetect;
using android::hardware::google::pixel::SysfsCollector;
using android::hardware::google::pixel::UeventListener;

#define UFSHC_PATH(filename) "/sys/devices/platform/soc/1d84000.ufshc/" #filename
const struct SysfsCollector::SysfsPaths sysfs_paths = {
    .SlowioReadCntPath = UFSHC_PATH(slowio_read_cnt),
    .SlowioWriteCntPath = UFSHC_PATH(slowio_write_cnt),
    .SlowioUnmapCntPath = UFSHC_PATH(slowio_unmap_cnt),
    .SlowioSyncCntPath = UFSHC_PATH(slowio_sync_cnt),
    .CycleCountBinsPath = "/sys/class/power_supply/battery/cycle_counts",
    .ImpedancePath = "/sys/devices/platform/codec_detect/resistance_left_right",
    .CodecPath =     "/sys/devices/platform/codec_detect/codec_state",
    .SpeechDspPath = "/sys/devices/platform/codec_detect/wdsp_stat",
    .Codec1Path = "/sys/devices/platform/codec_detect/headset_codec_state",
    .UFSLifetimeA = UFSHC_PATH(health/lifetimeA),
    .UFSLifetimeB = UFSHC_PATH(health/lifetimeB),
    .UFSLifetimeC = UFSHC_PATH(health/lifetimeC),
    .F2fsStatsPath = "/sys/fs/f2fs/",
    .EEPROMPath = "/dev/battery_history",
};

const char *const kAudioUevent = "/kernel/q6audio/q6voice_uevent";
const char *const kSSOCDetailsPath = "/sys/class/power_supply/battery/ssoc_details";

int main() {
    LOG(INFO) << "starting PixelStats";

    // b/118713028 Expect failure until drop detect nanoapp is enabled
    sp<DropDetect> dropDetector = DropDetect::start();
    if (!dropDetector) {
        LOG(ERROR) << "Unable to launch drop detection";
        return 1;
    }

    UeventListener ueventListener(kAudioUevent, kSSOCDetailsPath);
    std::thread listenThread(&UeventListener::ListenForever, &ueventListener);
    listenThread.detach();

    SysfsCollector collector(sysfs_paths);
    collector.collect();  // This blocks forever.

    return 0;
}
