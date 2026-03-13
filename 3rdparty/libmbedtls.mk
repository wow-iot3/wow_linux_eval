LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# module name
LOCAL_MODULE := mbedtls

# lib type: static or shared
LOCAL_LIB_TYPE := static

# build source tar name
LOCAL_TAR_NAME := mbedtls-2.13.1

# the command you do in tar dir
ifeq ($(HOST),)
LOCAL_TAR_TODO:=mkdir build && cd build;CC=gcc cmake  -DCMAKE_INSTALL_PREFIX=$(LOCAL_OUTPUT_PATH) -DUSE_SHARED_MBEDTLS_LIBRARY=On ../;make -j4; make install;cp $(LOCAL_OUTPUT_PATH)/lib/libmbedcrypto.so* $(LOCAL_OUTPUT_PATH)/lib/libmbedtls.so* $(LOCAL_OUTPUT_PATH)/lib/libmbedx509.so* $(LOCAL_INSTALL_BUILD)

else
LOCAL_TAR_TODO:=mkdir build && cd build;CC=$(HOST)-gcc cmake  -DCMAKE_INSTALL_PREFIX=$(LOCAL_OUTPUT_PATH) -DUSE_SHARED_MBEDTLS_LIBRARY=On ../;make -j4;make install;cp $(LOCAL_OUTPUT_PATH)/lib/libmbedcrypto.so* $(LOCAL_OUTPUT_PATH)/lib/libmbedtls.so* $(LOCAL_OUTPUT_PATH)/lib/libmbedx509.so* $(LOCAL_INSTALL_BUILD)
endif
 
LOCAL_PRE_BUILD :=
LOCAL_POST_BUILD :=
include $(BUILD_FROMTAR)