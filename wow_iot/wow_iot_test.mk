LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := wow_iot_test

LOCAL_CFLAGS := 
LOCAL_CXXFLAGS :=
 
LOCAL_LDFLAGS += -lwowiot -lwowhal -lwowbase  -lev -lmbedcrypto -lmbedx509 -lmbedtls
LOCAL_LDFLAGS += -L  -lrt -lc -ldl -lm  -pthread 

LOCAL_INC_PATHS := \
	$(LOCAL_OUTPUT_PATH)/include/wow_base \
	$(LOCAL_OUTPUT_PATH)/include/wow_hal \
	$(LOCAL_OUTPUT_PATH)/include/wow_iot \
	$(LOCAL_PATH)	\
	$(LOCAL_PATH)/inc/\
	$(LOCAL_PATH)/test/	
 
LOCAL_SRC_FILES :=  test/main.c 

LOCAL_SRC_FILES +=  \
			test/algorithm/suit_bitmap.c \
			test/algorithm/suit_bloomfilter.c

LOCAL_SRC_FILES +=  \
			test/database/suit_database_sqlite.c 

LOCAL_SRC_FILES +=  \
			test/config/suit_ini.c \
			test/config/suit_xml.c \
			test/config/suit_json.c

LOCAL_SRC_FILES +=  \
			test/encrypt/suit_crc.c \
			test/encrypt/suit_rsa.c

LOCAL_SRC_FILES +=  \
			test/hash/suit_hash.c \
			test/hash/suit_hash_map.c \
			test/hash/suit_hash_set.c

LOCAL_SRC_FILES += \
			test/network/suit_ftp.c \
			test/network/suit_ntpdate.c

LOCAL_SRC_FILES += \
			test/plugin/suit_shell.c \
			test/plugin/suit_log.c

LOCAL_SRC_FILES += \
			test/protocol/suit_modbus_m.c \
			test/protocol/suit_modbus_s.c \
			test/protocol/suit_iec104m.c \
			test/protocol/suit_iec104s.c 

include $(BUILD_EXECUTABLE)


