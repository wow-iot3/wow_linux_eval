LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := wow_hal_test

LOCAL_CFLAGS := 
LOCAL_CXXFLAGS :=
 
LOCAL_LDFLAGS += -lwowhal -lwowbase  -lev 
LOCAL_LDFLAGS += -L  -lrt -lc -ldl -lm  -pthread 

LOCAL_INC_PATHS := \
	$(LOCAL_OUTPUT_PATH)/include/wow_base \
	$(LOCAL_OUTPUT_PATH)/include/wow_hal \
	$(LOCAL_PATH)	\
	$(LOCAL_PATH)/inc/\
	$(LOCAL_PATH)/test/	
 
LOCAL_SRC_FILES :=  test/main.c 


include $(BUILD_EXECUTABLE)


