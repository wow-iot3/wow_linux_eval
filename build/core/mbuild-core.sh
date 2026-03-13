#!/bin/sh
##############################################################
##  Author: cblock@126.com  FergusZeng 2016 ShenZhen,China  ##
##  WARNING:本文件为mbuild系统核心脚本文件，修改时务必谨慎! ##
##############################################################
CPU_NUM=4
HOST=
HOST_FLAGS=
HOST_FNAME=`uname -m`
PRODUCT_NAME=default
BUILD_SYSTEM=`uname -o | tr '[A-Z]' 'a-z'`;
if [[ $BUILD_SYSTEM =~ "cygwin" ]];then
	BUILD_SYSTEM=OS_CYGWIN
else
	BUILD_SYSTEM=OS_UNIX
fi

function check_make_result()
{
	TIME_STRING=`date`
	if [ "$AUTO_BUILD" == "yes" ];then
		if [ "$3" -ne 0 ];then
			echo "$TIME_STRING | $1 $2 ==> ERROR on $4" >> $PROJECT_ROOT/autobuild.log
			exit
		else
			echo "$TIME_STRING | $1 $2 ==> SUCCESS on $4" >> $PROJECT_ROOT/autobuild.log
		fi
	fi
	if [ "$3" -ne 0 ]; then
    	while true
    	do
    		echo -e "\033[31m\033[1m$TIME_STRING | $1 $2 ==> ERROR on $4, press Ctrl+C to quit!\033[0m";
    		read n
    	done	
    fi
}
function mbuild_make()
{
    if [ $# -lt 2 -o $# -gt 3 ];then
        echo "[usage] mbuild_make <directory> <target> <libtype>"
	    return
	elif [ $# = 3 ];then
	    LINK_TYPE=$3
	else
	    LINK_TYPE=
    fi
	PNAME="$(echo $PRODUCT | tr '[:lower:]' '[:upper:]')"
	if [ "$PNAME" == "DEFAULT" ];then
		echo -e "\033[33m\033[1mmbuild_make $1 $2 for HOST: $HOST PRODUCT: not set\033[0m"
	else
    	echo -e "\033[33m\033[1mmbuild_make $1 $2 for HOST: $HOST PRODUCT: $PRODUCT\033[0m"
	fi
    make pre-build -f build/core/main.mk SOURCES_DIR=$1 HOST_FLAGS="$HOST_FLAGS" TARGET_NAME=$2 LINK_TYPE=$LINK_TYPE PROJECT_ROOT=$PROJECT_ROOT HOST=$HOST PRODUCT=$PNAME
	check_make_result $1 $2 $? pre-build
    make all -f build/core/main.mk -j$CPU_NUM SOURCES_DIR=$1 HOST_FLAGS="$HOST_FLAGS" TARGET_NAME=$2 LINK_TYPE=$LINK_TYPE PROJECT_ROOT=$PROJECT_ROOT HOST=$HOST PRODUCT=$PNAME
	check_make_result $1 $2 $? build
	make post-build -f build/core/main.mk SOURCES_DIR=$1 HOST_FLAGS="$HOST_FLAGS" TARGET_NAME=$2 LINK_TYPE=$LINK_TYPE PROJECT_ROOT=$PROJECT_ROOT HOST=$HOST PRODUCT=$PNAME
	check_make_result $1 $2 $? post-build
}

function mbuild_clean()
{
	if [ $# -lt 2 -o $# -gt 3 ];then
		echo "[usage] mbuild_clean <directory> <target>"
		return
	fi
	PNAME="$(echo $PRODUCT | tr '[:lower:]' '[:upper:]')"
	if [ "$PNAME" == "DEFAULT" ];then
		echo -e "\033[33m\033[1mmbuild_clean $1 $2 for HOST: $HOST PRODUCT: not set\033[0m"
	else
    	echo -e "\033[33m\033[1mmbuild_clean $1 $2 for HOST: $HOST PRODUCT: $PNAME\033[0m"
	fi
	make clean -f build/core/main.mk -j$CPU_NUM SOURCES_DIR=$1 TARGET_NAME=$2 PROJECT_ROOT=$PROJECT_ROOT HOST=$HOST PRODUCT=$PNAME
}

function mbuild_remake()
{
	USAGE_INFO="[usage] mbuild_remake <directory> <target> <libtype>"
    if [ $# -lt 2 -o $# -gt 3 ];then
        echo $USAGE_INFO
		return
    fi
	PNAME="$(echo $PRODUCT | tr '[:lower:]' '[:upper:]')"
    mbuild_clean $1 $2
    mbuild_make $1 $2 $3
}

# 创建子工程模块
function mbuild_project()
{
	USAGE_INFO="[usage] mbuild_project <directory> <target> [exec|lib|res|qt|fromsrc|fromtar]"
    if [ $# -ne 3 ];then
        echo $USAGE_INFO
	    return
	else
		DIR=$1
		PROJ=$2
	    TYPE=$3
    fi
	# 检查参数
	if [ "$TYPE" == "exec" -a "$TYPE" == "lib" -a "$TYPE" == "res" -a "$TYPE" == "qt" -a "$TYPE" == "fromsrc" -a "$TYPE" == "fromtar" ];then
		echo $USAGE_INFO
	    return
	fi
	# 工程已经存在,则返回
	if [ -f $DIR/$PROJ.mk ];then
		echo "project $DIR/$PROJ.mk allready exist!"
		return
	fi
    # 目录不存在则创建
	if [ ! -d $DIR ];then
		mkdir -p $DIR
	fi
	# 拷贝工程模板
	if [ "$TYPE" == "exec" ];then
        cp build/templates/target-exec.mk.in $DIR/$PROJ.mk
	elif [ "$TYPE" == "lib" ];then
    	cp build/templates/target-lib.mk.in $DIR/$PROJ.mk
	elif [ "$TYPE" == "res" ];then
    	cp build/templates/target-res.mk.in $DIR/$PROJ.mk
	elif [ "$TYPE" == "qt" ];then
    	cp build/templates/target-qt.mk.in $DIR/$PROJ.mk
	elif [ "$TYPE" == "fromsrc" ];then
    	cp build/templates/from-src.mk.in $DIR/$PROJ.mk
		cp build/templates/Makefile.mbuild.in $DIR/Makefile.mbuild
	elif [ "$TYPE" == "fromtar" ];then
    	cp build/templates/from-tar.mk.in $DIR/$PROJ.mk
	else
		echo $USAGE_INFO
	    return
	fi
	# 编辑模板
	LINE_NUM=`sed -n '/LOCAL_MODULE/=' $DIR/$PROJ.mk`
	sed -i "$LINE_NUM""c\LOCAL_MODULE := $PROJ" $DIR/$PROJ.mk
	echo "create project $DIR/$PROJ.mk ok."
}

# 创建CMakeLists.txt文件
function mbuild_cmake_project()
{
	USAGE_INFO="[usage] mbuild_cmake_project <directory>"
    if [ $# -ne 1 ];then
        echo $USAGE_INFO
	    return
	else
		DIR=$1
    fi

	# 工程已经存在,则返回
	if [ -f $DIR/CMakeLists.txt ];then
		echo "project $DIR/CMakeLists.txt allready exist!"
		return
	fi

    # 目录不存在则创建
	if [ ! -d $DIR ];then
		mkdir -p $DIR
	fi

	# 拷贝工程模板
    cp build/templates/CMakeLists.txt.in $DIR/CMakeLists.txt
	echo "create $DIR/CMakeLists.txt ok."
}

# 构建cmake工程
function mbuild_cmake_make()
{
	USAGE_INFO="[usage] mbuild_cmake_make <target>"
	if [ $# -ne 1 ];then
        echo $USAGE_INFO
	    return
	else
		TARGET=$1
		BUILD_DIR=$PROJECT_ROOT/output-cmake
    fi
	
	if [ ! -d $PROJECT_ROOT/$TARGET ];then
		echo "not CMakeLists.txt in $TARGET"
		return
	fi
	
	if [ ! -d $BUILD_DIR ];then
		mkdir -p $BUILD_DIR
	fi

	if [ ! -d $BUILD_DIR/$TARGET ];then
		mkdir -p $BUILD_DIR/$TARGET
	fi

	cd $BUILD_DIR/$TARGET
	cmake $PROJECT_ROOT/$TARGET
	make
	make install DESTDIR=$BUILD_DIR/install
	cd $PROJECT_ROOT
}

# 清理cmake工程
function mbuild_cmake_clean()
{
	USAGE_INFO="[usage] mbuild_cmake_clean <target>"
	if [ $# -ne 1 ];then
        echo $USAGE_INFO
	    return
	else
		TARGET=$1
		BUILD_DIR=$PROJECT_ROOT/output-cmake
    fi
	if [ ! -d $BUILD_DIR ];then
		echo "no cmake build directory: $BUILD_DIR, please run mbuild_cmake_make first!"
		return
	fi
	if [ ! -d $BUILD_DIR/$TARGET ];then
		echo "not cmake target exist: $TARGET, please run mbuild_cmake_make first!"
		return
	fi
	cd $BUILD_DIR/$TARGET
	make clean
	cd $PROJECT_ROOT
}
