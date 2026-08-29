#
# Copyright (C) 2020-2021 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

# Kernel
BOARD_KERNEL_IMAGE_NAME := Image.lz4
TARGET_COMPILE_WITH_MSM_KERNEL := true
TARGET_KERNEL_CONFIG := sunfish_defconfig
TARGET_KERNEL_SOURCE := kernel/google/msm-4.14
TARGET_NEEDS_DTBOIMAGE := true

# Partitions
AB_OTA_PARTITIONS += \
    vendor
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4

# Reserve space for gapps install
-include vendor/lineage/config/BoardConfigReservedSize.mk

# SELinux
BOARD_SEPOLICY_DIRS += device/google/sunfish/sepolicy-lineage/dynamic
BOARD_SEPOLICY_DIRS += device/google/sunfish/sepolicy-lineage/vendor

# Verified Boot
ifneq ($(WITH_AVB),true)
BOARD_AVB_MAKE_VBMETA_IMAGE_ARGS += --flags 3
endif

include vendor/google/sunfish/BoardConfigVendor.mk
