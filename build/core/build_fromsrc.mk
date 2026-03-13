##############################################################
##  Author: cblock@126.com  FergusZeng 2016 ShenZhen,China  ##
##############################################################
#                     target.mk template
##############################################################
# LOCAL_PATH := $(call my-dir)
# include $(CLEAR_VARS)
#
# LOCAL_LIB_TYPE:=static
#
# LOCAL_BUILD_DIR:=source_dir
# LOCAL_BUILD_MAKEFILE:=Makefile.mbuild
#
# include $(BUILD_FROMSRC)
##############################################################

ifeq ($(BUILD_SYSTEM),)
$(error "BUILD_SYSTEM is null.")
endif

ifeq ($(BUILD_OUTPUT_PATH),)
$(error "BUILD_OUTPUT_PATH is null.")
endif

ifeq ($(LOCAL_LIB_TYPE),)
$(error "LOCAL_LIB_TYPE is null.")
endif

ifeq ($(LOCAL_BUILD_DIR),)
$(error "LOCAL_BUILD_DIR is null.")
endif

ifeq ($(LOCAL_BUILD_MAKEFILE),)
$(error "LOCAL_BUILD_MAKEFILE is null.")
endif

#如果设置了LINK_TYPE,则覆盖mk中的LOCAL_LIB_TYPE
ifneq ($(LINK_TYPE),)
LOCAL_LIB_TYPE := $(LINK_TYPE)
endif

ifeq ($(LOCAL_LIB_TYPE),shared)
$(info "build shared lib.")
else ifeq ($(LOCAL_LIB_TYPE),static)
$(info "build static lib.")
else
$(error "LOCAL_LIB_TYPE:$(LOCAL_LIB_TYPE) error!")
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
	cd $(LOCAL_PATH)/$(LOCAL_BUILD_DIR);make all -f $(LOCAL_BUILD_MAKEFILE) HOST=$(HOST) LINKTYPE=$(LOCAL_LIB_TYPE) PREFIX=$(BUILD_OUTPUT_PATH) PROJECT_ROOT=$(PROJECT_ROOT)
	cd $(LOCAL_PATH)/$(LOCAL_BUILD_DIR);make install -f $(LOCAL_BUILD_MAKEFILE) HOST=$(HOST) LINKTYPE=$(LOCAL_LIB_TYPE) PREFIX=$(BUILD_OUTPUT_PATH) PROJECT_ROOT=$(PROJECT_ROOT)
.PHONY: clean
clean:
	cd $(LOCAL_PATH)/$(LOCAL_BUILD_DIR);make clean -f $(LOCAL_BUILD_MAKEFILE) HOST=$(HOST) LINKTYPE=$(LOCAL_LIB_TYPE) PREFIX=$(BUILD_OUTPUT_PATH) PROJECT_ROOT=$(PROJECT_ROOT)