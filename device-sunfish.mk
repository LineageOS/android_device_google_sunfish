#
# Copyright 2019 The Android Open Source Project
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

PRODUCT_HARDWARE := sunfish

include device/google/sunfish/device-common.mk

DEVICE_PACKAGE_OVERLAYS += device/google/sunfish/sunfish/overlay

# Audio XMLs for sunfish

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/mixer_paths_bolero_snd.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_bolero_snd.xml \
    $(LOCAL_PATH)/audio/audio_platform_info_bolero_snd.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_platform_info_bolero_snd.xml \
    $(LOCAL_PATH)/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml \
    $(LOCAL_PATH)/audio_policy_configuration_a2dp_offload_disabled.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration_a2dp_offload_disabled.xml \
    $(LOCAL_PATH)/audio_policy_configuration_bluetooth_legacy_hal.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration_bluetooth_legacy_hal.xml \
    $(LOCAL_PATH)/bluetooth_hearing_aid_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_hearing_aid_audio_policy_configuration.xml \
    $(LOCAL_PATH)/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/a2dp_in_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_in_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/bluetooth_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/bluetooth_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/hearing_aid_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/hearing_aid_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml

# RT5514 SoundTrigger
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/rt5514/rt5514_dsp_fw1.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/rt5514_dsp_fw1.bin \
    $(LOCAL_PATH)/audio/rt5514/rt5514_dsp_fw2.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/rt5514_dsp_fw2.bin \
    $(LOCAL_PATH)/audio/rt5514/rt5514_dsp_fw3.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/rt5514_dsp_fw3.bin \
    $(LOCAL_PATH)/audio/rt5514/rt5514_dsp_fw4.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/rt5514_dsp_fw4.bin

# CS35L41 SPEAKER AMP
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/audio/cs35l41/cs35l41-dsp1-spk-cali.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/cs35l41-dsp1-spk-cali.bin \
    $(LOCAL_PATH)/audio/cs35l41/cs35l41-dsp1-spk-cali.wmfw:$(TARGET_COPY_OUT_VENDOR)/firmware/cs35l41-dsp1-spk-cali.wmfw \
    $(LOCAL_PATH)/audio/cs35l41/R-cs35l41-dsp1-spk-cali.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/R-cs35l41-dsp1-spk-cali.bin \
    $(LOCAL_PATH)/audio/cs35l41/cs35l41-dsp1-spk-prot.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/cs35l41-dsp1-spk-prot.bin \
    $(LOCAL_PATH)/audio/cs35l41/cs35l41-dsp1-spk-prot.wmfw:$(TARGET_COPY_OUT_VENDOR)/firmware/cs35l41-dsp1-spk-prot.wmfw \
    $(LOCAL_PATH)/audio/cs35l41/R-cs35l41-dsp1-spk-prot.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/R-cs35l41-dsp1-spk-prot.bin

# Audio effects
PRODUCT_PACKAGES += \
    libqcomvoiceprocessingdescriptors

# Audio ACDB data
ifeq ($(wildcard vendor/google_cei/factory/prebuilt/ftm.mk),)
PRODUCT_COPY_FILES += \
     $(LOCAL_PATH)/audio/acdbdata/Bluetooth_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/Bluetooth_cal.acdb \
     $(LOCAL_PATH)/audio/acdbdata/General_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/General_cal.acdb \
     $(LOCAL_PATH)/audio/acdbdata/Global_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/Global_cal.acdb \
     $(LOCAL_PATH)/audio/acdbdata/Handset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/Handset_cal.acdb \
     $(LOCAL_PATH)/audio/acdbdata/Hdmi_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/Hdmi_cal.acdb \
     $(LOCAL_PATH)/audio/acdbdata/Headset_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/Headset_cal.acdb \
     $(LOCAL_PATH)/audio/acdbdata/Speaker_cal.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/Speaker_cal.acdb \
     $(LOCAL_PATH)/audio/acdbdata/adsp_avs_config.acdb:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/adsp_avs_config.acdb

# Audio ACDB workspace files for QACT
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_COPY_FILES += \
     $(LOCAL_PATH)/audio/acdbdata/workspaceFile.qwsp:$(TARGET_COPY_OUT_VENDOR)/etc/acdbdata/workspaceFile.qwsp
endif
endif

# Fingerprint HIDL
include device/google/sunfish/fingerprint.mk
