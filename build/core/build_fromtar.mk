##############################################################
##  Author: cblock@126.com  FergusZeng 2016 ShenZhen,China  ##
##############################################################
#                     target.mk template
##############################################################
# LOCAL_PATH := $(call my-dir)
# include $(CLEAR_VARS)
#
# LOCAL_TAR_NAME:=source_dir
# LOCAL_TAR_TODO:=./configure --prefix=$(LOCAL_OUTPUT_PATH) --host=$(HOST);make;make install
#
# include $(BUILD_FROMTAR)
##############################################################

ifeq ($(BUILD_SYSTEM),)
$(error "BUILD_SYSTEM is null.")
endif

ifeq ($(BUILD_OUTPUT_PATH),)
$(error "BUILD_OUTPUT_PATH is null.")
endif

ifeq ($(LOCAL_TAR_NAME),)
$(error "LOCAL_TAR_NAME is null.")
endif

ifeq ($(LOCAL_TAR_TODO),)
$(error "LOCAL_TAR_TODO is null.")
endif

.PHONY:pre-build
pre-build:
ifneq ("$(LOCAL_PRE_BUILD)","")
	@echo "make pre-build excute:";
	$(LOCAL_PRE_BUILD)
	@echo "make pre-build ok.";
else
	@echo "make pre-build do nothing."
endif

.PHONY:post-build
post-build:
ifneq ("$(LOCAL_POST_BUILD)","")
	@echo "make post-build excute:";
	$(LOCAL_POST_BUILD)
	@echo "make post-build ok.";
else
	@echo "make post-build do nothing."
endif

.PHONY:all
all:
	cd $(LOCAL_PATH);if [ -e $(LOCAL_TAR_NAME).tar.gz ];then \
	rm -rf $(LOCAL_TAR_NAME);tar zxvf $(LOCAL_TAR_NAME).tar.gz;elif [ -e $(LOCAL_TAR_NAME).tar.bz2 ];then \
	rm -rf $(LOCAL_TAR_NAME);tar jxvf $(LOCAL_TAR_NAME).tar.bz2;else \
	echo "no source tar in $(LOCAL_PATH)";exit -1;fi;
	cd $(LOCAL_PATH)/$(LOCAL_TAR_NAME);$(LOCAL_TAR_TODO);
.PHONY: clean
clean:
	cd $(LOCAL_PATH)/$(LOCAL_TAR_NAME);make clean;cd $(LOCAL_PATH);rm -rf $(LOCAL_TAR_NAME);