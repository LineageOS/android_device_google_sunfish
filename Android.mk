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

LOCAL_PATH := $(call my-dir)

$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,default-permissions.xml,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,libnfc-nci.conf,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,fstab.postinstall,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,ueventd.rc,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,wpa_supplicant.conf,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,hals.conf,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,media_profiles_V1_0.xml,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,media_codecs_performance.xml,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,device_state_configuration.xml,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,task_profiles.json,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,p2p_supplicant.conf,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,wpa_supplicant.conf,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))
$(eval $(call declare-copy-files-license-metadata,device/google/sunfish,wpa_supplicant_overlay.conf,SPDX-license-identifier-Apache-2.0,notice,build/soong/licenses/LICENSE,))

$(eval $(call declare-1p-copy-files,device/google/sunfish,audio_policy_configuration.xml))

ifeq ($(USES_DEVICE_GOOGLE_SUNFISH),true)
  subdir_makefiles=$(call first-makefiles-under,$(LOCAL_PATH))
  $(foreach mk,$(subdir_makefiles),$(info including $(mk) ...)$(eval include $(mk)))

SECUREUI_LIBS := libsecureuisvc_jni.so
SECUREUI_SYMLINKS := $(addprefix $(TARGET_OUT_SYSTEM_EXT)/app/com.qualcomm.qti.services.secureui/lib/arm64/,$(notdir $(SECUREUI_LIBS)))
$(SECUREUI_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "SecureUI lib link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf /system_ext/lib64/$(notdir $@) $@

DM_LIBS := libdmengine.so libdmjavaplugin.so
DM_SYMLINKS := $(addprefix $(TARGET_OUT_PRODUCT)/priv-app/DMService/lib/arm/,$(notdir $(DM_LIBS)))
$(DM_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "DMService lib link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf /product/lib/$(notdir $@) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SECUREUI_SYMLINKS) $(DM_SYMLINKS)
endif
