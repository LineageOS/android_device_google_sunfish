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

## Kernel
BOARD_KERNEL_IMAGE_NAME := Image.lz4
KERNEL_LD := LD=ld.lld
TARGET_COMPILE_WITH_MSM_KERNEL := true
TARGET_KERNEL_ARCH := arm64
TARGET_KERNEL_CLANG_COMPILE := true
TARGET_KERNEL_CONFIG := sunfish_defconfig
TARGET_KERNEL_SOURCE := kernel/google/sunfish
TARGET_NEEDS_DTBOIMAGE := true

# Manifests
DEVICE_MANIFEST_FILE += device/google/sunfish/lineage_manifest.xml
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE += device/google/sunfish/lineage_compatibility_matrix.xml

# Partitions
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
AB_OTA_PARTITIONS += \
    vendor \

# Reserve space for gapps install
ifneq ($(WITH_GMS),true)
BOARD_PRODUCTIMAGE_EXTFS_INODE_COUNT := -1
BOARD_PRODUCTIMAGE_PARTITION_RESERVED_SIZE := 408944640
BOARD_SYSTEMIMAGE_EXTFS_INODE_COUNT := -1
BOARD_SYSTEMIMAGE_PARTITION_RESERVED_SIZE := 1761607680
endif
BOARD_SYSTEM_EXTIMAGE_PARTITION_RESERVED_SIZE := 30720000
BOARD_VENDORIMAGE_PARTITION_RESERVED_SIZE := 30720000

# SELinux
BOARD_SEPOLICY_DIRS += device/google/sunfish/sepolicy-lineage/dynamic
BOARD_SEPOLICY_DIRS += device/google/sunfish/sepolicy-lineage/vendor

# Verified Boot
BOARD_AVB_MAKE_VBMETA_IMAGE_ARGS += --flags 3

-include vendor/google/sunfish/BoardConfigVendor.mk
