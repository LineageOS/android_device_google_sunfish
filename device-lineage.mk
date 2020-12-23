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

# Overlays
DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/overlay-lineage

# EUICC
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.euicc.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/android.hardware.telephony.euicc.xml

# Properties
TARGET_VENDOR_PROP := $(LOCAL_PATH)/vendor.prop

# Build necessary packages for vendor
PRODUCT_PACKAGES += \
    libdisplayconfig \
    libhidltransport.vendor \
    libhwbinder.vendor \
    libjson \
    libtinycompress.vendor \
    libtinyxml \
    libwifi-hal-qcom \
    vendor.qti.hardware.display.allocator@3.0.vendor \
    vendor.qti.hardware.display.mapper@2.0.vendor \
    vendor.qti.hardware.display.mapper@3.0.vendor \
    vendor.qti.hardware.display.mapperextensions@1.0.vendor \
    vendor.qti.hardware.display.mapperextensions@1.1.vendor \

# Partitions
AB_OTA_PARTITIONS += \
    vendor \

# Vendor Security Patch Level
VENDOR_SECURITY_PATCH := "2020-12-05"
