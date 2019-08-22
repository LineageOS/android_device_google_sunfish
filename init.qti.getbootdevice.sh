#!/vendor/bin/sh
# Copyright (c) 2018, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of The Linux Foundation nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
#Script to find if the boot device is SD card or UFS


echo "Bootdevice setup:  Starting..." > /dev/kmsg

bootdevice=`getprop ro.boot.bootdevice`

if [ "$bootdevice" = "8804000.sdhci" ]; then
	ln -s /dev/block/platform/soc/8804000.sdhci /dev/block/bootdevice
	echo "Waiting for SDHCI device to show up..." > /dev/kmsg
	while [ ! -e "/dev/block/platform/soc/8804000.sdhci" ]; do
		sleep 1
	done
elif [ "$bootdevice" = "1d84000.ufshc" ]; then
	ln -s /dev/block/platform/soc/1d84000.ufshc /dev/block/bootdevice
	echo "Waiting for UFS device to show up..." > /dev/kmsg
	while [ ! -e "/dev/block/platform/soc/1d84000.ufshc" ]; do
		sleep 1;
	done
else
	while true; do
		echo "Boot failure - invalid bootdevice ($bootdevice)" > /dev/kmsg
		sleep 30;
	done
fi
echo "Bootdevice setup:  Completed ($bootdevice)" > /dev/kmsg
