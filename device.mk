#
# Copyright (C) 2019 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := device/google/sunfish

PRODUCT_VENDOR_MOVE_ENABLED := true
TARGET_BOARD_PLATFORM := sm6150
MSMSTEPPE := sm6150

PRODUCT_CHECK_ELF_FILES := true

PRODUCT_SOONG_NAMESPACES += \
    device/google/sunfish \
    hardware/google/av \
    hardware/google/interfaces \
    hardware/google/pixel \
    hardware/qcom/sm7150 \
    hardware/qcom/sm7150/display \
    vendor/google/camera \
    vendor/qcom/sm7150 \
    vendor/google/interfaces

# Single vendor RIL/Telephony/data with SM8150
DEVICE_USES_SM8150_QCRIL_TELEPHONY := true

ifeq ($(DEVICE_USES_SM8150_QCRIL_TELEPHONY), true)
  PRODUCT_SOONG_NAMESPACES += \
      vendor/qcom/sm8150/codeaurora/telephony/ims \
      vendor/qcom/sm8150/proprietary/data/permissions \
      vendor/qcom/sm8150/proprietary/qcril-data-hal/qdp \
      vendor/qcom/sm8150/proprietary/qcril-data-hal/util \
      vendor/qcom/sm8150/proprietary/qcril-data-hal/datamodule \
      vendor/qcom/sm8150/proprietary/qcril-hal
else
  $(warning DEVICE_USES_SM8150_QCRIL_TELEPHONY is disabled)

  PRODUCT_SOONG_NAMESPACES += \
      vendor/qcom/sm7150/codeaurora/commonsys/telephony/ims \
      vendor/qcom/sm7150/proprietary/qcril-data-hal \
      vendor/qcom/sm7150/proprietary/qcril-hal
endif

PRODUCT_PROPERTY_OVERRIDES += \
    keyguard.no_require_sim=true

# enable cal by default on accel sensor
PRODUCT_PRODUCT_PROPERTIES += \
    persist.debug.sensors.accel_cal=1

# The default value of this variable is false and should only be set to true when
# the device allows users to retain eSIM profiles after factory reset of user data.
PRODUCT_PRODUCT_PROPERTIES += \
    masterclear.allow_retain_esim_profiles_after_fdr=true

PRODUCT_COPY_FILES += \
    device/google/sunfish/default-permissions.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/default-permissions/default-permissions.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/handheld_core_hardware.xml \
    frameworks/native/data/etc/android.software.verified_boot.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/android.software.verified_boot.xml

# Enforce privapp-permissions whitelist
PRODUCT_PROPERTY_OVERRIDES += \
    ro.control_privapp_permissions=enforce

PRODUCT_PACKAGES += \
    messaging

TARGET_PRODUCT_PROP := $(LOCAL_PATH)/product.prop

$(call inherit-product, $(LOCAL_PATH)/utils.mk)

# Installs gsi keys into ramdisk, to boot a GSI with verified boot.
$(call inherit-product, $(SRC_TARGET_DIR)/product/gsi_keys.mk)

# Installs gsi keys into ramdisk, to boot a developer GSI with verified boot.
$(call inherit-product, $(SRC_TARGET_DIR)/product/developer_gsi_keys.mk)

ifeq ($(wildcard vendor/google_devices/sunfish/proprietary/device-vendor-sunfish.mk),)
    BUILD_WITHOUT_VENDOR := true
endif

ifeq ($(TARGET_PREBUILT_KERNEL),)
    LOCAL_KERNEL := device/google/sunfish-kernel/Image.lz4
else
    LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif
PRODUCT_VENDOR_KERNEL_HEADERS := device/google/sunfish-kernel/sm7150/kernel-headers


PRODUCT_CHARACTERISTICS := nosdcard
PRODUCT_SHIPPING_API_LEVEL := 28

DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/overlay

PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel \
    $(LOCAL_PATH)/fstab.hardware:$(TARGET_COPY_OUT_RECOVERY)/root/first_stage_ramdisk/fstab.$(PRODUCT_PLATFORM) \
    $(LOCAL_PATH)/fstab.hardware:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.$(PRODUCT_PLATFORM) \
    $(LOCAL_PATH)/fstab.persist:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.persist \
    $(LOCAL_PATH)/init.hardware.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).rc \
    $(LOCAL_PATH)/init.power.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).power.rc \
    $(LOCAL_PATH)/init.radio.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.radio.sh \
    $(LOCAL_PATH)/uinput-fpc.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/uinput-fpc.kl \
    $(LOCAL_PATH)/uinput-fpc.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/uinput-fpc.idc \
    $(LOCAL_PATH)/init.hardware.usb.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).usb.rc \
    $(LOCAL_PATH)/init.insmod.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.insmod.sh \
    $(LOCAL_PATH)/init.sensors.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.sensors.sh \
    $(LOCAL_PATH)/sensors.hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/hals.conf \
    $(LOCAL_PATH)/thermal-engine-$(PRODUCT_HARDWARE).conf:$(TARGET_COPY_OUT_VENDOR)/etc/thermal-engine-$(PRODUCT_HARDWARE).conf \
    $(LOCAL_PATH)/ueventd.rc:$(TARGET_COPY_OUT_VENDOR)/ueventd.rc \
    $(LOCAL_PATH)/init.ramoops.sh:$(TARGET_COPY_OUT_VENDOR)/bin/init.ramoops.sh \

MSM_VIDC_TARGET_LIST := sm6150 # Get the color format from kernel headers
MASTER_SIDE_CP_TARGET_LIST := sm6150 # ION specific settings

ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.diag.rc.userdebug:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).diag.rc
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.mpssrfs.rc.userdebug:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).mpssrfs.rc
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.chamber.rc.userdebug:$(TARGET_COPY_OUT_VENDOR)/etc/init/init.$(PRODUCT_PLATFORM).chamber.rc
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.ipa.rc.userdebug:$(TARGET_COPY_OUT_VENDOR)/etc/init/init.$(PRODUCT_PLATFORM).ipa.rc
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.power_debug.rc.userdebug:$(TARGET_COPY_OUT_VENDOR)/etc/init/init.$(PRODUCT_PLATFORM).power_debug.rc
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.userdebug.rc.userdebug:$(TARGET_COPY_OUT_VENDOR)/etc/init/init.$(PRODUCT_PLATFORM).userdebug.rc
else
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.diag.rc.user:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).diag.rc
  PRODUCT_COPY_FILES += \
      $(LOCAL_PATH)/init.hardware.mpssrfs.rc.user:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.$(PRODUCT_PLATFORM).mpssrfs.rc
endif

# A/B support
PRODUCT_PACKAGES += \
    otapreopt_script \
    cppreopts.sh \
    update_engine \
    update_verifier

# Use Sdcardfs
PRODUCT_PRODUCT_PROPERTIES += \
    ro.sys.sdcardfs=1

PRODUCT_PACKAGES += \
    bootctrl.sm6150 \
    bootctrl.sm6150.recovery

PRODUCT_PROPERTY_OVERRIDES += \
    ro.cp_system_other_odex=1

AB_OTA_POSTINSTALL_CONFIG += \
    RUN_POSTINSTALL_system=true \
    POSTINSTALL_PATH_system=system/bin/otapreopt_script \
    FILESYSTEM_TYPE_system=ext4 \
    POSTINSTALL_OPTIONAL_system=true

PRODUCT_PACKAGES += \
    update_engine_sideload \
    sg_write_buffer \
    f2fs_io \
    check_f2fs

AB_OTA_POSTINSTALL_CONFIG += \
    RUN_POSTINSTALL_vendor=true \
    POSTINSTALL_PATH_vendor=bin/checkpoint_gc \
    FILESYSTEM_TYPE_vendor=ext4 \
    POSTINSTALL_OPTIONAL_vendor=true

# Userdata Checkpointing OTA GC
PRODUCT_PACKAGES += \
    checkpoint_gc

# The following modules are included in debuggable builds only.
PRODUCT_PACKAGES_DEBUG += \
    bootctl \
    r.vendor \
    update_engine_client

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.full.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.full.xml\
    frameworks/native/data/etc/android.hardware.camera.raw.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.raw.xml\
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.barometer.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.barometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepcounter.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepcounter.xml \
    frameworks/native/data/etc/android.hardware.sensor.stepdetector.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.stepdetector.xml \
    frameworks/native/data/etc/android.hardware.sensor.hifi_sensors.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.hifi_sensors.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.telephony.ims.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.ims.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.wifi.aware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.aware.xml \
    frameworks/native/data/etc/android.hardware.wifi.passpoint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.passpoint.xml \
    frameworks/native/data/etc/android.hardware.wifi.rtt.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.rtt.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.xml \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.xml \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.hcef.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hcef.xml \
    frameworks/native/data/etc/com.nxp.mifare.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/com.nxp.mifare.xml \
    frameworks/native/data/etc/android.hardware.vulkan.level-1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.level.xml \
    frameworks/native/data/etc/android.hardware.vulkan.compute-0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.compute.xml \
    frameworks/native/data/etc/android.hardware.vulkan.version-1_1.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.vulkan.version.xml \
    frameworks/native/data/etc/android.hardware.telephony.carrierlock.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.carrierlock.xml \
    frameworks/native/data/etc/android.hardware.strongbox_keystore.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.strongbox_keystore.xml \
    frameworks/native/data/etc/android.hardware.nfc.uicc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.uicc.xml \
    frameworks/native/data/etc/android.hardware.nfc.ese.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.ese.xml

# Audio fluence, ns, aec property, voice and media volume steps
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.audio.sdk.fluencetype=fluencepro \
    persist.audio.fluence.voicecall=true \
    persist.audio.fluence.speaker=true \
    persist.audio.fluence.voicecomm=true \
    persist.audio.fluence.voicerec=false \
    ro.config.vc_call_vol_steps=7 \
    ro.config.media_vol_steps=25 \

# Audio Features
PRODUCT_PROPERTY_OVERRIDES += \
    vendor.audio.feature.external_dsp.enable=true \
    vendor.audio.feature.external_speaker.enable=true \
    vendor.audio.feature.concurrent_capture.enable=false \
    vendor.audio.feature.a2dp_offload.enable=true \
    vendor.audio.feature.hfp.enable=true \
    vendor.audio.feature.hwdep_cal.enable=true \
    vendor.audio.feature.incall_music.enable=true \
    vendor.audio.feature.maxx_audio.enable=true \
    vendor.audio.feature.spkr_prot.enable=true \
    vendor.audio.feature.usb_offload.enable=true \
    vendor.audio.feature.audiozoom.enable=true \
    vendor.audio.feature.snd_mon.enable=true \
    vendor.audio.capture.enforce_legacy_copp_sr=true \
    vendor.audio.offload.buffer.size.kb=256 \
    persist.vendor.audio_hal.dsp_bit_width_enforce_mode=24 \

# MaxxAudio effect and add rotation monitor
PRODUCT_PROPERTY_OVERRIDES += \
    ro.audio.monitorRotation=true

# Iaxxx streming and factory binary
PRODUCT_PACKAGES += \
    libtunnel \
    libodsp \
    adnc_strm.primary.default

# graphics
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=196610

PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.display.foss=1 \
    ro.vendor.display.paneltype=2 \
    ro.vendor.display.sensortype=2 \
    vendor.display.foss.config=1 \
    vendor.display.foss.config_path=/vendor/etc/FOSSConfig.xml \
    vendor.display.qdcm.mode_combine=1

# camera hal buffer management
PRODUCT_PROPERTY_OVERRIDES += \
    persist.camera.managebuffer.enable=1

# camera google face detection
PRODUCT_PROPERTY_OVERRIDES += \
    persist.camera.googfd.enable=1

# Lets the vendor library that Google Camera HWL is enabled
PRODUCT_PROPERTY_OVERRIDES += \
    persist.camera.google_hwl.enabled=true \
    persist.camera.google_hwl.name=libgooglecamerahwl_impl.so

# camera common HWL
CAMERA_COMMON_HWL := true

# OEM Unlock reporting
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.oem_unlock_supported=1

PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.cne.feature=1 \
    persist.vendor.data.iwlan.enable=true \
    persist.vendor.radio.RATE_ADAPT_ENABLE=1 \
    persist.vendor.radio.ROTATION_ENABLE=1 \
    persist.vendor.radio.VT_ENABLE=1 \
    persist.vendor.radio.VT_HYBRID_ENABLE=1 \
    persist.vendor.radio.apm_sim_not_pwdn=1 \
    persist.vendor.radio.custom_ecc=1 \
    persist.vendor.radio.data_ltd_sys_ind=1 \
    persist.vendor.radio.videopause.mode=1 \
    persist.vendor.radio.multisim_switch_support=true \
    persist.vendor.radio.sib16_support=1 \
    persist.vendor.radio.data_con_rprt=true \
    persist.vendor.radio.relay_oprt_change=1 \
    persist.vendor.radio.no_wait_for_card=1 \
    persist.vendor.radio.sap_silent_pin=1 \
    persist.rcs.supported=1 \
    vendor.rild.libpath=/vendor/lib64/libril-qc-hal-qmi.so \
    ro.hardware.keystore_desede=true \
    ro.telephony.iwlan_operation_mode=legacy \
    persist.vendor.radio.procedure_bytes=SKIP \

# Disable snapshot timer
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.radio.snapshot_enabled=0 \
    persist.vendor.radio.snapshot_timer=0

# RenderScript HAL
PRODUCT_PACKAGES += \
    android.hardware.renderscript@1.0-impl

# Light HAL
PRODUCT_PACKAGES += \
    lights.sm6150 \
    hardware.google.light@1.1-service

# Memtrack HAL
PRODUCT_PACKAGES += \
    memtrack.sm6150 \
    android.hardware.memtrack@1.0-impl \
    android.hardware.memtrack@1.0-service

# Bluetooth HAL
PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.0-impl-qti \
    android.hardware.bluetooth@1.0-service-qti

#Bluetooth SAR HAL
PRODUCT_PACKAGES += \
    vendor.qti.hardware.bluetooth_sar@1.0-impl

# Bluetooth SoC
PRODUCT_PROPERTY_OVERRIDES += \
    vendor.qcom.bluetooth.soc=cherokee

# Property for loading BDA from device tree
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.bt.bdaddr_path=/proc/device-tree/chosen/cdt/cdb2/bt_addr

# Bluetooth WiPower
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.bluetooth.emb_wp_mode=false \
    ro.vendor.bluetooth.wipower=false

# DRM HAL
PRODUCT_PACKAGES += \
    android.hardware.drm@1.0-impl \
    android.hardware.drm@1.0-service \
    android.hardware.drm@1.2-service.clearkey \
    android.hardware.drm@1.2-service.widevine

# NFC and Secure Element packages
PRODUCT_PACKAGES += \
    NfcNci \
    Tag \
    SecureElement \
    android.hardware.nfc@1.2-service.st \
    android.hardware.secure_element@1.0-service.st

PRODUCT_COPY_FILES += \
    device/google/sunfish/nfc/libnfc-hal-st.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-hal-st.conf \
    device/google/sunfish/nfc/libese-hal-st.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libese-hal-st.conf \
    device/google/sunfish/nfc/libnfc-nci.conf:$(TARGET_COPY_OUT_PRODUCT)/etc/libnfc-nci.conf

PRODUCT_PACKAGES += \
    android.hardware.usb@1.2-service.sunfish

PRODUCT_PACKAGES += \
    android.hardware.health@2.0-service.sunfish

# Storage health HAL
PRODUCT_PACKAGES += \
    android.hardware.health.storage@1.0-service

PRODUCT_PACKAGES += \
    libmm-omxcore \
    libOmxCore \
    libstagefrighthw \
    libOmxVdec \
    libOmxVdecHevc \
    libOmxVenc \
    libc2dcolorconvert

# Enable Codec 2.0
PRODUCT_PACKAGES += \
    libqcodec2 \
    vendor.qti.media.c2@1.0-service \
    media_codecs_c2.xml \
    codec2.vendor.ext.policy \
    codec2.vendor.base.policy

PRODUCT_PROPERTY_OVERRIDES += \
    debug.stagefright.omx_default_rank=512

# Create input surface on the framework side
PRODUCT_PROPERTY_OVERRIDES += \
    debug.stagefright.c2inputsurface=-1 \

# Disable OMX
PRODUCT_PROPERTY_OVERRIDES += \
    vendor.media.omx=0 \

# Enable ECO service
QC2_HAVE_ECO_SERVICE := true

PRODUCT_PROPERTY_OVERRIDES += \
    vendor.qc2.venc.avgqp.enable=1

PRODUCT_PACKAGES += \
    android.hardware.camera.provider@2.4-impl-google \
    android.hardware.camera.provider@2.4-service-google \
    camera.sm6150 \
    libgooglecamerahal \
    libgooglecamerahwl_impl \
    libgoogle_camera_hal_tests \
    libqomx_core \
    libmmjpeg_interface \
    libmmcamera_interface \
    libcameradepthcalibrator

PRODUCT_PACKAGES += \
    sensors.$(PRODUCT_HARDWARE) \
    android.hardware.sensors@2.0-impl \
    android.hardware.sensors@2.0-service \
    android.hardware.sensors@2.0-service.rc

PRODUCT_PACKAGES += \
    fs_config_dirs \
    fs_config_files

# Context hub HAL
PRODUCT_PACKAGES += \
    android.hardware.contexthub@1.0-impl.generic \
    android.hardware.contexthub@1.0-service

# Boot control HAL
PRODUCT_PACKAGES += \
    android.hardware.boot@1.0-impl \
    android.hardware.boot@1.0-impl.recovery \
    android.hardware.boot@1.0-service \

# Vibrator HAL
PRODUCT_PACKAGES += \
    android.hardware.vibrator@1.3-service.sunfish \

# DRV2624 Haptics Waveform
PRODUCT_COPY_FILES += \
    device/google/sunfish/vibrator/drv2624/drv2624.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/drv2624.bin

# Vibrator HAL
PRODUCT_PRODUCT_PROPERTIES +=\
    ro.vibrator.hal.config.dynamic=1 \
    ro.vibrator.hal.click.duration=6 \
    ro.vibrator.hal.tick.duration=6 \
    ro.vibrator.hal.heavyclick.duration=6 \
    ro.vibrator.hal.short.voltage=161 \
    ro.vibrator.hal.long.voltage=161 \
    ro.vibrator.hal.long.frequency.shift=10 \
    ro.vibrator.hal.steady.shape=1 \
    ro.vibrator.hal.lptrigger=0

# Thermal HAL
PRODUCT_PACKAGES += \
    android.hardware.thermal@2.0-service.pixel \

# Thermal HAL config
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/thermal_info_config_$(PRODUCT_HARDWARE).json:$(TARGET_COPY_OUT_VENDOR)/etc/thermal_info_config.json \

#GNSS HAL
PRODUCT_PACKAGES += \
    libgps.utils \
    libgnss \
    libloc_core \
    liblocation_api \
    libbatching \
    libgeofencing \
    android.hardware.gnss@2.0-impl-qti \
    android.hardware.gnss@2.0-service-qti

# Wireless Charger HAL
PRODUCT_PACKAGES += \
    vendor.google.wireless_charger@1.0

ENABLE_VENDOR_RIL_SERVICE := true

HOSTAPD := hostapd
HOSTAPD += hostapd_cli
PRODUCT_PACKAGES += $(HOSTAPD)

WPA := wpa_supplicant.conf
WPA += wpa_supplicant_wcn.conf
WPA += wpa_supplicant
PRODUCT_PACKAGES += $(WPA)

ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_PACKAGES += wpa_cli
endif

# Wifi
PRODUCT_PACKAGES += \
    android.hardware.wifi@1.0-service \
    wificond \
    libwpa_client

# WLAN driver configuration files
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf \
    $(LOCAL_PATH)/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf \
    $(LOCAL_PATH)/wifi_concurrency_cfg.txt:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wifi_concurrency_cfg.txt \
    $(LOCAL_PATH)/WCNSS_qcom_cfg.ini:$(TARGET_COPY_OUT_VENDOR)/firmware/wlan/qca_cld/WCNSS_qcom_cfg.ini \

LIB_NL := libnl_2
PRODUCT_PACKAGES += $(LIB_NL)

# Factory OTA
PRODUCT_PACKAGES += \
    FactoryOta

# Audio effects
PRODUCT_PACKAGES += \
    libvolumelistener \
    libqcomvisualizer \
    libqcomvoiceprocessing \
    libqcomvoiceprocessingdescriptors \
    libqcompostprocbundle

PRODUCT_PACKAGES += \
    audio.primary.sm6150 \
    audio.a2dp.default \
    audio.usb.default \
    audio.r_submix.default \
    libaudio-resampler \
    audio.hearing_aid.default \
    audio.bluetooth.default

PRODUCT_PACKAGES += \
    android.hardware.audio@5.0-impl:32 \
    android.hardware.audio.effect@5.0-impl:32 \
    android.hardware.broadcastradio@1.0-impl \
    android.hardware.soundtrigger@2.2-impl \
    android.hardware.bluetooth.audio@2.0-impl \
    android.hardware.audio@2.0-service

# Modules for Audio HAL
PRODUCT_PACKAGES += \
    libcirrusspkrprot \
    libsndmonitor \
    libmalistener \
    liba2dpoffload \
    btaudio_offload_if \
    libmaxxaudio \
    libaudiozoom

ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_PACKAGES += \
    tinyplay \
    tinycap \
    tinymix \
    tinypcminfo \
    cplay
endif

# Audio hal xmls

# Audio Policy tables

# Audio ACDB data

# Audio ACDB workspace files for QACT

# Audio speaker tunning config data

# Audio audiozoom config data

# and ensure that the xaac decoder is built
PRODUCT_PACKAGES += \
    libstagefright_soft_xaacdec.vendor

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    $(LOCAL_PATH)/media_codecs_omx.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_omx.xml \
    $(LOCAL_PATH)/media_codecs_performance_c2.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance_c2.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml \
    $(LOCAL_PATH)/media_profiles_V1_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_V1_0.xml

# Vendor seccomp policy files for media components:
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/seccomp_policy/mediacodec.policy:$(TARGET_COPY_OUT_VENDOR)/etc/seccomp_policy/mediacodec.policy

PRODUCT_PROPERTY_OVERRIDES += \
    audio.snd_card.open.retries=50


ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
# Subsystem ramdump
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.sys.ssr.enable_ramdumps=1
endif

# Subsystem silent restart
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.sys.ssr.restart_level=modem,adsp,slpi

ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
# Sensor debug flag
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.debug.ash.logger=0 \
    persist.vendor.debug.ash.logger.time=0
endif

# setup dalvik vm configs
$(call inherit-product, frameworks/native/build/phone-xhdpi-2048-dalvik-heap.mk)

# Use the default charger mode images
PRODUCT_PACKAGES += \
    charger_res_images

ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
# b/36703476: Set default log size to 1M
PRODUCT_PROPERTY_OVERRIDES += \
  ro.logd.size=1M
# b/114766334: persist all logs by default rotating on 30 files of 1MiB
PRODUCT_PROPERTY_OVERRIDES += \
  logd.logpersistd=logcatd \
  logd.logpersistd.size=30
endif

# Dumpstate HAL
PRODUCT_PACKAGES += \
    android.hardware.dumpstate@1.0-service.sunfish

# Citadel
#
# Set CITADEL_LAZY_PSK_SYNC to true on projects with faceauth, otherwise false.
#
#      EVT devices left the factory without being provisioned,
#      and thus the shared authtoken key is yet to be established.
#      Since faceauth HAT enforcement fails without the preshared
#      authtoken, auto-sync it in the field for userdebug/eng.
#      Please refer to b/135295587 for more detail.
#
CITADEL_LAZY_PSK_SYNC := false

PRODUCT_PACKAGES += \
    citadeld \
    citadel_updater \
    android.hardware.authsecret@1.0-service.citadel \
    android.hardware.oemlock@1.0-service.citadel \
    android.hardware.weaver@1.0-service.citadel \
    android.hardware.keymaster@4.0-service.citadel \
    wait_for_strongbox

# Citadel debug stuff
PRODUCT_PACKAGES_DEBUG += \
    test_citadel

# Storage: for factory reset protection feature
PRODUCT_PROPERTY_OVERRIDES += \
    ro.frp.pst=/dev/block/bootdevice/by-name/frp

PRODUCT_PACKAGES += \
    vndk-sp

PRODUCT_ENFORCE_RRO_TARGETS := *

# Override heap growth limit due to high display density on device
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapgrowthlimit=256m

PRODUCT_PACKAGES += \
    ipacm \
    IPACM_cfg.xml

#Set default CDMA subscription to RUIM
PRODUCT_PROPERTY_OVERRIDES += \
    ro.telephony.default_cdma_sub=0

# Set display color mode to Adaptive by default
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.sf.color_saturation=1.0 \
    persist.sys.sf.native_mode=2 \
    persist.sys.sf.color_mode=9

# Keymaster configuration
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.device_id_attestation.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.device_id_attestation.xml

# Enable modem logging
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.radio.log_loc="/data/vendor/modem_dump" \
    ro.vendor.radio.log_prefix="modem_log_"

# Enable modem logging for debug
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.sys.modem.diag.mdlog=true \
    persist.vendor.sys.modem.diag.mdlog_br_num=5
else
endif

# Preopt SystemUI
PRODUCT_DEXPREOPT_SPEED_APPS += \
    SystemUIGoogle

# Enable stats logging in LMKD
TARGET_LMKD_STATS_LOG := true
PRODUCT_PRODUCT_PROPERTIES += \
    ro.lmk.log_stats=true

# default usb oem functions
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
  PRODUCT_PROPERTY_OVERRIDES += \
      persist.vendor.usb.usbradio.config=diag
endif

# Default app/sf phase offset
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.surface_flinger.vsync_event_phase_offset_ns=2000000
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.surface_flinger.vsync_sf_event_phase_offset_ns=6000000

# Early phase offset configuration for SurfaceFlinger (b/75985430)
PRODUCT_PROPERTY_OVERRIDES += \
    debug.sf.early_phase_offset_ns=11600000
PRODUCT_PROPERTY_OVERRIDES += \
    debug.sf.early_app_phase_offset_ns=11600000
PRODUCT_PROPERTY_OVERRIDES += \
    debug.sf.early_gl_phase_offset_ns=3000000
PRODUCT_PROPERTY_OVERRIDES += \
    debug.sf.early_gl_app_phase_offset_ns=15000000
PRODUCT_PROPERTY_OVERRIDES += \
    debug.sf.phase_offset_threshold_for_next_vsync_ns=11600000

# Enable backpressure for GL comp
PRODUCT_PROPERTY_OVERRIDES += \
    debug.sf.enable_gl_backpressure=1

# Do not skip init trigger by default
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    vendor.skip.init=0

BOARD_USES_QCNE := true

#per device
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/sunfish/init.sunfish.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.sunfish.rc

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/sec_config:$(TARGET_COPY_OUT_VENDOR)/etc/sec_config

# power HAL
PRODUCT_PACKAGES += \
    android.hardware.power@1.3-service.pixel-libperfmgr

# GPS configuration file
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/gps.conf:$(TARGET_COPY_OUT_VENDOR)/etc/gps.conf

# default atrace HAL
PRODUCT_PACKAGES += \
    android.hardware.atrace@1.0-service

# Fingerprint
PRODUCT_PACKAGES += \
    android.hardware.biometrics.fingerprint@2.1-service.fpc
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.fingerprint.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.fingerprint.xml

# Reliability reporting
PRODUCT_PACKAGES += \
    pixelstats-vendor

PRODUCT_USE_DYNAMIC_PARTITIONS := true

# fastbootd
PRODUCT_PACKAGES += \
    android.hardware.fastboot@1.0-impl.pixel \
    fastbootd

# insmod files
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/init.insmod.sunfish.cfg:$(TARGET_COPY_OUT_VENDOR)/etc/init.insmod.sunfish.cfg

# Use /product/etc/fstab.postinstall to mount system_other
PRODUCT_PRODUCT_PROPERTIES += \
    ro.postinstall.fstab.prefix=/product

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/fstab.postinstall:$(TARGET_COPY_OUT_PRODUCT)/etc/fstab.postinstall

# powerstats HAL
PRODUCT_PACKAGES += \
    android.hardware.power.stats@1.0-service.pixel

# Recovery
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/init.recovery.device.rc:recovery/root/init.recovery.sunfish.rc

# Setup Wizard gesture settings
PRODUCT_PRODUCT_PROPERTIES += \
    setupwizard.enable_assist_gesture_training=false

QTI_TELEPHONY_UTILS := qti-telephony-utils
QTI_TELEPHONY_UTILS += qti_telephony_utils.xml
PRODUCT_PACKAGES += $(QTI_TELEPHONY_UTILS)

HIDL_WRAPPER := qti-telephony-hidl-wrapper
HIDL_WRAPPER += qti_telephony_hidl_wrapper.xml
PRODUCT_PACKAGES += $(HIDL_WRAPPER)

# Increment the SVN for any official public releases
PRODUCT_PROPERTY_OVERRIDES += \
	ro.vendor.build.svn=4

# ZRAM writeback
PRODUCT_PROPERTY_OVERRIDES += \
    ro.zram.mark_idle_delay_mins=60 \
    ro.zram.first_wb_delay_mins=180 \
    ro.zram.periodic_wb_delay_hours=24

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/powerhint.json:$(TARGET_COPY_OUT_VENDOR)/etc/powerhint.json

-include hardware/qcom/sm7150/display/config/display-product.mk
-include vendor/qcom/sm7150/proprietary/display/config/display-product-proprietary.mk
-include vendor/qcom/sm7150/proprietary/commonsys-intf/data/data_commonsys-intf_system_product.mk
-include vendor/qcom/sm7150/proprietary/commonsys-intf/data/data_commonsys-intf_vendor_product.mk
# Security
-include vendor/qcom/sm7150/proprietary/securemsm/config/keymaster_vendor_proprietary_board.mk
-include vendor/qcom/sm7150/proprietary/securemsm/config/keymaster_vendor_proprietary_product.mk
-include vendor/qcom/sm7150/proprietary/securemsm/config/cpz_vendor_proprietary_board.mk
-include vendor/qcom/sm7150/proprietary/securemsm/config/cpz_vendor_proprietary_product.mk
-include vendor/qcom/sm7150/proprietary/securemsm/config/smcinvoke_vendor_proprietary_product.mk
-include vendor/qcom/sm7150/proprietary/commonsys/securemsm/securemsm_system_product.mk
