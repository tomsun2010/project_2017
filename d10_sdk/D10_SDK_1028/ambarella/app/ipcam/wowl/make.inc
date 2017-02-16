##
## app/ipcam/fastboot_app/wowl/make.inc
##
## History:
##    2015/04/03 - [Tao Wu] Created file
##
## Copyright (C) 2015, Ambarella, Inc.
##
## All rights reserved. No Part of this file may be reproduced, stored
## in a retrieval system, or transmitted, in any form, or by any means,
## electronic, mechanical, photocopying, recording, or otherwise,
## without the prior consent of Ambarella, Inc.
##

ifeq ($(BUILD_AMBARELLA_APP_WOWL), y)

LOCAL_PATH   := $(call my-dir)
include $(CLEAR_VARS)

IPCAM_APP_BIN_DIR = $(FAKEROOT_DIR)/usr/bin

LOCAL_TARGET	:= wowl
LOCAL_SRCS	:= $(LOCAL_PATH)/wowl.c

LOCAL_CFLAGS := -I$(PREBUILD_3RD_PARTY_DIR)/libpcap/include
LOCAL_LDFLAGS := -L$(PREBUILD_3RD_PARTY_DIR)/libpcap/usr/lib -lpcap \
	-Wl,-rpath-link=$(PREBUILD_3RD_PARTY_DIR)/libnl/usr/lib \
	-Wl,-rpath-link=$(PREBUILD_3RD_PARTY_DIR)/dbus/usr/lib \
	-lpthread

include $(BUILD_APP)

.PHONY: $(LOCAL_TARGET)

$(LOCAL_TARGET): $(LOCAL_MODULE)
	@mkdir -p $(IPCAM_APP_BIN_DIR)/
	@cp -dpRf $< $(IPCAM_APP_BIN_DIR)/
	@echo "Build $@ Done."

$(call add-target-into-build, $(LOCAL_TARGET))

endif

