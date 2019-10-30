#ifndef ANDROID_HARDWARE_POWERSTATS_RAILDATAPROVIDER_H
#define ANDROID_HARDWARE_POWERSTATS_RAILDATAPROVIDER_H

#include <fmq/MessageQueue.h>
#include <pixelpowerstats/PowerStats.h>

namespace android {
namespace hardware {
namespace google {
namespace pixel {
namespace powerstats {

typedef MessageQueue<EnergyData, kSynchronizedReadWrite> MessageQueueSync;
struct RailData {
    std::string devicePath;
    uint32_t index;
    std::string subsysName;
    uint32_t samplingRate;
};

struct OnDeviceMmt {
    std::mutex mLock;
    bool hwEnabled;
    std::vector<std::string> devicePaths;
    std::map<std::string, RailData> railsInfo;
    std::vector<EnergyData> reading;
    std::unique_ptr<MessageQueueSync> fmqSynchronized;
};

class RailDataProvider : public IRailDataProvider {
public:
    RailDataProvider();
    // Methods from ::android::hardware::power::stats::V1_0::IPowerStats follow.
    Return<void> getRailInfo(IPowerStats::getRailInfo_cb _hidl_cb) override;
    Return<void> getEnergyData(const hidl_vec<uint32_t>& railIndices,
                        IPowerStats::getEnergyData_cb _hidl_cb) override;
    Return<void> streamEnergyData(uint32_t timeMs, uint32_t samplingRate,
                        IPowerStats::streamEnergyData_cb _hidl_cb) override;
 private:
     OnDeviceMmt mOdpm;
     void findIioPowerMonitorNodes();
     size_t parsePowerRails();
     int parseIioEnergyNode(std::string devName);
     Status parseIioEnergyNodes();
};

}  // namespace powerstats
}  // namespace pixel
}  // namespace google
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_POWERSTATS_RAILDATAPROVIDER_H
