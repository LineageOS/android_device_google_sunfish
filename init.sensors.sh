#! /vendor/bin/sh

version=`grep -ao "OEM_IMAGE_VERSION_STRING[ -~]*" \
              /vendor/firmware/slpi.b04 | \
         sed -e s/OEM_IMAGE_VERSION_STRING=SLPI.version.// -e s/\(.*\).//`
setprop sys.slpi.firmware.version "$version"
