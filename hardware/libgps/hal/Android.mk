#include $(all-subdir-makefiles)

LOCAL_PATH := $(call my-dir)

#ifeq ($(BOARD_GPS_LIBRARIES), libgps)

#ifneq ($(TARGET_PRODUCT),sim)
# HAL module implemenation, not prelinked and stored in
# # hw/<GPS_HARDWARE_MODULE_ID>.<ro.hardware>.so
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
#LOCAL_MULTILIB := both
#LOCAL_MODULE_RELATIVE_PATH := hw
#LOCAL_MULTILIB := TARGET_PREFER_64_BIT
LOCAL_CFLAGS := -DHAVE_GPS_HARDWARE
LOCAL_SHARED_LIBRARIES := liblog libcutils libhardware libc libutils
LOCAL_SRC_FILES := gps_zkw.c

ifeq ($(SUPL_ENABLED),1)
LOCAL_SUPL_PATH=../asn-supl
LOCAL_RRLP_PATH=../asn-rrlp
LOCAL_C_INCLUDES += external/openssl/include/
LOCAL_C_INCLUDES += external/openssl/
LOCAL_C_INCLUDES += external/
LOCAL_SHARED_LIBRARIES += libcrypto
LOCAL_SHARED_LIBRARIES += libssl
LOCAL_SHARED_LIBRARIES += libasnsupl
LOCAL_SHARED_LIBRARIES += libasnrrlp
#LOCAL_STATIC_LIBRARIES += libAsn1
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/$(LOCAL_SUPL_PATH) \
	$(LOCAL_PATH)/$(LOCAL_RRLP_PATH) 
LOCAL_CFLAGS += -DSUPL_ENABLED
LOCAL_SRC_FILES += supl.c 
LOCAL_SRC_FILES += casaid.c 
endif

#LOCAL_MODULE := gps.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE := gps.default
LOCAL_MODULE_TAGS := debug eng
include $(BUILD_SHARED_LIBRARY)
#endif
#adb push  $(TARGET_OUT_SHARED_LIBRARIES)/hw/gps.default /system/lib/hw/
#endif
