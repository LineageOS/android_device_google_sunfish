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

#include <stdint.h>
#include <string.h>

#include <string>
#include <vector>

#include <android-base/endian.h>
#include <android-base/logging.h>
#include <android-base/strings.h>
#include <app_nugget.h>
#include <bootloader_message/bootloader_message.h>
#include <nos/NuggetClient.h>
#include <nos/debug.h>
#include <recovery_ui/device.h>
#include <recovery_ui/screen_ui.h>

namespace android {
namespace device {
namespace google {
namespace sunfish {

namespace {

/** Wipe user data from Titan M. */
bool WipeTitanM() {
    // Connect to Titan M
    ::nos::NuggetClient client;
    client.Open();
    if (!client.IsOpen()) {
        LOG(ERROR) << "Failed to connect to Titan M";
        return false;
    }

    // Tell it to wipe user data
    const uint32_t magicValue = htole32(ERASE_CONFIRMATION);
    std::vector<uint8_t> magic(sizeof(magicValue));
    memcpy(magic.data(), &magicValue, sizeof(magicValue));
    const uint32_t status
            = client.CallApp(APP_ID_NUGGET, NUGGET_PARAM_NUKE_FROM_ORBIT, magic, nullptr);
    if (status != APP_SUCCESS) {
        LOG(ERROR) << "Titan M user data wipe failed: " << ::nos::StatusCodeString(status)
                   << " (" << status << ")";
        return false;
    }

    LOG(INFO) << "Titan M wipe successful";
    return true;
}

// Wipes the boot theme flag as part of data wipe.
bool WipeBootThemeFlag() {
    // Must be consistent with the one in init.hardware.rc (10-byte `theme-dark`).
    const std::string wipe_str(10, '\x00');
    constexpr size_t kThemeFlagOffsetInVendorSpace = 0;
    if (std::string err; !WriteMiscPartitionVendorSpace(wipe_str.data(), wipe_str.size(),
                                                        kThemeFlagOffsetInVendorSpace, &err)) {
        LOG(ERROR) << "Failed to write wipe string: " << err;
        return false;
    }
    LOG(INFO) << "Theme flag wiped successfully";
    return true;
}

bool GetReason(std::string* reason) {
    bootloader_message boot = {};
    std::string err;
    bool ret = false;
    if (!read_bootloader_message(&boot, &err)) {
        LOG(ERROR) << err;
        return ret;
    }

    std::vector<std::string> args;
    boot.recovery[sizeof(boot.recovery) - 1] = '\0';  // Ensure termination
    std::string boot_recovery(boot.recovery);
    std::vector<std::string> tokens = android::base::Split(boot_recovery, "\n");
    if (!tokens.empty() && tokens[0] == "recovery") {
        for (auto it = tokens.begin() + 1; it != tokens.end(); it++) {
            // Skip empty and '\0'-filled tokens.
            if (!it->empty() && (*it)[0] != '\0') args.push_back(std::move(*it));
        }
        LOG(INFO) << "Got " << args.size() << " arguments from boot message";
    } else if (boot.recovery[0] != 0) {
        LOG(ERROR) << "Bad boot message: \"" << boot_recovery << "\"";
        return ret;
    }

    for (const auto& arg : args) {
        if (android::base::StartsWith(arg, "--reason=")) {
            *reason = arg.substr(strlen("--reason="));
            LOG(INFO) << "reason is " << *reason;
            ret = true;
            break;
        }
    }
    return ret;
}

bool ProvisionSilentOtaFlag() {
    const std::string sota_str("enable-sota");
    constexpr size_t kSotaFlagOffsetInVendorSpace = 32;
    if (std::string err; !WriteMiscPartitionVendorSpace(sota_str.data(), sota_str.size(),
                                                        kSotaFlagOffsetInVendorSpace, &err)) {
        LOG(ERROR) << "Failed to write SOTA string: " << err;
        return false;
    }
    LOG(INFO) << "Provision SOTA flag successfully";
    return true;
}
} // namespace

class SunfishDevice : public ::Device
{
public:
    SunfishDevice(::ScreenRecoveryUI* const ui) : ::Device(ui) {}

    /** Hook to wipe user data not stored in /data */
    bool PostWipeData() override {
        // Try to do everything but report a failure if anything wasn't successful
        bool totalSuccess = true;
        ::RecoveryUI* const ui = GetUI();

        ui->Print("Wiping Titan M...\n");
        if (!WipeTitanM()) {
            totalSuccess = false;
        }

        if (!WipeBootThemeFlag()) {
            totalSuccess = false;
        }

        // Extendable to wipe other components

        // Additional behavior along with wiping data...
        std::string reason;
        if (GetReason(&reason) && android::base::StartsWith(reason, "enable-sota")) {
            ui->Print("Enabling Silent OTA...\n");
            if (!ProvisionSilentOtaFlag()) {
                totalSuccess = false;
            }
        }

        return totalSuccess;
    }
};

} // namespace sunfish
} // namespace google
} // namespace device
} // namespace android

Device *make_device()
{
    return new ::android::device::google::sunfish::SunfishDevice(new ::ScreenRecoveryUI);
}
