LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libwowiot

$(shell cp -rf $(LOCAL_PATH)/inc/* $(LOCAL_OUTPUT_PATH)/include/wow_iot/)

LOCAL_CFLAGS +=  
LOCAL_CXXFLAGS +=
LOCAL_LDFLAGS += -lwowhal -lwowbase -lev -lmbedcrypto -lmbedx509 -lmbedtls
LOCAL_LDFLAGS += -L -pthread -ldl -lm -lrt -lc 
LOCAL_INC_PATHS := \
	$(LOCAL_OUTPUT_PATH)/include/wow_base \
	$(LOCAL_OUTPUT_PATH)/include/wow_hal \
	$(LOCAL_PATH)/src	\
	$(LOCAL_PATH)/inc 		


LOCAL_SRC_FILES +=  src/wow_iot_init.c

LOCAL_SRC_FILES +=  \
				src/algorithm/wow_bitmap.c \
				src/algorithm/wow_bloomfilter.c

LOCAL_SRC_FILES +=  \
				src/database/sqlite/sqlite3.c \
				src/database/sqlite/wow_sqlite.c \
				src/database/wow_database_val.c \
				src/database/wow_database.c \

LOCAL_SRC_FILES += \
				src/config/ini/dictionary.c \
				src/config/ini/iniparser.c \
				src/config/ini/wow_ini.c \
				src/config/xml/ezxml.c \
				src/config/xml/wow_xml.c \
				src/config/json/cJSON.c \
				src/config/json/wow_json.c 

LOCAL_SRC_FILES += \
				src/datastruct/wow_iterator.c 

LOCAL_SRC_FILES += \
				src/element/element_ptr.c \
				src/element/element_str.c \
				src/element/element_uint8.c \
				src/element/element_uint16.c \
				src/element/element_uint32.c \
				src/element/wow_element.c
				
LOCAL_SRC_FILES += \
				src/encrypt/wow_crc.c \
				src/encrypt/wow_rsa.c

LOCAL_SRC_FILES += \
				src/hash/wow_hash.c \
				src/hash/wow_hash_map.c \
				src/hash/wow_hash_set.c

LOCAL_SRC_FILES += \
				src/network/wow_ftp.c \
				src/network/wow_ntpdate.c

LOCAL_SRC_FILES += \
				src/plugin/wow_shell.c \
				src/plugin/wow_debug.c \
				src/plugin/wow_log.c

LOCAL_SRC_FILES += \
				src/protocol/wow_proto_port.c \
				src/protocol/wow_proto_data.c \
				src/protocol/modbus/modbus_rtu.c \
				src/protocol/modbus/modbus_tcp.c \
				src/protocol/modbus/wow_modbus_m.c \
				src/protocol/modbus/wow_modbus_s.c \
				src/protocol/iec10x/wow_iec104m.c \
				src/protocol/iec10x/wow_iec104s.c 

LOCAL_SRC_FILES += \
				src/shmcache/shm_hashtable.c\
				src/shmcache/shm_lock.c\
				src/shmcache/shm_object_pool.c\
				src/shmcache/shm_op_wrapper.c\
				src/shmcache/shm_striping_allocator.c \
				src/shmcache/shm_value_allocator.c \
				src/shmcache/shmcache.c \
				src/shmcache/shmopt.c \
				src/shmcache/wow_shmcache.c 

LOCAL_LIB_TYPE:=shared

include $(BUILD_LIBRARY)