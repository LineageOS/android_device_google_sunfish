LOCAL_PATH := $(call my-dir)

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
