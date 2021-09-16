#
# Copyright (C) 2020 The LineageOS Project
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

# Camera
PRODUCT_PACKAGES += \
    Snap \

# Display
PRODUCT_SOONG_NAMESPACES += \
    vendor/qcom/opensource/commonsys-intf/display \

# LiveDisplay
PRODUCT_PACKAGES += \
    vendor.lineage.livedisplay@2.0-service-sdm \

# Parts
PRODUCT_PACKAGES += \
    GoogleParts \

# Touch
PRODUCT_PACKAGES += \
    vendor.lineage.touch@1.0-service.pixel \

# Trust HAL
PRODUCT_PACKAGES += \
    vendor.lineage.trust@1.0-service \

# Overlays
DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/overlay-lineage

# EUICC
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.euicc.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/android.hardware.telephony.euicc.xml

# Google Assistant
PRODUCT_PRODUCT_PROPERTIES += ro.opa.eligible_device=true

# Properties
TARGET_VENDOR_PROP := $(LOCAL_PATH)/vendor.prop

# Build necessary packages for system_ext
PRODUCT_PACKAGES += \
    com.qualcomm.qti.bluetooth_audio@1.0 \
    qti-telephony-utils \
    vendor.qti.hardware.cryptfshw@1.0 \
    vendor.display.config@1.8 \

# Build necessary packages for vendor
PRODUCT_PACKAGES += \
    android.hardware.atrace@1.0-service.pixel \
    android.hardware.camera.provider@2.6-service-google \
    android.hardware.sensors@2.0-ScopedWakelock.vendor \
    android.hardware.identity-support-lib.vendor \
    chre \
    ese_spi_st \
    hardware.google.light@1.0.vendor:32 \
    libavservices_minijail_vendor:32 \
    libcodec2_hidl@1.0.vendor:32 \
    libcodec2_vndk.vendor \
    libcppbor.vendor \
    libdisplayconfig \
    libdisplayconfig.qti.vendor \
    libhidltransport.vendor \
    libhwbinder.vendor \
    libjson \
    libmedia_ecoservice.vendor \
    libnos:64 \
    libnosprotos:64 \
    libnos_client_citadel:64 \
    libnos_datagram:64 \
    libnos_datagram_citadel:64 \
    libnos_transport:64 \
    libprotobuf-cpp-full-vendorcompat \
    libqcomvoiceprocessingdescriptors \
    libqti_vndfwk_detect.vendor \
    libstagefright_bufferpool@2.0.1.vendor \
    libteeui_hal_support.vendor \
    libtinycompress \
    libtinyxml \
    libvndfwk_detect_jni.qti.vendor \
    libwifi-hal \
    libwifi-hal-qcom \
    memtrack.sm6150 \
    nfc_nci.st21nfc.default \
    nos_app_avb:64 \
    nos_app_identity:64 \
    nos_app_keymaster:64 \
    nos_app_weaver:64 \
    pixelatoms-cpp \
    vendor.display.config@1.10.vendor \
    vendor.display.config@1.11.vendor \
    vendor.display.config@2.0.vendor \
    vendor.qti.hardware.bluetooth_audio@2.0.vendor \
    vendor.qti.hardware.capabilityconfigstore@1.0.vendor \
    vendor.qti.hardware.cryptfshw@1.0.vendor \
    vendor.qti.hardware.display.allocator@3.0.vendor \
    vendor.qti.hardware.display.mapper@2.0.vendor \
    vendor.qti.hardware.display.mapper@3.0.vendor \
    vendor.qti.hardware.display.mapperextensions@1.0.vendor \
    vendor.qti.hardware.display.mapperextensions@1.1.vendor \
    vendor.qti.hardware.perf@1.0.vendor \
    vendor.qti.hardware.perf@2.0.vendor

# Vendor Security Patch Level
VENDOR_SECURITY_PATCH := "2021-09-05"
