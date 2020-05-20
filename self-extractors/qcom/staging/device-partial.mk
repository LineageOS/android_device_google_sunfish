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


# AOSP packages required by the blobs
PRODUCT_PACKAGES := \
    ims \
    qcrilmsgtunnel \
    QtiTelephonyService

#  blob(s) necessary for sunfish hardware
PRODUCT_COPY_FILES := \
    vendor/qcom/sunfish/proprietary/ATT_profiles.xml:system/etc/cne/Nexus/ATT/ATT_profiles.xml \
    vendor/qcom/sunfish/proprietary/ROW_profiles.xml:system/etc/cne/Nexus/ROW/ROW_profiles.xml \
    vendor/qcom/sunfish/proprietary/VZW_profiles.xml:system/etc/cne/Nexus/VZW/VZW_profiles.xml \
    vendor/qcom/sunfish/proprietary/audiosphere.xml:system/etc/permissions/audiosphere.xml \
    vendor/qcom/sunfish/proprietary/com.qti.snapdragon.sdk.display.xml:system/etc/permissions/com.qti.snapdragon.sdk.display.xml \
    vendor/qcom/sunfish/proprietary/com.qti.location.sdk.jar:system/framework/com.qti.location.sdk.jar \
    vendor/qcom/sunfish/proprietary/com.qti.snapdragon.sdk.display.jar:system/framework/com.qti.snapdragon.sdk.display.jar \
    vendor/qcom/sunfish/proprietary/ConnectivityExt.jar:system/framework/ConnectivityExt.jar \
    vendor/qcom/sunfish/proprietary/lib64/libadsprpc_system.so:system/lib64/libadsprpc_system.so \
    vendor/qcom/sunfish/proprietary/lib64/libcdsprpc_system.so:system/lib64/libcdsprpc_system.so \
    vendor/qcom/sunfish/proprietary/lib64/libdisplayconfig.so:system/lib64/libdisplayconfig.so \
    vendor/qcom/sunfish/proprietary/lib64/libmdsprpc_system.so:system/lib64/libmdsprpc_system.so \
    vendor/qcom/sunfish/proprietary/lib64/libmediaplayerservice.so:system/lib64/libmediaplayerservice.so \
    vendor/qcom/sunfish/proprietary/lib64/libmmosal.so:system/lib64/libmmosal.so \
    vendor/qcom/sunfish/proprietary/lib64/libOpenCL_system.so:system/lib64/libOpenCL_system.so \
    vendor/qcom/sunfish/proprietary/lib64/libqmi_cci_system.so:system/lib64/libqmi_cci_system.so \
    vendor/qcom/sunfish/proprietary/lib64/libqti-util_system.so:system/lib64/libqti-util_system.so \
    vendor/qcom/sunfish/proprietary/lib64/libsdsprpc_system.so:system/lib64/libsdsprpc_system.so \
    vendor/qcom/sunfish/proprietary/lib64/libsns_device_mode_stub.so:system/lib64/libsns_device_mode_stub.so \
    vendor/qcom/sunfish/proprietary/lib64/libsns_fastRPC_util.so:system/lib64/libsns_fastRPC_util.so \
    vendor/qcom/sunfish/proprietary/lib64/libsns_low_lat_stream_stub.so:system/lib64/libsns_low_lat_stream_stub.so \
    vendor/qcom/sunfish/proprietary/lib64/libstagefright_httplive.so:system/lib64/libstagefright_httplive.so \
    vendor/qcom/sunfish/proprietary/libadsprpc_system.so:system/lib/libadsprpc_system.so \
    vendor/qcom/sunfish/proprietary/libcdsprpc_system.so:system/lib/libcdsprpc_system.so \
    vendor/qcom/sunfish/proprietary/libdisplayconfig.so:system/lib/libdisplayconfig.so \
    vendor/qcom/sunfish/proprietary/libmdsprpc_system.so:system/lib/libmdsprpc_system.so \
    vendor/qcom/sunfish/proprietary/libmmosal.so:system/lib/libmmosal.so \
    vendor/qcom/sunfish/proprietary/libOpenCL_system.so:system/lib/libOpenCL_system.so \
    vendor/qcom/sunfish/proprietary/libqct_resampler.so:system/lib/libqct_resampler.so \
    vendor/qcom/sunfish/proprietary/libqmi_cci_system.so:system/lib/libqmi_cci_system.so \
    vendor/qcom/sunfish/proprietary/libqti-util_system.so:system/lib/libqti-util_system.so \
    vendor/qcom/sunfish/proprietary/libsdsprpc_system.so:system/lib/libsdsprpc_system.so \
    vendor/qcom/sunfish/proprietary/libsns_device_mode_stub.so:system/lib/libsns_device_mode_stub.so \
    vendor/qcom/sunfish/proprietary/libsns_fastRPC_util.so:system/lib/libsns_fastRPC_util.so \
    vendor/qcom/sunfish/proprietary/libsns_low_lat_stream_stub.so:system/lib/libsns_low_lat_stream_stub.so \
    vendor/qcom/sunfish/proprietary/qcrilhook.xml:product/etc/permissions/qcrilhook.xml \
    vendor/qcom/sunfish/proprietary/telephonyservice.xml:product/etc/permissions/telephonyservice.xml \
    vendor/qcom/sunfish/proprietary/qcrilhook.jar:product/framework/qcrilhook.jar \
    vendor/qcom/sunfish/proprietary/qcrilhook.jar:product/framework/oat/arm64/qcrilhook.vdex \
    vendor/qcom/sunfish/proprietary/qcrilhook.jar:product/framework/oat/arm/qcrilhook.vdex \
    vendor/qcom/sunfish/proprietary/qcrilhook.jar:product/framework/oat/arm64/qcrilhook.odex \
    vendor/qcom/sunfish/proprietary/qcrilhook.jar:product/framework/oat/arm/qcrilhook.odex \
    vendor/qcom/sunfish/proprietary/qti-telephony-utils.jar:product/framework/qti-telephony-utils.jar \
    vendor/qcom/sunfish/proprietary/qti-telephony-hidl-wrapper.jar:product/framework/qti-telephony-hidl-wrapper.jar \
    vendor/qcom/sunfish/proprietary/qti-telephony-hidl-wrapper.jar:product/framework/oat/arm64/qti-telephony-hidl-wrapper.vdex \
    vendor/qcom/sunfish/proprietary/qti-telephony-hidl-wrapper.jar:product/framework/oat/arm/qti-telephony-hidl-wrapper.vdex \
    vendor/qcom/sunfish/proprietary/qti-telephony-hidl-wrapper.jar:product/framework/oat/arm64/qti-telephony-hidl-wrapper.odex \
    vendor/qcom/sunfish/proprietary/qti-telephony-hidl-wrapper.jar:product/framework/oat/arm/qti-telephony-hidl-wrapper.odex \
    vendor/qcom/sunfish/proprietary/qti_telephony_hidl_wrapper.xml:product/etc/permissions/qti_telephony_hidl_wrapper.xml \
    vendor/qcom/sunfish/proprietary/qti_telephony_utils.xml:product/etc/permissions/qti_telephony_utils.xml \
    vendor/qcom/sunfish/proprietary/com.qualcomm.qcrilmsgtunnel.xml:product/etc/permissions/com.qualcomm.qcrilmsgtunnel.xml \
    vendor/qcom/sunfish/proprietary/QtiTelephonyServicelibrary.jar:product/framework/QtiTelephonyServicelibrary.jar \
    vendor/qcom/sunfish/proprietary/QtiTelephonyServicelibrary.jar:product/framework/oat/arm64/QtiTelephonyServicelibrary.vdex \
    vendor/qcom/sunfish/proprietary/QtiTelephonyServicelibrary.jar:product/framework/oat/arm/QtiTelephonyServicelibrary.vdex \
    vendor/qcom/sunfish/proprietary/QtiTelephonyServicelibrary.jar:product/framework/oat/arm64/QtiTelephonyServicelibrary.odex \
    vendor/qcom/sunfish/proprietary/QtiTelephonyServicelibrary.jar:product/framework/oat/arm/QtiTelephonyServicelibrary.odex \
    vendor/qcom/sunfish/proprietary/lib64/libimsmedia_jni.so:product/lib64/libimsmedia_jni.so \
    vendor/qcom/sunfish/proprietary/libimsmedia_jni.so:product/lib/libimsmedia_jni.so \
    vendor/qcom/sunfish/proprietary/QtiTelephonyServicelibrary.jar:product/priv-app/qcrilmsgtunnel/oat/arm64/qcrilmsgtunnel.vdex \
    vendor/qcom/sunfish/proprietary/QtiTelephonyServicelibrary.jar:product/priv-app/qcrilmsgtunnel/oat/arm64/qcrilmsgtunnel.odex \

