LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libwowbase

$(shell cp -rf $(LOCAL_PATH)/inc/* $(LOCAL_OUTPUT_PATH)/include/wow_base/)

LOCAL_CFLAGS +=  
LOCAL_CXXFLAGS +=
LOCAL_LDFLAGS += -lev
LOCAL_LDFLAGS += -L -pthread -ldl -lm -lrt -lc 
LOCAL_INC_PATHS := \
	$(LOCAL_PATH)/src	\
	$(LOCAL_PATH)/inc 		
 
LOCAL_SRC_FILES += \
				src/wow_base_init.c 

LOCAL_SRC_FILES += \
				src/file/wow_dir.c \
				src/file/wow_file.c	\
				src/file/wow_hfile.c \
				src/file/hfile/io.c \
				src/file/hfile/fio.c	

LOCAL_SRC_FILES += \
				src/ipc/ipc_skt_client.c \
				src/ipc/ipc_skt_server.c \
				src/ipc/wow_ipc.c 

LOCAL_SRC_FILES += \
				src/list/wow_slist.c \
				src/list/wow_string_list.c \
				src/list/wow_kv_list.c
				 
LOCAL_SRC_FILES += \
				src/network/wow_net_iface.c \
				src/network/wow_socket.c \
				src/network/wow_tcp.c \
				src/network/wow_udp.c

LOCAL_SRC_FILES += \
				src/system/wow_cond.c \
				src/system/wow_countdown.c \
				src/system/wow_lock.c \
				src/system/wow_thread.c \
				src/system/wow_time.c \
				src/system/wow_timer.c 

LOCAL_SRC_FILES += \
				src/utils/wow_byte_array.c \
				src/utils/wow_error.c \
				src/utils/wow_hex.c \
				src/utils/wow_math.c \
				src/utils/wow_mem_leak.c \
				src/utils/wow_ring_buff.c \
				src/utils/wow_singleton.c \
				src/utils/wow_str.c \
				src/utils/wow_string.c \
				src/utils/wow_type.c 

	
LOCAL_LIB_TYPE:=shared

include $(BUILD_LIBRARY)