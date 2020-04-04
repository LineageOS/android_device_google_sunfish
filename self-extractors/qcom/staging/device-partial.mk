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

PRODUCT_SOONG_NAMESPACES += \
    vendor/qcom/sunfish/proprietary

#  blob(s) necessary for sunfish hardware
PRODUCT_PACKAGES := \
    libadsprpc_system \
    libcdsprpc_system \
    libdisplayconfig \
    libmdsprpc_system \
    libmmosal \
    libOpenCL_system \
    libqct_resampler \
    libqmi_cci_system \
    libqti-util_system \
    libsdsprpc_system \
    libsns_device_mode_stub \
    libsns_fastRPC_util \
    libsns_low_lat_stream_stub \

PRODUCT_COPY_FILES := \
    vendor/qcom/sunfish/proprietary/ATT_profiles.xml:system/etc/cne/Nexus/ATT/ATT_profiles.xml \
    vendor/qcom/sunfish/proprietary/ROW_profiles.xml:system/etc/cne/Nexus/ROW/ROW_profiles.xml \
    vendor/qcom/sunfish/proprietary/VZW_profiles.xml:system/etc/cne/Nexus/VZW/VZW_profiles.xml \
    vendor/qcom/sunfish/proprietary/audiosphere.xml:system/etc/permissions/audiosphere.xml \
    vendor/qcom/sunfish/proprietary/com.qti.snapdragon.sdk.display.xml:system/etc/permissions/com.qti.snapdragon.sdk.display.xml \
    vendor/qcom/sunfish/proprietary/com.qti.location.sdk.jar:system/framework/com.qti.location.sdk.jar \
    vendor/qcom/sunfish/proprietary/com.qti.snapdragon.sdk.display.jar:system/framework/com.qti.snapdragon.sdk.display.jar \
    vendor/qcom/sunfish/proprietary/ConnectivityExt.jar:system/framework/ConnectivityExt.jar \

