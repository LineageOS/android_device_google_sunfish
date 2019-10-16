#! /vendor/bin/sh

version=`grep -ao "OEM_IMAGE_VERSION_STRING[ -~]*" \
              /vendor/firmware/adsp.b04 | \
         sed -e s/OEM_IMAGE_VERSION_STRING=ADSP.version.// -e s/\(.*\).//`
setprop vendor.sys.adsp.firmware.version "$version"
