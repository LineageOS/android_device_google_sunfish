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

# If you don't need to do a full clean build but would like to touch
# a file or delete some intermediate files, add a clean step to the end
# of the list.  These steps will only be run once, if they haven't been
# run before.
#
# E.g.:
#     $(call add-clean-step, touch -c external/sqlite/sqlite3.h)
#     $(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/STATIC_LIBRARIES/libz_intermediates)
#
# Always use "touch -c" and "rm -f" or "rm -rf" to gracefully deal with
# files that are missing or have been moved.
#
# Use $(PRODUCT_OUT) to get to the "out/target/product/blah/" directory.
# Use $(OUT_DIR) to refer to the "out" directory.
#
# If you need to re-do something that's already mentioned, just copy
# the command and add it to the bottom of the list.  E.g., if a change
# that you made last week required touching a file and a change you
# made today requires touching the same file, just copy the old
# touch step and add it to the end of the list.
#
# ************************************************
# NEWER CLEAN STEPS MUST BE AT THE END OF THE LIST
# ************************************************

# For example:
#$(call add-clean-step, rm -rf $(OUT_DIR)/target/common/obj/APPS/AndroidTests_intermediates)
#$(call add-clean-step, rm -rf $(OUT_DIR)/target/common/obj/JAVA_LIBRARIES/core_intermediates)
#$(call add-clean-step, find $(OUT_DIR) -type f -name "IGTalkSession*" -print0 | xargs -0 rm -f)
#$(call add-clean-step, rm -rf $(PRODUCT_OUT)/data/*)

# Remove default android.hardware.health@2.0-service
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/bin/hw/android.hardware.health@2.0-service)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/init/android.hardware.health@2.0-service.rc)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/bin/hw/android.hardware.health@2.0-service.sunfish)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/init/android.hardware.health@2.0-service.sunfish.rc)

# Remove health HAL 2.1
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/bin/hw/android.hardware.health@2.1-service)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/init/android.hardware.health@2.1-service.rc)

# Remove healthd
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/bin/healthd)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/etc/init/healthd.rc)

# Move libnfc-nci.conf to /vendor
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/etc/libnfc-nci.conf)

# Remove /firmware which used to be a symlink to /vendor/firmware_mnt
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/root/firmware)

# Remove thermalHAL 1.0
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/init/android.hardware.thermal@1.0-service.sunfish.rc)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/bin/hw/android.hardware.thermal@1.0-service.sunfish)

# Remove default android.hardware.composer@2.2
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/bin/hw/android.hardware.graphics.composer@2.2-service)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/init/android.hardware.graphics.composer@2.2-service.rc)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/lib64/hw/android.hardware.graphics.composer@2.2-impl.so)

# Remove default android.hardware.graphics.composer@2.3-service
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/bin/hw/android.hardware.graphics.composer@2.3-service)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/init/android.hardware.graphics.composer@2.3-service.rc)

# Remove android.hardware.graphics.composer@2.3-service-sm7150
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/bin/hw/android.hardware.graphics.composer@2.3-service-sm7150)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/init/android.hardware.graphics.composer@2.3-service-sm7150.rc)

# Remove super_empty.img
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/super_empty.img)

# Remove Vibrator HAL 1.2
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/init/android.hardware.vibrator@1.2-service.sunfish.rc)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/bin/hw/android.hardware.vibrator@1.2-service.sunfish)

# Remove Misnamed Vibrator VINTF Fragment
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/vintf/manifest/manifest.xml)

# Rename power HAL
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/init/android.hardware.power@1.3-service.sunfish-libperfmgr.rc)

# Remove VR permission
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/permissions/android.hardware.vr.headtracking.xml)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/permissions/android.hardware.vr.high_performance.xml)

# Remove obsolete android.hardware.boot@1.0-impl-wrapper.recovery.so
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/recovery/root/system/lib64/hw/android.hardware.boot@1.0-impl-wrapper.recovery.so)

# Move android.hidl.base@1.0.so to system_ext
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/android.hidl.base@1.0.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib64/android.hidl.base@1.0.so)

# Remove Face permission
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/permissions/android.hardware.biometrics.face.xml)

# Removing GSI keys from the ramdisk.
# Those keys will be embedded into VTS instead, to verify the GSI image in used.
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/recovery/root/first_stage_ramdisk/avb/q-gsi.avbpubkey)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/recovery/root/first_stage_ramdisk/avb/r-gsi.avbpubkey)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/recovery/root/first_stage_ramdisk/avb/s-gsi.avbpubkey)

# Use stable aidl power HAL
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/init/android.hardware.power@1.3-service.pixel-libperfmgr.rc)

# Remove generic atrace HAL
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/init/android.hardware.atrace@1.0-service.rc)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/vendor/etc/vintf/manifest/android.hardware.atrace@1.0-service.xml)
