LOCAL_PATH := $(call my-dir)

#A/B builds require us to create the mount points at compile time.
#Just creating it for all cases since it does not hurt.
FIRMWARE_MOUNT_POINT := $(TARGET_OUT_VENDOR)/firmware_mnt
ALL_DEFAULT_INSTALLED_MODULES += $(FIRMWARE_MOUNT_POINT)

$(FIRMWARE_MOUNT_POINT):
	@echo "Creating $(FIRMWARE_MOUNT_POINT)"
	@mkdir -p $(TARGET_OUT_VENDOR)/firmware_mnt

# copy kernel headers to the build tree
$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr: $(wildcard $(PRODUCT_VENDOR_KERNEL_HEADERS)/*)
	rm -rf $@
	mkdir -p $@/include
	cp -a $(PRODUCT_VENDOR_KERNEL_HEADERS)/. $@/include

#----------------------------------------------------------------------
# build and sign the final stage of bootloader
#----------------------------------------------------------------------
.PHONY: aboot
ifeq ($(USESECIMAGETOOL), true)
aboot: gensecimage_target gensecimage_install
else
aboot: $(INSTALLED_BOOTLOADER_MODULE)
endif
