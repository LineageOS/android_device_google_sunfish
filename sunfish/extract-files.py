#!/usr/bin/env -S PYTHONPATH=../../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.extract import extract_fns_user_type
from extract_utils.extract_pixel import (
    extract_pixel_factory_image,
    extract_pixel_firmware,
    pixel_factory_image_regex,
    pixel_firmware_regex,
)
from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/google/sunfish',
    'hardware/google/interfaces',
    'hardware/google/pixel/pixelstats',
    'hardware/google/pixel/power-libperfmgr',
    'hardware/qcom/sm8150/display',
    'hardware/qcom/sm8150/gps',
    'hardware/qcom/wlan/legacy',
    'vendor/qcom/opensource/display',
]


def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'vendor' else None


lib_fixups: lib_fixups_user_type = {
    **lib_fixups,
    (
        'vendor.qti.hardware.tui_comm@1.0',
        'vendor.qti.imsrtpservice@3.0',
    ): lib_fixup_vendor_suffix,
}

blob_fixups: blob_fixups_user_type = {
    'product/etc/felica/common.cfg': blob_fixup()
        .patch_file('osaifu-keitai.patch'),
    'product/etc/sysconfig/nexus.xml': blob_fixup()
        .regex_replace('qulacomm', 'qualcomm'),
    'system_ext/lib64/libsecureuisvc_jni.so': blob_fixup()
        .add_needed('libgui_shim.so'),
    'system_ext/priv-app/HbmSVManager/HbmSVManager.apk': blob_fixup()
        .apktool_patch('HbmSVManager.patch'),
    (
        'vendor/bin/hw/android.hardware.rebootescrow-service.citadel',
        'vendor/lib64/android.hardware.keymaster@4.1-impl.nos.so',
    ): blob_fixup()
        .add_needed('libcrypto_shim.so'),
    'vendor/lib64/libgooglecamerahal.so': blob_fixup()
        .add_needed('libmeminfo_shim.so'),
    (
        'vendor/lib/libdpps.so',
        'vendor/lib64/libdpps.so',
        'vendor/lib64/libgooglecamerahwl_impl.so',
    ): blob_fixup()
        .replace_needed('libtinyxml2.so', 'libtinyxml2-v34.so'),
}  # fmt: skip

extract_fns: extract_fns_user_type = {
    pixel_factory_image_regex: extract_pixel_factory_image,
    pixel_firmware_regex: extract_pixel_firmware,
}

module = ExtractUtilsModule(
    'sunfish',
    'google',
    device_rel_path='device/google/sunfish/sunfish',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
    add_firmware_proprietary_file=True,
    extract_fns=extract_fns,
)

module.add_proprietary_file('proprietary-files-carriersettings.txt')
module.add_proprietary_file('proprietary-files-vendor.txt')

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
