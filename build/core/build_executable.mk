##############################################################
##  Author: cblock@126.com  FergusZeng 2016 ShenZhen,China  ##
##############################################################
#                     target.mk template
##############################################################
# LOCAL_PATH := $(call my-dir)
# include $(CLEAR_VARS)
# LOCAL_MODULE := test
# LOCAL_CFLAGS :=
# LOCAL_CXXFLAGS :=
# LOCAL_LDFLAGS :=
# 	
# LOCAL_INC_PATHS := \
# 	$(LOCAL_PATH) \
# 	$(LOCAL_PATH)/inc
# 
# LOCAL_SRC_FILES := \
# 	$(LOCAL_PATH)/aaa.cpp
#
# LOCAL_LIB_PATHS := \
# 	$(LOCAL_PATH)/lib
#
# include $(BUILD_EXECUTABLE)
##############################################################

INC_PATHS := -I$(DEFAULT_INC_PATH)
INC_PATHS += $(foreach f,$(LOCAL_INC_PATHS),-I$(f))
LIB_PATHS := -L$(DEFAULT_LIB_PATH)
LIB_PATHS += $(foreach f,$(LOCAL_LIB_PATHS),-L$(f))

ALL_CFLAGS := $(DEFAULT_CC_FLAGS) $(INC_PATHS) $(LOCAL_CFLAGS) $(LIB_PATHS) $(LOCAL_LDFLAGS) 
ALL_CXXFLAGS := $(ALL_CFLAGS) $(LOCAL_CXXFLAGS)

# MODULE_FILE:目标模块文件名
MODULE_FILE := $(OBJECT_PATH)/$(LOCAL_MODULE)
DEST_FILE := $(DEFAULT_BIN_PATH)/$(notdir $(MODULE_FILE))

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
	@cp $(DEST_FILE) $(LOCAL_INSTALL_BUILD)

# 生成编译单个.o文件所需的Makefile
include $(BUILD_DEPFILE)

# 生成目标文件
$(MODULE_FILE): $(OBJECT_FILES)
	$(CC) $(OBJECT_FILES) $(ALL_CXXFLAGS) -g -o $@

.PHONY:clean
clean:
	@-rm -f $(DEST_FILE)
	@-rm -f $(OBJECT_FILES)
	@-rm -f $(DEP_FILE)
	@-rm -f $(basename $(DEP_FILE))
	@echo -e "clean objects ok: $(DEST_FILE) $(OBJECT_FILES) $(DEP_FILE)"
