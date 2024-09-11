#!/bin/bash
#
# SPDX-FileCopyrightText: 2016 The CyanogenMod Project
# SPDX-FileCopyrightText: 2017-2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

set -e

DEVICE=sunfish
VENDOR=google

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

export TARGET_ENABLE_CHECKELF=true

# If XML files don't have comments before the XML header, use this flag
# Can still be used with broken XML files by using blob_fixup
export TARGET_DISABLE_XML_FIXING=true

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

ONLY_FIRMWARE=
KANG=
SECTION=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        --only-firmware)
            ONLY_FIRMWARE=true
            ;;
        -n | --no-cleanup)
            CLEAN_VENDOR=false
            ;;
        -k | --kang)
            KANG="--kang"
            ;;
        -s | --section)
            SECTION="${2}"
            shift
            CLEAN_VENDOR=false
            ;;
        *)
            SRC="${1}"
            ;;
    esac
    shift
done

if [ -z "${SRC}" ]; then
    SRC="adb"
fi

function blob_fixup() {
    case "${1}" in
        product/etc/felica/common.cfg)
            [ "$2" = "" ] && return 0
            sed -i -e '$a00000018,1' -e '/^00000014/d' -e '/^00000015/d' "${2}"
            ;;
        # Fix typo in qcrilmsgtunnel whitelist
        product/etc/sysconfig/nexus.xml)
            sed -i 's/qulacomm/qualcomm/' "${2}"
            ;;
        system_ext/priv-app/HbmSVManager/HbmSVManager.apk)
            [ "$2" = "" ] && return 0
            apktool_patch "${2}" "${MY_DIR}/blob-patches/HbmSVManager.patch" -r
            ;;
        vendor/bin/hw/android.hardware.rebootescrow-service.citadel | \
            vendor/lib64/android.hardware.keymaster@4.1-impl.nos.so)
            [ "$2" = "" ] && return 0
            "${PATCHELF}" --add-needed "libcrypto_shim.so" "${2}"
            ;;
        vendor/lib64/libgooglecamerahal.so)
            [ "$2" = "" ] && return 0
            "${PATCHELF}" --add-needed "libmeminfo_shim.so" "${2}"
            ;;
        *)
            return 1
            ;;
    esac

    return 0
}

function blob_fixup_dry() {
    blob_fixup "$1" ""
}

function prepare_firmware() {
    if [ "${SRC}" != "adb" ]; then
        bash "${ANDROID_ROOT}"/lineage/scripts/pixel/prepare-firmware.sh "${DEVICE}" "${SRC}"
    fi
}

# Initialize the helper
setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" false "${CLEAN_VENDOR}"

if [ -z "${ONLY_FIRMWARE}" ]; then
    extract "${MY_DIR}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"
    extract "${MY_DIR}/proprietary-files-carriersettings.txt" "${SRC}" "${KANG}" --section "${SECTION}"
    extract "${MY_DIR}/proprietary-files-vendor.txt" "${SRC}" "${KANG}" --section "${SECTION}"
fi

if [ -z "${SECTION}" ]; then
    extract_firmware "${MY_DIR}/proprietary-firmware.txt" "${SRC}"
fi

"${MY_DIR}/setup-makefiles.sh"
