LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libwowhal

$(shell cp -rf $(LOCAL_PATH)/inc/* $(LOCAL_OUTPUT_PATH)/include/wow_hal/)

LOCAL_CFLAGS +=  
LOCAL_CXXFLAGS +=
LOCAL_LDFLAGS += -lwowbase -lev
LOCAL_LDFLAGS += -L -pthread -ldl -lm -lrt -lc 
LOCAL_INC_PATHS := \
	$(LOCAL_OUTPUT_PATH)/include/wow_base \
	$(LOCAL_PATH)/src	\
	$(LOCAL_PATH)/inc 		
 
LOCAL_SRC_FILES += \
				src/wow_hal_init.c 

LOCAL_SRC_FILES += \
				src/board/ubuntu-2004.c 

LOCAL_SRC_FILES += \
				src/usart/wow_usart.c 
	
LOCAL_LIB_TYPE:=shared

include $(BUILD_LIBRARY)