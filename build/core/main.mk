##############################################################
### 该文件被envsetup.sh调用
### 调用时指定三个参数:
### HOST            目标主机名            
### SOURCES_DIR		当前要编译的源码目录
### PROJECT_ROOT 	顶层目录
###该文件主要定义编译的环境并最终调用对应编译目录下的Makefile
##############################################################
##  Author: cblock@126.com  FergusZeng 2016 ShenZhen,China  ##
##############################################################

ifeq ($(HOST),)
CROSS_COMPILE:=
else
CROSS_COMPILE:=$(HOST)-
endif

CC := $(CROSS_COMPILE)gcc
CPP := "$(CC) -E"
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar

BUILD_SYSTEM_NAME := $(shell uname -o)
BUILD_SYSTEM_NAME := $(shell echo $(BUILD_SYSTEM_NAME) | tr '[A-Z]' '[a-z]')
ifeq ($(findstring cygwin,$(BUILD_SYSTEM_NAME)),cygwin)
BUILD_SYSTEM := OS_CYGWIN
else ifeq ($(findstring linux,$(BUILD_SYSTEM_NAME)),linux)
BUILD_SYSTEM := OS_UNIX
else
$(error "Not support build system '$(BUILD_SYSTEM_NAME)'")
endif

#增加编译参数和全局宏定义:
#平台宏(BUILD_SYSTEM): OS_CYGWIN,OS_UNIX
#产品宏(PRODUCT): PRODUCT_XXX,默认产品名称为DEFAULT
ifeq ($(PRODUCT),DEFAULT)
DEFAULT_CC_FLAGS := -D$(BUILD_SYSTEM) -Wall -g
else
DEFAULT_CC_FLAGS := -DPRODUCT_$(PRODUCT) -D$(BUILD_SYSTEM) -Wall -g
endif

#增加编译器及平台特定的一些定义
ifneq ($(HOST_FLAGS),)
DEFAULT_CC_FLAGS += $(HOST_FLAGS)
endif

#定义编译时用到目录(必须是绝对路径)及文件
BUILD_OUTPUT_PATH := $(PROJECT_ROOT)/build/local/$(PRODUCT)
DEFAULT_BIN_PATH := $(BUILD_OUTPUT_PATH)/bin
DEFAULT_LIB_PATH := $(BUILD_OUTPUT_PATH)/lib
DEFAULT_INC_PATH := $(BUILD_OUTPUT_PATH)/include
BUILD_PATH := $(PROJECT_ROOT)/build
CLEAR_VARS := $(BUILD_PATH)/core/clear_vars.mk
BUILD_DEPFILE := $(BUILD_PATH)/core/dep_file.mk
BUILD_EXECUTABLE := $(BUILD_PATH)/core/build_executable.mk
BUILD_LIBRARY := $(BUILD_PATH)/core/build_library.mk
BUILD_FROMSRC := $(BUILD_PATH)/core/build_fromsrc.mk
BUILD_FROMTAR := $(BUILD_PATH)/core/build_fromtar.mk
BUILD_QTPROJECT := $(BUILD_PATH)/core/build_qtproject.mk
CLEAN_BUILD := $(BUILD_PATH)/core/clean_build.mk
BUILD_RESOURCE := $(BUILD_PATH)/core/build_resource.mk
BUILD_TOOLS := $(BUILD_PATH)/core/build_tools.mk

SOURCES_PATH := $(PROJECT_ROOT)/$(SOURCES_DIR)
TARGET_MAKEFILE := $(SOURCES_PATH)/$(TARGET_NAME).mk

# OBJECT_PATH:创建存放目标的目录,目录名称与源码目录名称一致
OBJECT_PATH := $(BUILD_OUTPUT_PATH)/objects/$(SOURCES_DIR)
ifeq ($(wildcard $(OBJECT_PATH)),)
$(shell mkdir -p $(OBJECT_PATH))
$(shell chmod a+wx -R $(BUILD_OUTPUT_PATH))
endif

BUILD_INSTALL_PATH := $(PROJECT_ROOT)/output/$(PRODUCT)
ifeq ($(wildcard $(BUILD_INSTALL_PATH)),)
$(shell mkdir -p $(BUILD_INSTALL_PATH))
$(shell chmod a+wx -R $(BUILD_INSTALL_PATH))
endif

define my-dir
$(strip $(SOURCES_PATH))
endef

ifeq ($(wildcard $(TARGET_MAKEFILE)),)
$(error "No $(TARGET_MAKEFILE) in '$(SOURCES_DIR)',stop build!")
else
include $(TARGET_MAKEFILE)
endif



