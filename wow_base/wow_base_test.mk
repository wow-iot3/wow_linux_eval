LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := wow_base_test

LOCAL_CFLAGS := 
LOCAL_CXXFLAGS :=
 
LOCAL_LDFLAGS += -lwowbase -lev 
LOCAL_LDFLAGS += -L  -lrt -lc -ldl -lm  -pthread 

LOCAL_INC_PATHS := \
	$(LOCAL_PATH)	\
	$(LOCAL_PATH)/inc/\
	$(LOCAL_PATH)/test/	
 
LOCAL_SRC_FILES :=  test/main.c 

LOCAL_SRC_FILES += \
				test/file/suit_dir.c \
				test/file/suit_file.c \
				test/file/suit_hfile.c

LOCAL_SRC_FILES += \
				test/ipc/suit_ipc.c

LOCAL_SRC_FILES += \
				test/network/suit_net_iface.c \
				test/network/suit_socket.c \
				test/network/suit_tcp.c \
				test/network/suit_udp.c

LOCAL_SRC_FILES += \
				test/list/suit_slist.c \
				test/list/suit_string_list.c \
				test/list/suit_kv_list.c

LOCAL_SRC_FILES += \
				test/system/suit_cond.c \
				test/system/suit_countdown.c \
				test/system/suit_lock.c \
				test/system/suit_thread.c \
				test/system/suit_time.c \
				test/system/suit_timer.c

LOCAL_SRC_FILES += \
				test/utils/suit_byte_array.c \
				test/utils/suit_hex.c \
				test/utils/suit_ring_buff.c \
				test/utils/suit_singleton.c \
				test/utils/suit_str.c \
				test/utils/suit_string.c \
				test/utils/suit_type.c 
				
include $(BUILD_EXECUTABLE)


