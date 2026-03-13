##############################################################
##  Author: cblock@126.com  FergusZeng 2016 ShenZhen,China  ##
##############################################################
#                    target.mk template
##############################################################
# LOCAL_PATH := $(call my-dir)
# include $(CLEAR_VARS)
# LOCAL_MODULE := libdemo
# LOCAL_CFLAGS :=
# LOCAL_CXXFLAGS :=
# LOCAL_LDFLAGS :=
# LOCAL_INC_PATHS := \
#     $(LOCAL_PATH)
#
# LOCAL_SRC_FILES := \
#     aaa.c \
# 
# LOCAL_LIB_TYPE:=static 
# include $(BUILD_LIBRARY)
##############################################################

INC_PATHS := -I$(DEFAULT_INC_PATH)
INC_PATHS += $(foreach f,$(LOCAL_INC_PATHS),-I$(f))
LIB_PATHS := -L$(DEFAULT_LIB_PATH)
LIB_PATHS += $(foreach f,$(LOCAL_LIB_PATHS),-L$(f))

$(warning "LOCAL_INC_PATHS->$(LOCAL_INC_PATHS)")
$(warning "INC_PATHS->$(INC_PATHS)")

ALL_CFLAGS := $(DEFAULT_CC_FLAGS) $(INC_PATHS) $(LOCAL_CFLAGS) $(LIB_PATHS) $(LOCAL_LDFLAGS) 
ALL_CXXFLAGS := $(ALL_CFLAGS) $(LOCAL_CXXFLAGS)

#如果设置了LINK_TYPE,则覆盖mk中的LOCAL_LIB_TYPE
ifneq ($(LINK_TYPE),)
LOCAL_LIB_TYPE := $(LINK_TYPE)
endif

# MODULE_FILE:目标模块文件名
ifeq ($(LOCAL_LIB_TYPE),shared)
MODULE_FILE := $(OBJECT_PATH)/$(LOCAL_MODULE).so
ALL_CFLAGS += -fPIC
ALL_CXXFLAGS += -fPIC
else ifeq ($(LOCAL_LIB_TYPE),static)
MODULE_FILE := $(OBJECT_PATH)/$(LOCAL_MODULE).a
else ifeq ($(LOCAL_LIB_TYPE),)
MODULE_FILE := $(OBJECT_PATH)/$(LOCAL_MODULE).a
else
$(error "LOCAL_LIB_TYPE:$(LOCAL_LIB_TYPE) error!")
endif

DEST_FILE := $(DEFAULT_LIB_PATH)/$(notdir $(MODULE_FILE)).$(version_major).$(version_minor).$(version_patch)

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
all:$(MODULE_FILE)
	@install $(MODULE_FILE) $(DEST_FILE)
	@echo -e "\033[33m\033[1m==>build '$(SOURCES_DIR)' successfully: [$(DEST_FILE)].\033[0m"
	@$(LOCAL_INSTALL_BUILD)/appendcrc $(DEST_FILE)
	@echo -e "\033[33m\033[1m==>exec appendcrc successfully.\033[0m"
	@rm -f $(DEFAULT_LIB_PATH)/$(notdir $(MODULE_FILE))
	@ln -s $(DEST_FILE) $(DEFAULT_LIB_PATH)/$(notdir $(MODULE_FILE))
	@cp $(DEST_FILE) $(LOCAL_INSTALL_BUILD)/$(notdir $(MODULE_FILE))

# 生成编译单个.o文件所需的Makefile
include $(BUILD_DEPFILE)

# 生成目标文件
$(MODULE_FILE): $(OBJECT_FILES)
ifeq ($(LOCAL_LIB_TYPE),shared)
	$(CC) $(ALL_CFLAGS) -shared -fPIC -o $@ $(OBJECT_FILES)
else
	$(AR) -r $@ $(OBJECT_FILES)
endif 

.PHONY:clean
clean:
	@-rm -f $(DEST_FILE)
	@-rm -f $(OBJECT_FILES)
	@-rm -f $(DEP_FILE)
	@-rm -f $(basename $(DEP_FILE))
	@echo -e "clean objects ok: $(DEST_FILE) $(OBJECT_FILES) $(DEP_FILE)"
