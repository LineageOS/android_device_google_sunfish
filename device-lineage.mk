#
# Copyright (C) 2020-2021 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Overlays
DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/overlay-lineage

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += \
    vendor/qcom/opensource/commonsys-intf/display \
    vendor/qcom/opensource/display

# Add common definitions for Qualcomm
$(call inherit-product, hardware/qcom-caf/common/common.mk)

# AiAi Config
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/allowlist_com.google.android.as.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/sysconfig/allowlist_com.google.android.as.xml

# Camera
PRODUCT_PRODUCT_PROPERTIES += \
    ro.vendor.camera.extensions.package=com.google.android.apps.camera.services \
    ro.vendor.camera.extensions.service=com.google.android.apps.camera.services.extensions.service.PixelExtensions

# DebugFS
PRODUCT_SET_DEBUGFS_RESTRICTIONS := true

# EUICC
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.euicc.xml:$(TARGET_COPY_OUT_PRODUCT)/etc/permissions/android.hardware.telephony.euicc.xml

# Google Assistant
PRODUCT_PRODUCT_PROPERTIES += ro.opa.eligible_device=true

# HBM
PRODUCT_PACKAGES += \
    HbmSVManagerOverlay

# Lineage Health
include hardware/google/pixel/lineage_health/device.mk

# LiveDisplay
PRODUCT_PACKAGES += \
    vendor.lineage.livedisplay@2.0-service-sdm

# Parts
PRODUCT_PACKAGES += \
    GoogleParts

# RCS
PRODUCT_PACKAGES += \
    PresencePolling \
    RcsService

# Touch
include hardware/google/pixel/touch/device.mk

# Build necessary packages for vendor

# CHRE
PRODUCT_PACKAGES += \
    chre

# Configstore
PRODUCT_PACKAGES += \
    disable_configstore

# GPS
PRODUCT_PACKAGES += \
    flp.conf

# Identity credential
PRODUCT_PACKAGES += \
    android.hardware.identity_credential.xml

# VNDK FWK detect
PRODUCT_PACKAGES += \
    libvndfwk_detect_jni.qti.vendor

# Wi-Fi
PRODUCT_PACKAGES += \
    libwifi-hal-ctrl:64

# Properties
TARGET_VENDOR_PROP := $(LOCAL_PATH)/vendor.prop
