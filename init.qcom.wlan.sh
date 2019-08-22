#! /vendor/bin/sh

wlan_driver_version=`cat /sys/kernel/wlan/wlan/driver_version`
setprop vendor.wlan.driver.version "${wlan_driver_version:0:91}"
wlan_fw_version=`cat /sys/kernel/wlan/fw/1/version`
setprop vendor.wlan.firmware.version "${wlan_fw_version:0:91}"
