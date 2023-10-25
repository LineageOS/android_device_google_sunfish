/*
 * Copyright 2018 The Android Open Source Project
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

#define LOG_TAG "dumpstate"

#include "DumpstateDevice.h"

#include <android-base/file.h>
#include <android-base/properties.h>
#include <android-base/unique_fd.h>
#include <cutils/properties.h>
#include <hidl/HidlBinderSupport.h>
#include <hidl/HidlSupport.h>

#include <log/log.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>

#define _SVID_SOURCE
#include <dirent.h>

#include "DumpstateUtil.h"

#define MODEM_LOG_PREFIX_PROPERTY "ro.vendor.radio.log_prefix"
#define MODEM_LOG_LOC_PROPERTY "ro.vendor.radio.log_loc"
#define MODEM_LOGGING_SWITCH "persist.vendor.radio.smlog_switch"

#define DIAG_MDLOG_PERSIST_PROPERTY "persist.vendor.sys.modem.diag.mdlog"
#define DIAG_MDLOG_PROPERTY "vendor.sys.modem.diag.mdlog"
#define DIAG_MDLOG_STATUS_PROPERTY "vendor.sys.modem.diag.mdlog_on"

#define DIAG_MDLOG_NUMBER_BUGREPORT "persist.vendor.sys.modem.diag.mdlog_br_num"

#define TCPDUMP_NUMBER_BUGREPORT "persist.vendor.tcpdump.log.br_num"
#define TCPDUMP_PERSIST_PROPERTY "persist.vendor.tcpdump.log.alwayson"

#define MODEM_EFS_DUMP_PROPERTY "vendor.sys.modem.diag.efsdump"

#define VENDOR_VERBOSE_LOGGING_ENABLED_PROPERTY "persist.vendor.verbose_logging_enabled"

using android::os::dumpstate::CommandOptions;
using android::os::dumpstate::DumpFileToFd;
using android::os::dumpstate::PropertiesHelper;
using android::os::dumpstate::RunCommandToFd;

namespace android {
namespace hardware {
namespace dumpstate {
namespace V1_1 {
namespace implementation {

#define DIAG_LOG_PREFIX "diag_log_"
#define TCPDUMP_LOG_PREFIX "tcpdump"
#define EXTENDED_LOG_PREFIX "extended_log_"

#define BUFSIZE 65536
static void copyFile(std::string srcFile, std::string destFile) {
    uint8_t buffer[BUFSIZE];
    ssize_t size;

    int fdSrc = open(srcFile.c_str(), O_RDONLY);
    if (fdSrc < 0) {
        ALOGD("Failed to open source file %s\n", srcFile.c_str());
        return;
    }

    int fdDest = open(destFile.c_str(), O_WRONLY | O_CREAT, 0666);
    if (fdDest < 0) {
        ALOGD("Failed to open destination file %s\n", destFile.c_str());
        close(fdSrc);
        return;
    }

    ALOGD("Copying %s to %s\n", srcFile.c_str(), destFile.c_str());
    while ((size = TEMP_FAILURE_RETRY(read(fdSrc, buffer, BUFSIZE))) > 0) {
        TEMP_FAILURE_RETRY(write(fdDest, buffer, size));
    }

    close(fdDest);
    close(fdSrc);
}

struct PosixTarHeader {
    char name[100];               /*   0 */
    char mode[8];                 /* 100 */
    char uid[8];                  /* 108 */
    char gid[8];                  /* 116 */
    char size[12];                /* 124 */
    char mtime[12];               /* 136 */
    char chksum[8];               /* 148 */
    char typeflag;                /* 156 */
    char linkname[100];           /* 157 */
    char magic[6];                /* 257 */
    char version[2];              /* 259 */
    char uname[32];               /* 265 */
    char gname[32];               /* 297 */
    char devmajor[8];             /* 329 */
    char devminor[8];             /* 337 */
    char prefix[155];             /* 345 */
    char pad[12];                 /* 500 */
};

static unsigned int tarCheckSum(PosixTarHeader *header) {
    unsigned int sum = 0;
    char *p = (char *)header;
    char *q = p + sizeof(PosixTarHeader);
    for (int i = 0; i < 8; i++) {
        header->chksum[i] = ' ';
    }
    while (p < q) {
        sum += *p++ & 0xff;
    }
    return sum;
}

static PosixTarHeader *tarHeader(PosixTarHeader *header, char *fileName, ssize_t fileSize) {
    memset(header, 0, sizeof(PosixTarHeader));
    strcpy(header->name, fileName);
    sprintf(header->mode, "%07o", 0600);
    sprintf(header->size, "%011llo", (long long unsigned int)fileSize);
    sprintf(header->mtime, "%011o", 0);
    header->typeflag = '0';
    strcpy(header->magic, "ustar");
    strcpy(header->version, " ");
    sprintf(header->chksum, "%06o", tarCheckSum(header));
    return header;
}

static void createTarFile(std::string tarFile, std::string srcDir) {
    uint8_t buffer[BUFSIZE];
    struct dirent *dirent;
    struct stat st;
    PosixTarHeader header;
    ssize_t size, tarLen = 0, blockSize = sizeof(PosixTarHeader);
    char padding = '\0';

    DIR *dirp = opendir(srcDir.c_str());
    if (dirp == NULL) {
        ALOGD("Unable to open folder %s\n", srcDir.c_str());
        return;
    }

    int fdTar = open(tarFile.c_str(), O_WRONLY | O_CREAT, 0666);
    if (fdTar < 0) {
        ALOGD("Unable to open file %s\n", tarFile.c_str());
        closedir(dirp);
        return;
    }

    ALOGD("Creating tar file %s\n", tarFile.c_str());
    while ((dirent = readdir(dirp)) != NULL) {
        if (dirent->d_name[0] == '.') {
            continue;
        }

        std::string path = srcDir + "/" + dirent->d_name;
        int fd = open(path.c_str(), O_RDONLY);
        if (fd < 0) {
            ALOGD("Unable to open file %s\n", path.c_str());
            continue;
        }
        fstat(fd, &st);

        if (TEMP_FAILURE_RETRY(write(fdTar, tarHeader(
            &header, dirent->d_name, st.st_size), blockSize)) <= 0) {
            ALOGD("Error while writing file %s, errno=%d\n", tarFile.c_str(), errno);
            close(fd);
            continue;
        }
        tarLen += blockSize;

        while ((size = TEMP_FAILURE_RETRY(read(fd, buffer, BUFSIZE))) > 0) {
            write(fdTar, buffer, size);
            tarLen += size;
        }

        while (tarLen % blockSize != 0) {
            write(fdTar, &padding, 1);
            tarLen++;
        }
        close(fd);
    }
    close(fdTar);
    closedir(dirp);
}

static void dumpLogs(int fd, std::string srcDir, std::string destDir,
                     int maxFileNum, const char *logPrefix) {
    (void) fd;
    struct dirent **dirent_list = NULL;
    int num_entries = scandir(srcDir.c_str(),
                              &dirent_list,
                              0,
                              (int (*)(const struct dirent **, const struct dirent **)) alphasort);

    if (!dirent_list) {
        return;
    } else if (num_entries <= 0) {
        return;
    }

    int copiedFiles = 0;

    for (int i = num_entries - 1; i >= 0; i--) {
        ALOGD("Found %s\n", dirent_list[i]->d_name);

        if (0 != strncmp(dirent_list[i]->d_name, logPrefix, strlen(logPrefix))) {
            continue;
        }

        if ((copiedFiles >= maxFileNum) && (maxFileNum != -1)) {
            ALOGD("Skipped %s\n", dirent_list[i]->d_name);
            continue;
        }

        copiedFiles++;

        std::string srcLogFile = srcDir + "/" + dirent_list[i]->d_name;
        std::string destLogFile = destDir + "/" + dirent_list[i]->d_name;
        copyFile(srcLogFile, destLogFile);
    }

    while (num_entries--) {
        free(dirent_list[num_entries]);
    }

    free(dirent_list);
}

static void *dumpModemThread(void *data)
{
    long fdModem = (long)data;

    ALOGD("dumpModemThread started\n");

    std::string modemLogDir = android::base::GetProperty(MODEM_LOG_LOC_PROPERTY, "");
    if (modemLogDir.empty()) {
        ALOGD("No modem log place is set");
        return NULL;
    }

    std::string filePrefix = android::base::GetProperty(MODEM_LOG_PREFIX_PROPERTY, "");

    if (filePrefix.empty()) {
        ALOGD("Modem log prefix is not set");
        return NULL;
    }

    bool diagLogEnabled = android::base::GetBoolProperty(DIAG_MDLOG_PERSIST_PROPERTY, false);
    bool diagLogStarted = android::base::GetBoolProperty(DIAG_MDLOG_STATUS_PROPERTY, false);

    if (diagLogEnabled) {
        if (diagLogStarted) {
            android::base::SetProperty(DIAG_MDLOG_PROPERTY, "false");
            ALOGD("Stopping diag_mdlog...\n");
            if (android::base::WaitForProperty(DIAG_MDLOG_STATUS_PROPERTY, "false", std::chrono::seconds(10))) {
                ALOGD("diag_mdlog exited");
            } else {
                ALOGE("Waited mdlog timeout after 10 second");
            }
        } else {
            ALOGD("diag_mdlog is not running");
        }
    }

    sleep(1);
    ALOGD("Waited modem for 1 second to flush logs");

    const std::string modemLogCombined = modemLogDir + "/" + filePrefix + "all.tar";
    const std::string modemLogAllDir = modemLogDir + "/modem_log";

    RunCommandToFd(STDOUT_FILENO, "MKDIR MODEM LOG", {"/vendor/bin/mkdir", "-p", modemLogAllDir.c_str()}, CommandOptions::WithTimeout(2).Build());

    const std::string diagLogDir = "/data/vendor/radio/diag_logs/logs";
    const std::string diagPoweronLogPath = "/data/vendor/radio/diag_logs/logs/diag_poweron_log.qmdl";

    if (diagLogEnabled) {
        dumpLogs(STDOUT_FILENO, diagLogDir, modemLogAllDir, android::base::GetIntProperty(DIAG_MDLOG_NUMBER_BUGREPORT, 100), DIAG_LOG_PREFIX);

        if (diagLogStarted) {
            ALOGD("Restarting diag_mdlog...");
            android::base::SetProperty(DIAG_MDLOG_PROPERTY, "true");
        }
    }
    copyFile(diagPoweronLogPath, modemLogAllDir + "/" + basename(diagPoweronLogPath.c_str()));

    if (!PropertiesHelper::IsUserBuild()) {
        char cmd[256] = { 0 };

        android::base::SetProperty(MODEM_EFS_DUMP_PROPERTY, "true");

        const std::string tcpdumpLogDir = "/data/vendor/tcpdump_logger/logs";
        const std::string extendedLogDir = "/data/vendor/radio/extended_logs";
        const std::vector <std::string> rilAndNetmgrLogs
            {
                "/data/vendor/radio/haldebug_ril0",
                "/data/vendor/radio/haldebug_ril1",
                "/data/vendor/radio/ril_log0",
                "/data/vendor/radio/ril_log0_old",
                "/data/vendor/radio/ril_log1",
                "/data/vendor/radio/ril_log1_old",
                "/data/vendor/radio/qmi_fw_log",
                "/data/vendor/radio/qmi_fw_log_old",
                "/data/vendor/radio/imsdatadaemon_log",
                "/data/vendor/radio/imsdatadaemon_log_old",
                "/data/vendor/netmgr/netmgr_log",
                "/data/vendor/netmgr/netmgr_log_old",
                "/data/vendor/radio/omadm_logs.txt",
                "/data/vendor/radio/power_anomaly_data.txt",
                "/data/vendor/radio/diag_logs/diag_trace.txt",
                "/data/vendor/radio/diag_logs/diag_trace_old.txt",
                "/data/vendor/radio/metrics_data",
                "/data/vendor/ssrlog/ssr_log.txt",
                "/data/vendor/ssrlog/ssr_log_old.txt",
                "/data/vendor/rfs/mpss/modem_efs",
                "/sys/kernel/debug/ipa/ipa_statistics_msg"
            };

       bool tcpdumpEnabled = android::base::GetBoolProperty(TCPDUMP_PERSIST_PROPERTY, false);
       if (tcpdumpEnabled) {
            dumpLogs(STDOUT_FILENO, tcpdumpLogDir, modemLogAllDir, android::base::GetIntProperty(TCPDUMP_NUMBER_BUGREPORT, 5), TCPDUMP_LOG_PREFIX);
        }

        for (const auto& logFile : rilAndNetmgrLogs) {
            copyFile(logFile, modemLogAllDir + "/" + basename(logFile.c_str()));
        }

        //Dump IPA log
        snprintf(cmd, sizeof(cmd),
                "cat /d/ipc_logging/ipa/log > %s/ipa_log",
                modemLogAllDir.c_str());
        RunCommandToFd(STDOUT_FILENO, "Dump IPA log", {"/vendor/bin/sh", "-c", cmd});

        dumpLogs(STDOUT_FILENO, extendedLogDir, modemLogAllDir, 100, EXTENDED_LOG_PREFIX);
        android::base::SetProperty(MODEM_EFS_DUMP_PROPERTY, "false");
    }

    createTarFile(modemLogCombined, modemLogAllDir);

    std::vector<uint8_t> buffer(65536);
    android::base::unique_fd fdLog(TEMP_FAILURE_RETRY(open(modemLogCombined.c_str(), O_RDONLY | O_CLOEXEC | O_NONBLOCK)));

    if (fdLog >= 0) {
        while (1) {
            ssize_t bytes_read = TEMP_FAILURE_RETRY(read(fdLog, buffer.data(), buffer.size()));

            if (bytes_read == 0) {
                break;
            } else if (bytes_read < 0) {
                ALOGD("read(%s): %s\n", modemLogCombined.c_str(), strerror(errno));
                break;
            }

            ssize_t result = TEMP_FAILURE_RETRY(write(fdModem, buffer.data(), bytes_read));

            if (result != bytes_read) {
                ALOGD("Failed to write %ld bytes, actually written: %ld", bytes_read, result);
                break;
            }
        }
    }

    RunCommandToFd(STDOUT_FILENO, "RM MODEM DIR", { "/vendor/bin/rm", "-r", modemLogAllDir.c_str()}, CommandOptions::WithTimeout(2).Build());
    RunCommandToFd(STDOUT_FILENO, "RM LOG", { "/vendor/bin/rm", modemLogCombined.c_str()}, CommandOptions::WithTimeout(2).Build());

    ALOGD("dumpModemThread finished\n");

    return NULL;
}

static void DumpTouch(int fd) {
    const char touch_spi_path[] = "/sys/bus/i2c/drivers/fts/1-0049";
    char cmd[256];

    RunCommandToFd(fd, "Force Set AP as Bus Owner",
                   {"/vendor/bin/sh", "-c",
                    "echo A0 01 > /proc/fts/driver_test && "
                    "cat /proc/fts/driver_test"});

    snprintf(cmd, sizeof(cmd), "%s/appid", touch_spi_path);
    if (!access(cmd, R_OK)) {
        // Touch firmware version
        DumpFileToFd(fd, "STM touch firmware version", cmd);

        // Touch controller status
        snprintf(cmd, sizeof(cmd), "%s/status", touch_spi_path);
        DumpFileToFd(fd, "STM touch status", cmd);

        // Mutual raw data
        snprintf(cmd, sizeof(cmd),
                 "echo 13 00 01 > %s/stm_fts_cmd && cat %s/stm_fts_cmd",
                 touch_spi_path, touch_spi_path);
        RunCommandToFd(fd, "Mutual Raw", {"/vendor/bin/sh", "-c", cmd});

        // Mutual strength data
        snprintf(cmd, sizeof(cmd),
                 "echo 17 01 > %s/stm_fts_cmd && cat %s/stm_fts_cmd",
                 touch_spi_path, touch_spi_path);
        RunCommandToFd(fd, "Mutual Strength", {"/vendor/bin/sh", "-c", cmd});

        // Self raw data
        snprintf(cmd, sizeof(cmd),
                 "echo 15 00 01> %s/stm_fts_cmd && cat %s/stm_fts_cmd",
                 touch_spi_path, touch_spi_path);
        RunCommandToFd(fd, "Self Raw", {"/vendor/bin/sh", "-c", cmd});
    }

    if (!access("/proc/fts/driver_test", R_OK)) {
        RunCommandToFd(fd, "Lock Normal Active Mode",
                       {"/vendor/bin/sh", "-c",
                        "echo 16 A0 03 00 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
	RunCommandToFd(fd, "Mutual Raw Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 23 00 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Mutual Baseline Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 23 03 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Mutual Strength Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 23 02 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Self Raw Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 24 00 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Self Baseline Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 24 03 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Self Strength Data",
                       {"/vendor/bin/sh", "-c",
                        "echo 24 02 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Mutual Compensation",
                       {"/vendor/bin/sh", "-c",
                        "echo 32 10 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Self Compensation",
                       {"/vendor/bin/sh", "-c",
                        "echo 33 12 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
	RunCommandToFd(fd, "Golden MS Raw",
                       {"/vendor/bin/sh", "-c",
                        "echo 34 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
        RunCommandToFd(fd, "Packaging Plant - HW reset",
                       {"/vendor/bin/sh", "-c",
                        "echo 01 FA 20 00 00 24 80 > /proc/fts/driver_test"});
	RunCommandToFd(fd, "Packaging Plant - Hibernate Memory",
                       {"/vendor/bin/sh", "-c",
                        "echo 01 FA 20 00 00 68 08 > /proc/fts/driver_test"});
	RunCommandToFd(fd, "Packaging Plant - Read 10 bytes from Address 0x00043F28",
                       {"/vendor/bin/sh", "-c",
                        "echo 02 FA 00 04 3F 28 00 0A 00 > /proc/fts/driver_test && "
                        "cat /proc/fts/driver_test"});
    }

    snprintf(cmd, sizeof(cmd), "%s/stm_fts_cmd", touch_spi_path);
    if (!access(cmd, R_OK)) {
        // ITO raw data
        snprintf(cmd, sizeof(cmd),
                 "echo 01 > %s/stm_fts_cmd && cat %s/stm_fts_cmd",
                 touch_spi_path, touch_spi_path);
        RunCommandToFd(fd, "ITO Raw", {"/vendor/bin/sh", "-c", cmd});
    }
    RunCommandToFd(fd, "Restore Bus Owner",
                   {"/vendor/bin/sh", "-c",
                    "echo A0 00 > /proc/fts/driver_test && "
                    "cat /proc/fts/driver_test"});
}

static void DumpDisplay(int fd) {
    DumpFileToFd(fd, "PANEL VENDOR NAME", "/sys/class/panel_info/panel0/panel_vendor_name");
    DumpFileToFd(fd, "PANEL SN", "/sys/class/panel_info/panel0/serial_number");
    DumpFileToFd(fd, "PANEL EXTRA INFO", "/sys/class/panel_info/panel0/panel_extinfo");

    const std::string pmic_regmap_path = "/sys/kernel/debug/regmap/spmi0-05";
    using android::base::WriteStringToFile;

    if (WriteStringToFile("0x80", pmic_regmap_path + "/count", true) &&
            WriteStringToFile("0xE000", pmic_regmap_path + "/address", true)) {
        DumpFileToFd(fd, "OLEDB Register Dump", pmic_regmap_path + "/data");
    } else {
        dprintf(fd, "Unable to print OLEDB Register Dump\n");
    }

    if (WriteStringToFile("0x80", pmic_regmap_path + "/count", true) &&
            WriteStringToFile("0xDE00", pmic_regmap_path + "/address", true)) {
        DumpFileToFd(fd, "ELVDD Register Dump", pmic_regmap_path + "/data");
    } else {
       dprintf(fd, "Unable to print ELVDD Register Dump\n");
    }

    if (WriteStringToFile("0x60", pmic_regmap_path + "/count", true) &&
            WriteStringToFile("0xDC00", pmic_regmap_path + "/address", true)) {
        DumpFileToFd(fd, "ELVSS Register Dump", pmic_regmap_path + "/data");
    } else {
        dprintf(fd, "Unable to print ELVSS Register Dump\n");
    }
}

static void DumpSensorLog(int fd) {
    const std::string logPath = "/data/vendor/sensors/log/sensor_log.txt";
    const std::string lastlogPath = "/data/vendor/sensors/log/sensor_lastlog.txt";

    if (!access(logPath.c_str(), R_OK)) {
        DumpFileToFd(fd, "sensor log", logPath);
    }
    if (!access(lastlogPath.c_str(), R_OK)) {
        DumpFileToFd(fd, "sensor lastlog", lastlogPath);
    }
}

static void DumpF2FS(int fd) {
    DumpFileToFd(fd, "F2FS", "/sys/kernel/debug/f2fs/status");
    DumpFileToFd(fd, "F2FS", "/dev/fscklogs/fsck");
    RunCommandToFd(fd, "F2FS - fsck time (ms)", {"/vendor/bin/sh", "-c", "getprop ro.boottime.init.fsck.data"});
    RunCommandToFd(fd, "F2FS - checkpoint=disable time (ms)", {"/vendor/bin/sh", "-c", "getprop ro.boottime.init.mount.data"});
}

static void DumpUFS(int fd) {
    DumpFileToFd(fd, "UFS model", "/sys/block/sda/device/model");
    DumpFileToFd(fd, "UFS rev", "/sys/block/sda/device/rev");
    DumpFileToFd(fd, "UFS size", "/sys/block/sda/size");
    DumpFileToFd(fd, "UFS show_hba", "/sys/kernel/debug/ufshcd0/show_hba");

    DumpFileToFd(fd, "UFS Slow IO Read", "/dev/sys/block/bootdevice/slowio_read_cnt");
    DumpFileToFd(fd, "UFS Slow IO Write", "/dev/sys/block/bootdevice/slowio_write_cnt");
    DumpFileToFd(fd, "UFS Slow IO Unmap", "/dev/sys/block/bootdevice/slowio_unmap_cnt");
    DumpFileToFd(fd, "UFS Slow IO Sync", "/dev/sys/block/bootdevice/slowio_sync_cnt");

    std::string ufs_health = "for f in $(find /dev/sys/block/bootdevice/health -type f); do if [[ -r $f && -f $f ]]; then echo --- $f; cat $f; echo ''; fi; done";
    RunCommandToFd(fd, "UFS health", {"/vendor/bin/sh", "-c", ufs_health.c_str()});
    RunCommandToFd(fd, "UFS err_stats", {"/vendor/bin/sh", "-c",
                       "path=\"/dev/sys/block/bootdevice/err_stats\"; "
                       "for node in `ls $path/err_*`; do "
                       "printf \"%s:%d\\n\" $(basename $node) $(cat $node); done;"});
    RunCommandToFd(fd, "UFS io_stats", {"/vendor/bin/sh", "-c",
                       "path=\"/dev/sys/block/bootdevice/io_stats\"; "
                       "printf \"\\t\\t%-10s %-10s %-10s %-10s %-10s %-10s\\n\" "
                       "ReadCnt ReadBytes WriteCnt WriteBytes RWCnt RWBytes; "
                       "str=$(cat $path/*_start); arr=($str); "
                       "printf \"Started: \\t%-10s %-10s %-10s %-10s %-10s %-10s\\n\" "
                       "${arr[1]} ${arr[0]} ${arr[5]} ${arr[4]} ${arr[3]} ${arr[2]}; "
                       "str=$(cat $path/*_complete); arr=($str); "
                       "printf \"Completed: \\t%-10s %-10s %-10s %-10s %-10s %-10s\\n\" "
                       "${arr[1]} ${arr[0]} ${arr[5]} ${arr[4]} ${arr[3]} ${arr[2]}; "
                       "str=$(cat $path/*_maxdiff); arr=($str); "
                       "printf \"MaxDiff: \\t%-10s %-10s %-10s %-10s %-10s %-10s\\n\\n\" "
                       "${arr[1]} ${arr[0]} ${arr[5]} ${arr[4]} ${arr[3]} ${arr[2]}; "});

    RunCommandToFd(fd, "UFS req_stats", {"/vendor/bin/sh", "-c",
                       "path=\"/dev/sys/block/bootdevice/req_stats\"; "
                       "printf \"\\t%-10s %-10s %-10s %-10s %-10s %-10s %-10s\\n\" "
                       "All Write Read Read\\(urg\\) Write\\(urg\\) Flush Discard; "
                       "str=$(cat $path/*_min); arr=($str); "
                       "printf \"Min:\\t%-10s %-10s %-10s %-10s %-10s %-10s %-10s\\n\" "
                       "${arr[0]} ${arr[3]} ${arr[6]} ${arr[4]} ${arr[5]} ${arr[2]} ${arr[1]}; "
                       "str=$(cat $path/*_max); arr=($str); "
                       "printf \"Max:\\t%-10s %-10s %-10s %-10s %-10s %-10s %-10s\\n\" "
                       "${arr[0]} ${arr[3]} ${arr[6]} ${arr[4]} ${arr[5]} ${arr[2]} ${arr[1]}; "
                       "str=$(cat $path/*_avg); arr=($str); "
                       "printf \"Avg.:\\t%-10s %-10s %-10s %-10s %-10s %-10s %-10s\\n\" "
                       "${arr[0]} ${arr[3]} ${arr[6]} ${arr[4]} ${arr[5]} ${arr[2]} ${arr[1]}; "
                       "str=$(cat $path/*_sum); arr=($str); "
                       "printf \"Count:\\t%-10s %-10s %-10s %-10s %-10s %-10s %-10s\\n\\n\" "
                       "${arr[0]} ${arr[3]} ${arr[6]} ${arr[4]} ${arr[5]} ${arr[2]} ${arr[1]};"});
}

static void DumpPower(int fd) {
    RunCommandToFd(fd, "Power Stats Times", {"/vendor/bin/sh", "-c",
                   "echo -n \"Boot: \" && /vendor/bin/uptime -s && "
                   "echo -n \"Now: \" && date"});
    DumpFileToFd(fd, "Sleep Stats", "/sys/power/system_sleep/stats");
    DumpFileToFd(fd, "Power Management Stats", "/sys/power/rpmh_stats/master_stats");
    DumpFileToFd(fd, "WLAN Power Stats", "/sys/kernel/wlan/power_stats");
}

// Methods from ::android::hardware::dumpstate::V1_0::IDumpstateDevice follow.
Return<void> DumpstateDevice::dumpstateBoard(const hidl_handle& handle) {
    // Ignore return value, just return an empty status.
    dumpstateBoard_1_1(handle, DumpstateMode::DEFAULT, 30 * 1000 /* timeoutMillis */);
    return Void();
}

// Methods from ::android::hardware::dumpstate::V1_1::IDumpstateDevice follow.
Return<DumpstateStatus> DumpstateDevice::dumpstateBoard_1_1(const hidl_handle& handle,
                                                            const DumpstateMode mode,
                                                            const uint64_t timeoutMillis) {
    // Unused arguments.
    (void) timeoutMillis;

    // Exit when dump is completed since this is a lazy HAL.
    addPostCommandTask([]() {
        exit(0);
    });

    if (handle == nullptr || handle->numFds < 1) {
        ALOGE("no FDs\n");
        return DumpstateStatus::ILLEGAL_ARGUMENT;
    }

    int fd = handle->data[0];
    if (fd < 0) {
        ALOGE("invalid FD: %d\n", handle->data[0]);
        return DumpstateStatus::ILLEGAL_ARGUMENT;
    }

    bool isModeValid = false;
    for (const auto dumpstateMode : hidl_enum_range<DumpstateMode>()) {
        if (mode == dumpstateMode) {
            isModeValid = true;
            break;
        }
    }
    if (!isModeValid) {
        ALOGE("Invalid mode: %d\n", mode);
        return DumpstateStatus::ILLEGAL_ARGUMENT;
    } else if (mode == DumpstateMode::WEAR) {
        // We aren't a Wear device.
        ALOGE("Unsupported mode: %d\n", mode);
        return DumpstateStatus::UNSUPPORTED_MODE;
    }

    RunCommandToFd(fd, "Notify modem", {"/vendor/bin/modem_svc", "-s"}, CommandOptions::WithTimeout(1).Build());

    pthread_t modemThreadHandle = 0;
    if (getVerboseLoggingEnabled()) {
        ALOGD("Verbose logging is enabled.\n");
        if (handle->numFds < 2) {
            ALOGE("no FD for modem\n");
        } else {
            int fdModem = handle->data[1];
            if (pthread_create(&modemThreadHandle, NULL, dumpModemThread, (void *)((long)fdModem)) != 0) {
                ALOGE("could not create thread for dumpModem\n");
            }
        }
    }

    RunCommandToFd(fd, "VENDOR PROPERTIES", {"/vendor/bin/getprop"});
    DumpFileToFd(fd, "CPU present", "/sys/devices/system/cpu/present");
    DumpFileToFd(fd, "CPU online", "/sys/devices/system/cpu/online");
    DumpFileToFd(fd, "Bootloader Log", "/proc/bldrlog");
    DumpTouch(fd);
    DumpDisplay(fd);

    DumpF2FS(fd);
    DumpUFS(fd);

    DumpSensorLog(fd);

    DumpFileToFd(fd, "INTERRUPTS", "/proc/interrupts");

    DumpPower(fd);

    DumpFileToFd(fd, "LL-Stats", "/d/wlan0/ll_stats");
    DumpFileToFd(fd, "WLAN Connect Info", "/d/wlan0/connect_info");
    DumpFileToFd(fd, "WLAN Offload Info", "/d/wlan0/offload_info");
    DumpFileToFd(fd, "WLAN Roaming Stats", "/d/wlan0/roam_stats");
    DumpFileToFd(fd, "ICNSS Stats", "/d/icnss/stats");
    DumpFileToFd(fd, "SMD Log", "/d/ipc_logging/smd/log");
    RunCommandToFd(fd, "ION HEAPS", {"/vendor/bin/sh", "-c", "for d in $(ls -d /d/ion/*); do for f in $(ls $d); do echo --- $d/$f; cat $d/$f; done; done"});
    DumpFileToFd(fd, "dmabuf info", "/d/dma_buf/bufinfo");
    DumpFileToFd(fd, "dmabuf process info", "/d/dma_buf/dmaprocs");
    RunCommandToFd(fd, "Temperatures", {"/vendor/bin/sh", "-c", "for f in /sys/class/thermal/thermal* ; do type=`cat $f/type` ; temp=`cat $f/temp` ; echo \"$type: $temp\" ; done"});
    RunCommandToFd(fd, "Cooling Device Current State", {"/vendor/bin/sh", "-c", "for f in /sys/class/thermal/cooling* ; do type=`cat $f/type` ; temp=`cat $f/cur_state` ; echo \"$type: $temp\" ; done"});
    RunCommandToFd(
        fd, "LMH info",
        {"/vendor/bin/sh", "-c",
         "for f in /sys/bus/platform/drivers/msm_lmh_dcvs/*qcom,limits-dcvs@*/lmh_freq_limit; do "
         "state=`cat $f` ; echo \"$f: $state\" ; done"});
    RunCommandToFd(fd, "CPU time-in-state", {"/vendor/bin/sh", "-c", "for cpu in /sys/devices/system/cpu/cpu*; do f=$cpu/cpufreq/stats/time_in_state; if [ ! -f $f ]; then continue; fi; echo $f:; cat $f; done"});
    RunCommandToFd(fd, "CPU cpuidle", {"/vendor/bin/sh", "-c", "for cpu in /sys/devices/system/cpu/cpu*; do for d in $cpu/cpuidle/state*; do if [ ! -d $d ]; then continue; fi; echo \"$d: `cat $d/name` `cat $d/desc` `cat $d/time` `cat $d/usage`\"; done; done"});
    RunCommandToFd(fd, "Airbrush debug info", {"/vendor/bin/sh", "-c", "for f in `ls /sys/devices/platform/soc/c84000.i2c/i2c-4/4-0066/@(*curr|temperature|vbat|total_power)`; do echo \"$f: `cat $f`\" ; done; file=/d/airbrush/airbrush_sm/chip_state; echo \"$file: `cat $file`\""});
    DumpFileToFd(fd, "TCPM logs", "/d/tcpm/usbpd0");
    DumpFileToFd(fd, "PD Engine", "/d/logbuffer/usbpd");
    DumpFileToFd(fd, "ipc-local-ports", "/d/msm_ipc_router/dump_local_ports");
    RunCommandToFd(fd, "TRICKLE-DEFEND Config", {"/vendor/bin/sh", "-c", " cd /sys/devices/platform/soc/soc:google,battery/power_supply/battery/; echo \"bd_trickle_enable: `cat bd_trickle_enable`\"; echo \"bd_trickle_cnt: `cat bd_trickle_cnt`\";  echo \"bd_trickle_recharge_soc: `cat bd_trickle_recharge_soc`\";  echo \"bd_trickle_dry_run: `cat bd_trickle_dry_run`\";  echo \"bd_trickle_reset_sec: `cat bd_trickle_reset_sec`\""});
    RunCommandToFd(fd, "DWELL-DEFEND Config", {"/vendor/bin/sh", "-c", " cd /sys/devices/platform/soc/soc:google,charger/; for f in `ls charge_s*` ; do echo \"$f: `cat $f`\" ; done"});
    RunCommandToFd(fd, "TEMP-DEFEND Config", {"/vendor/bin/sh", "-c", " cd /sys/devices/platform/soc/soc:google,charger/; for f in `ls bd_*` ; do echo \"$f: `cat $f`\" ; done"});
    RunCommandToFd(fd, "USB Device Descriptors", {"/vendor/bin/sh", "-c", "cd /sys/bus/usb/devices/1-1 && cat product && cat bcdDevice; cat descriptors | od -t x1 -w16 -N96"});
    RunCommandToFd(fd, "Power supply properties", {"/vendor/bin/sh", "-c", "for f in `ls /sys/class/power_supply/*/uevent` ; do echo \"------ $f\\n`cat $f`\\n\" ; done"});
    RunCommandToFd(fd, "PMIC Votables", {"/vendor/bin/sh", "-c", "cat /sys/kernel/debug/pmic-votable/*/status"});
    RunCommandToFd(fd, "Google Charger", {"/vendor/bin/sh", "-c", "cd /d/google_charger/; for f in `ls pps_*` ; do echo \"$f: `cat $f`\" ; done"});
    RunCommandToFd(fd, "Google Battery", {"/vendor/bin/sh", "-c", "cd /d/google_battery/; for f in `ls ssoc_*` ; do echo \"$f: `cat $f`\" ; done"});
    RunCommandToFd(fd, "Battery EEPROM", {"/vendor/bin/sh", "-c", "xxd /sys/devices/platform/soc/a8c000.i2c/i2c-2/2-0050/2-00500/nvmem"});
    DumpFileToFd(fd, "BMS", "/d/logbuffer/ssoc");
    DumpFileToFd(fd, "smblib", "/d/logbuffer/smblib");
    DumpFileToFd(fd, "TTF", "/d/logbuffer/ttf");
    DumpFileToFd(fd, "TTF details", "/sys/class/power_supply/battery/ttf_details");
    DumpFileToFd(fd, "TTF stats", "/sys/class/power_supply/battery/ttf_stats");
    DumpFileToFd(fd, "aacr_state", "/sys/class/power_supply/battery/aacr_state");

    DumpFileToFd(fd, "Modem Stat", "/data/vendor/modem_stat/debug.txt");
    DumpFileToFd(fd, "Pixel trace", "/d/tracing/instances/pixel-trace/trace");
    DumpFileToFd(fd, "Charging table dump", "/d/google_battery/chg_raw_profile");

    // Slower dump put later in case stuck the rest of dump
    // Timeout after 3s as TZ log missing EOF
    RunCommandToFd(fd, "QSEE logs", {"/vendor/bin/sh", "-c", "/vendor/bin/timeout 3 cat /d/tzdbg/qsee_log"});

    // Citadel info
    RunCommandToFd(fd, "Citadel VERSION", {"/vendor/bin/hw/citadel_updater", "-lv"});
    RunCommandToFd(fd, "Citadel STATS", {"/vendor/bin/hw/citadel_updater", "--stats"});
    RunCommandToFd(fd, "Citadel BOARDID", {"/vendor/bin/hw/citadel_updater", "--board_id"});

    // Dump fastrpc dma buffer size
    DumpFileToFd(fd, "Fastrpc dma buffer", "/sys/kernel/fastrpc/total_dma_kb");

    // Keep this at the end as very long on not for humans
    DumpFileToFd(fd, "WLAN FW Log Symbol Table", "/vendor/firmware/Data.msc");

    if (modemThreadHandle) {
        pthread_join(modemThreadHandle, NULL);
    }

    return DumpstateStatus::OK;
}

Return<void> DumpstateDevice::setVerboseLoggingEnabled(const bool enable) {
    android::base::SetProperty(VENDOR_VERBOSE_LOGGING_ENABLED_PROPERTY, enable ? "true" : "false");
    return Void();
}

Return<bool> DumpstateDevice::getVerboseLoggingEnabled() {
    return android::base::GetBoolProperty(VENDOR_VERBOSE_LOGGING_ENABLED_PROPERTY, false);
}

}  // namespace implementation
}  // namespace V1_1
}  // namespace dumpstate
}  // namespace hardware
}  // namespace android
