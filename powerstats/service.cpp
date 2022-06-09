/*
 * Copyright (C) 2020 The Android Open Source Project
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

#define LOG_TAG "android.hardware.power.stats@1.0-service.pixel"

#include <android/log.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>

#include <pixelpowerstats/AidlStateResidencyDataProvider.h>
#include <pixelpowerstats/GenericStateResidencyDataProvider.h>
#include <pixelpowerstats/PowerStats.h>
#include <pixelpowerstats/WlanStateResidencyDataProvider.h>
#include <pixelpowerstats/DisplayStateResidencyDataProvider.h>

#include "RailDataProvider.h"

using android::OK;
using android::sp;
using android::status_t;

// libhwbinder:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
using android::hardware::power::stats::V1_0::IPowerStats;
using android::hardware::power::stats::V1_0::PowerEntityInfo;
using android::hardware::power::stats::V1_0::PowerEntityStateSpace;
using android::hardware::power::stats::V1_0::PowerEntityType;
using android::hardware::power::stats::V1_0::implementation::PowerStats;

// Pixel specific
using android::hardware::google::pixel::powerstats::AidlStateResidencyDataProvider;
using android::hardware::google::pixel::powerstats::generateGenericStateResidencyConfigs;
using android::hardware::google::pixel::powerstats::GenericStateResidencyDataProvider;
using android::hardware::google::pixel::powerstats::PowerEntityConfig;
using android::hardware::google::pixel::powerstats::StateResidencyConfig;
using android::hardware::google::pixel::powerstats::RailDataProvider;
using android::hardware::google::pixel::powerstats::WlanStateResidencyDataProvider;
using android::hardware::google::pixel::powerstats::DisplayStateResidencyDataProvider;

int main(int /* argc */, char ** /* argv */) {
    ALOGE("power.stats service 1.0 is starting.");


    PowerStats *service = new PowerStats();

    // Add rail data provider
    service->setRailDataProvider(std::make_unique<RailDataProvider>());

    // Add power entities related to rpmh
    const uint64_t RPM_CLK = 19200;  // RPM runs at 19.2Mhz. Divide by 19200 for msec
    std::function<uint64_t(uint64_t)> rpmConvertToMs = [](uint64_t a) { return a / RPM_CLK; };
    std::vector<StateResidencyConfig> rpmStateResidencyConfigs = {
        {.name = "Sleep",
         .entryCountSupported = true,
         .entryCountPrefix = "Sleep Count:",
         .totalTimeSupported = true,
         .totalTimePrefix = "Sleep Accumulated Duration:",
         .totalTimeTransform = rpmConvertToMs,
         .lastEntrySupported = true,
         .lastEntryPrefix = "Sleep Last Entered At:",
         .lastEntryTransform = rpmConvertToMs}};

    sp<GenericStateResidencyDataProvider> rpmSdp =
        new GenericStateResidencyDataProvider("/sys/power/rpmh_stats/master_stats");

    uint32_t apssId = service->addPowerEntity("APSS", PowerEntityType::SUBSYSTEM);
    rpmSdp->addEntity(apssId, PowerEntityConfig("APSS", rpmStateResidencyConfigs));

    uint32_t mpssId = service->addPowerEntity("MPSS", PowerEntityType::SUBSYSTEM);
    rpmSdp->addEntity(mpssId, PowerEntityConfig("MPSS", rpmStateResidencyConfigs));

    uint32_t adspId = service->addPowerEntity("ADSP", PowerEntityType::SUBSYSTEM);
    rpmSdp->addEntity(adspId, PowerEntityConfig("ADSP", rpmStateResidencyConfigs));

    uint32_t cdspId = service->addPowerEntity("CDSP", PowerEntityType::SUBSYSTEM);
    rpmSdp->addEntity(cdspId, PowerEntityConfig("CDSP", rpmStateResidencyConfigs));

    service->addStateResidencyDataProvider(rpmSdp);

    // Add SoC power entity
    StateResidencyConfig socStateConfig = {
        .entryCountSupported = true,
        .entryCountPrefix = "count:",
        .totalTimeSupported = true,
        .totalTimePrefix = "actual last sleep(msec):",
        .lastEntrySupported = false
    };
    std::vector<std::pair<std::string, std::string>> socStateHeaders = {
        std::make_pair("AOSD", "RPM Mode:aosd"),
        std::make_pair("CXSD", "RPM Mode:cxsd"),
        std::make_pair("DDR", "RPM Mode:ddr"),
    };

    auto socSdp = sp<GenericStateResidencyDataProvider>::make("/sys/power/system_sleep/stats");

    uint32_t socId = service->addPowerEntity("SoC", PowerEntityType::POWER_DOMAIN);
    socSdp->addEntity(socId,
        PowerEntityConfig(generateGenericStateResidencyConfigs(socStateConfig, socStateHeaders)));

    service->addStateResidencyDataProvider(socSdp);

    // Add WLAN power entity
    uint32_t wlanId = service->addPowerEntity("WLAN", PowerEntityType::SUBSYSTEM);
    auto wlanSdp = sp<WlanStateResidencyDataProvider>::make(wlanId, "/sys/kernel/wlan/power_stats");
    service->addStateResidencyDataProvider(wlanSdp);

    uint32_t displayId = service->addPowerEntity("Display", PowerEntityType::SUBSYSTEM);
    auto displaySdp = sp<DisplayStateResidencyDataProvider>::make(displayId,
        "/sys/class/backlight/panel0-backlight/state",
        std::vector<std::string>{"Off", "LP", "1080x2340@60"});
    service->addStateResidencyDataProvider(displaySdp);

    // Add NFC power entity
    StateResidencyConfig nfcStateConfig = {
        .entryCountSupported = true,
        .entryCountPrefix = "Cumulative count:",
        .totalTimeSupported = true,
        .totalTimePrefix = "Cumulative duration msec:",
        .lastEntrySupported = true,
        .lastEntryPrefix = "Last entry timestamp msec:"
    };
    std::vector<std::pair<std::string, std::string>> nfcStateHeaders = {
        std::make_pair("Idle", "Idle mode:"),
        std::make_pair("Active", "Active mode:"),
        std::make_pair("Active-RW", "Active Reader/Writer mode:"),
    };

    sp<GenericStateResidencyDataProvider> nfcSdp =
            new GenericStateResidencyDataProvider("/sys/class/misc/st21nfc/device/power_stats");

    uint32_t nfcId = service->addPowerEntity("NFC", PowerEntityType::SUBSYSTEM);
    nfcSdp->addEntity(nfcId,
        PowerEntityConfig(generateGenericStateResidencyConfigs(nfcStateConfig, nfcStateHeaders)));

    service->addStateResidencyDataProvider(nfcSdp);

    // Add Power Entities that require the Aidl data provider
    auto aidlSdp = sp<AidlStateResidencyDataProvider>::make();
    uint32_t citadelId = service->addPowerEntity("Citadel", PowerEntityType::SUBSYSTEM);
    aidlSdp->addEntity(citadelId, "Citadel",
        std::vector<std::string>{"Last-Reset", "Active", "Deep-Sleep"});

    auto serviceStatus = android::defaultServiceManager()->addService(
        android::String16("power.stats-vendor"), aidlSdp);
    if (serviceStatus != android::OK) {
        ALOGE("Unable to register power.stats-vendor service %d", serviceStatus);
        return 1;
    }
    sp<android::ProcessState> ps{android::ProcessState::self()};  // Create non-HW binder threadpool
    ps->startThreadPool();

    service->addStateResidencyDataProvider(aidlSdp);

    // Configure the threadpool
    configureRpcThreadpool(1, true /*callerWillJoin*/);

    status_t status = service->registerAsService();
    if (status != OK) {
        ALOGE("Could not register service for power.stats HAL Iface (%d), exiting.", status);
        return 1;
    }

    ALOGI("power.stats service is ready");
    joinRpcThreadpool();

    // In normal operation, we don't expect the thread pool to exit
    ALOGE("power.stats service is shutting down");
    return 1;
}
