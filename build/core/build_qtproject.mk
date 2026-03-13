##############################################################
##  Author: cblock@126.com  FergusZeng 2016 ShenZhen,China  ##
##############################################################
#                     target.mk template
##############################################################
# LOCAL_PATH := $(call my-dir)
# include $(CLEAR_VARS)
# #在编译QT应用时,LOCAL_MODULE是mk所在目录的名称(相对于工程根目录)
# LOCAL_MODULE := qtdemo
# 
# #qmake所在路径,编译时根据此路径查找相关工具
# QMAKE_PATH := /opt/qt-5.2.0/bin
# 
# #在编译过程中mbuild系统会这些变量传递到QT的工程文件(*.pro)中.
# LOCAL_CFLAGS :=
# LOCAL_LDFLAGS :=
# LOCAL_LIB_PATHS := \
# 	$(LOCAL_PATH)/lib
# LOCAL_INC_PATHS := \
# 	$(LOCAL_PATH)/inc
# 
# #在编译QT应用时,LOCAL_SRC_FILES指当前目录下(.mk文件所在目录)的QT工程名称,
# #注意：这里的名称不需要后缀.pro,如demo.pro只需要填写demo即可!!!
# LOCAL_SRC_FILES := \
#     demo
# 
# #如果qt工程包含子工程,则需要指定子工程目标文件路径:
# LOCAL_TARGETS := \
#     subproject-a/aaa \
#     subproject-b/bbb
# 
# include $(BUILD_QTPROJECT)
##############################################################

ifeq ($(QMAKE_PATH),)
QMAKE=qmake
else
ifeq ($(wildcard $(QMAKE_PATH)),)
$(error "QMAKE_PATH set error, cannot find $(QMAKE_PATH)!")
endif
QMAKE := $(QMAKE_PATH)/qmake
endif

ALL_LIB_PATHS :=$(foreach n,$(LOCAL_LIB_PATHS),-L$(n))

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

#修改pro文件:在文件最后添加库和头文件路径
# LIBS += $(LOCAL_LDFLAGS)
# LIBS += -L
# INCLUDEPATH += $(LOCAL_INC_PATHS)

.PHONY:all
all:
	@for f in $(LOCAL_TARGETS); do \
		if [ -f $(LOCAL_PATH)/$$f.pro ];then \
			sed -i '/##--START--/,/##--END--/d' $(LOCAL_PATH)/$$f.pro; \
			echo "##--START--" >> $(LOCAL_PATH)/$$f.pro; \
			echo "INCLUDEPATH += $(DEFAULT_INC_PATH) $(LOCAL_INC_PATHS)" >> $(LOCAL_PATH)/$$f.pro; \
			echo 'LIBS += -L"$(DEFAULT_LIB_PATH)" $(ALL_LIB_PATHS) $(LOCAL_LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_CXXFLAGS)'  >> $(LOCAL_PATH)/$$f.pro; \
			echo "##--END--" >> $(LOCAL_PATH)/$$f.pro; \
			echo -e "\033[33m\033 add external libs for qt project ok: $(LOCAL_PATH)/$$f.pro \033[0m" ;\
		fi;\
		done
	@for f in $(LOCAL_SRC_FILES); do \
		CURPATH=$$PWD; \
		if [ ! -e $(SOURCES_PATH)/$$f.pro ]; then \
			while true; \
			do \
			echo -e "\033[31m\033 No qt project file: $(SOURCES_PATH)/$$f.pro !\033[0m"; \
			echo -e "\033[31m\033 Error occur on build $(LOCAL_MODULE), press Ctrl+C to quit!\033[0m"; \
			read n; \
			done;\
		fi;\
		if [ ! -d $(SOURCES_PATH)/../build-$(HOST_FNAME)-$$f ];then \
			mkdir $(SOURCES_PATH)/../build-$(HOST_FNAME)-$$f; \
		fi;\
		cd $(SOURCES_PATH)/../build-$(HOST_FNAME)-$$f; \
		$(QMAKE) $(SOURCES_PATH)/$$f.pro; \
		make; \
		if [ "$$?" -ne 0 ]; then \
			while true; \
			do \
			echo -e "\033[31m\033 build qt application: $(SOURCES_PATH)/$$f failed!\033[0m"; \
			echo -e "\033[31m\033 Error occur on build $(LOCAL_MODULE), press Ctrl+C to quit!\033[0m"; \
			read n; \
			done;\
		fi;\
		if [ -f $(SOURCES_PATH)/$$f ];then \
			install $(SOURCES_PATH)/$$f $(DEFAULT_BIN_PATH)/`basename $$f`; \
			echo -e "\033[33m\033 release ok: $(SOURCES_PATH)/$$f --> $(DEFAULT_BIN_PATH)\033[0m";\
		fi; \
		cd $$CURPATH; \
		done
	@for f in $(LOCAL_TARGETS); do \
		if [ -f $(SOURCES_PATH)/../build-$(HOST_FNAME)-$(LOCAL_MODULE)/$$f ];then \
			install $(SOURCES_PATH)/../build-$(HOST_FNAME)-$(LOCAL_MODULE)/$$f $(DEFAULT_BIN_PATH)/`basename $$f`; \
			echo -e "\033[33m\033 release ok: $(SOURCES_PATH)/../build-$(HOST_FNAME)-$(LOCAL_MODULE)/$$f --> $(DEFAULT_BIN_PATH)\033[0m" ;\
		fi;\
		if [ -f $(LOCAL_PATH)/$$f.pro ];then \
		sed -i '/##--START--/,/##--END--/d' $(LOCAL_PATH)/$$f.pro; \
		fi;\
		done
	@echo -e "\033[33m\033[1m==>build $(LOCAL_MODULE) successfully.\033[0m"

.PHONY: clean
clean:
	@for f in $(LOCAL_SRC_FILES); do \
		CURPATH=$$PWD; \
		if [ -d $(SOURCES_PATH)/../build-$(HOST_FNAME)-$$f ];then \
		cd $(SOURCES_PATH)/../build-$(HOST_FNAME)-$$f; \
		make clean; \
		fi; \
		cd $$CURPATH; \
        done
