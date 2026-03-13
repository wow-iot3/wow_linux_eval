##############################################################
##  Author: cblock@126.com  FergusZeng 2016 ShenZhen,China  ##
##############################################################
#                     target.mk template
##############################################################
# LOCAL_PATH := $(call my-dir)
# include $(CLEAR_VARS)
# LOCAL_MODULE := demo-deploy
#
# LOCAL_RESOURCE_FILES:= \
#    $(LOCAL_PATH)/demo.conf \
#    $(LOCAL_PATH)/images/
#    
# LOCAL_RESOURCE_DEST := 
# include $(BUILD_RESOURCE)
##############################################################

ifeq ($(LOCAL_RESOURCE_DEST),)
LOCAL_RESOURCE_DEST := $(DEFAULT_BIN_PATH)
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
	@for f in $(LOCAL_RESOURCE_FILES); do \
	if [ -f $$f ];then \
		install -t $(LOCAL_RESOURCE_DEST) $$f; \
		echo -e "\033[33m\033[1m==>install file:$$f, dst:$(LOCAL_RESOURCE_DEST)\033[0m"; \
	elif [ -d $$f ];then \
		cp -pR $$f $(LOCAL_RESOURCE_DEST); \
		echo -e "\033[33m\033[1m==>install dir:$$f, dst:$(LOCAL_RESOURCE_DEST)\033[0m"; \
	else \
	echo -e "\033[31m\033[1m==>not install resource:$$f\033[0m"; \
	fi;\
	done
	@echo -e "\033[33m\033[1m==>build $(SOURCES_DIR) $(LOCAL_MODULE) finished.\033[0m"

.PHONY:clean
clean:
	@for f in $(LOCAL_RESOURCE_FILES); do \
	INSTALLED_RESOURCE_FILE=$(LOCAL_RESOURCE_DEST)/`basename $$f`; \
	if [ -f $$INSTALLED_RESOURCE_FILE ];then \
		rm $$INSTALLED_RESOURCE_FILE; \
		echo -e "\033[33m\033[1m==>uninstall file:$$INSTALLED_RESOURCE_FILE\033[0m"; \
	elif [ -d $$INSTALLED_RESOURCE_FILE ];then \
		rm -rf $$INSTALLED_RESOURCE_FILE; \
		echo -e "\033[33m\033[1m==>uninstall dir:$$INSTALLED_RESOURCE_FILE\033[0m"; \
	fi;\
	echo -e "\033[33m\033[1m==>clean $(SOURCES_DIR) $(LOCAL_MODULE) finished.\033[0m"; \
	done

