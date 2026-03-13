##############################################################
##  Author: cblock@126.com  FergusZeng 2016 ShenZhen,China  ##
##############################################################
DEP_FILE := $(MODULE_FILE).dep
SOURCE_FILES := 
$(DEP_FILE):
	@echo "Generating new dependency file...";
	@-rm -f $(DEP_FILE)
	@for f in $(LOCAL_SRC_FILES); do \
	OBJ=$(OBJECT_PATH)/`basename $$f|sed -e 's/\.cpp/\.o/' -e 's/\.cxx/\.o/' -e 's/\.cc/\.o/' -e 's/\.c/\.o/'`; \
        echo $$OBJ: $(SOURCES_PATH)/$$f>> $(DEP_FILE); \
		FILENAME=`basename $$f`; \
 		EXTNAME="$${FILENAME##*.}"; \
		if [ "$$EXTNAME" = "c" ];then \
		echo '	$(CC) $$(ALL_CFLAGS)  -c -g -o $$@ $$^'>> $(DEP_FILE);else \
		echo '	$(CXX) $$(ALL_CXXFLAGS)  -c -g -o $$@ $$^'>> $(DEP_FILE);fi; \
        done

include $(DEP_FILE)

# OBJECT_FILES:目标所依赖的所有.o文件
OBJECT_FILES:=$(addprefix $(OBJECT_PATH)/, $(addsuffix .o, $(basename $(notdir $(LOCAL_SRC_FILES)))))