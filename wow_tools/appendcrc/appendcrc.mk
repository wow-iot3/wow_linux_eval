LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := appendcrc

LOCAL_CFLAGS := -Wall
LOCAL_CXXFLAGS :=
LOCAL_LDFLAGS := -L  -lrt -lc -ldl -lm  -pthread 

LOCAL_INC_PATHS := \
	$(LOCAL_PATH)/../../wow_base/inc/ \
	$(LOCAL_PATH)/../../wow_iot/inc/ \
	$(LOCAL_PATH)

LOCAL_SRC_FILES := \
				wow_append_crc.c
	
LOCAL_SRC_FILES += \
				../../wow_base/src/file/wow_hfile.c \
				../../wow_base/src/file/hfile/io.c \
				../../wow_base/src/file/hfile/fio.c	
				
LOCAL_SRC_FILES += \
				../../wow_base/src/system/wow_cond.c \
				../../wow_base/src/system/wow_lock.c \

LOCAL_SRC_FILES +=  \
				../../wow_base/src/utils/wow_mem_leak.c \
				../../wow_base/src/utils/wow_error.c \
				../../wow_base/src/utils/wow_type.c 


LOCAL_SRC_FILES +=  \
				../../wow_iot/src/encrypt/wow_crc.c

include $(BUILD_TOOLS)
