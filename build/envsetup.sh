#!/bin/sh
##############################################################
##  Author: cblock@126.com  FergusZeng 2016 ShenZhen,China  ##
##  WARNING:本文件为mbuild系统环境配置脚本，修改时务必谨慎! ##
##############################################################
PROJECT_ROOT=$PWD

#检查环境是否正确
if [ -f "$PROJECT_ROOT/build/core/mbuild-core.sh" -a -f "$PROJECT_ROOT/build/envsetup.sh" -a -f "$PROJECT_ROOT/build/core/main.mk" ]; then
echo -e "\033[33m\033[1m"
cat <<EOF
-------------------------------------------------------------------------------
  [ mbuild system help ] <20190918-v2.1>
  --------------------------------------
  Invoke "source ./build/envsetup.sh" from your shell to add the following 
  functions to your environment:
- mbuild_setup: set product and host for build envoriment.
- mbuild_project <directory> <target> [exec|lib|res|qt] : create project for target in the directory.
- mbuild_make <directory> <target> <linktype> : make target from the directory.
- mbuild_clean <directory> <target> : make clean target from the directory.
- mbuild_remake <directory> <target> <linktype> : mclean and mbuild.
- mbuild_auto: generate autobuild.sh for automatic building.
- mbuild_automake <linktype> : build all with linktype
- mbuild_autoclean: clean all targets
- mbuild_quit: quit and reset environment 
  -----------------------------------------------------------------------------
  1. Commands only can work well once you hava a <target>.mk file in the target
     source directory.
  2. <linktype> has two type: static and shared, if you donot specify this 
     value, you should set LOCAL_LIB_TYPE in <target>.mk 
-------------------------------------------------------------------------------
EOF
echo -e "\033[0m"
else
	echo -e '\033[33m\033[1mYou should invoke ". build/envsetup.sh" in project top directory!'
	echo -e "come back to $HOME\033[0m"
	exit 1
fi

source $PROJECT_ROOT/build/core/mbuild-core.sh
source $PROJECT_ROOT/build/profile.sh

function mbuild_setup()
{
    PS1="\[\e[1;33m\]mbuild@\[\e[0m\]\[\e[1;35m\]$HOST_FNAME:\[\e[0m\]\[\e[1;36m\]\W@\[\e[0m\]"
    SYSROOT_PATH=
    # 该命令可以带参数,用于指定product
    if [ $# -eq 0 ];then
        set_product
	elif [ $# -eq 1 ];then
        set_product $1
    else
        echo "[usage]"
        echo "mbuild_setup"
		echo "mbuild_setup <product-id>"
        return
    fi

    # 创建目标文件夹
    if [ ! -d "$PROJECT_ROOT/build/local/output-$PRODUCT/bin" ];then
        mkdir -p $PROJECT_ROOT/build/local/$PRODUCT/bin
    fi
    if [ ! -d "$PROJECT_ROOT/build/local/output-$PRODUCT/lib" ];then
        mkdir -p $PROJECT_ROOT/build/local/$PRODUCT/lib
    fi
    if [ ! -d "$PROJECT_ROOT/build/local/output-$PRODUCT/include" ];then
        mkdir -p $PROJECT_ROOT/build/local/$PRODUCT/include
    fi
	if [ ! -d "$PROJECT_ROOT/build/local/output-$PRODUCT/include/wow_base" ];then
        mkdir -p $PROJECT_ROOT/build/local/$PRODUCT/include/wow_base
    fi	
	if [ ! -d "$PROJECT_ROOT/build/local/output-$PRODUCT/includ/wow_hal" ];then
        mkdir -p $PROJECT_ROOT/build/local/$PRODUCT/include/wow_hal
    fi	
	if [ ! -d "$PROJECT_ROOT/build/local/output-$PRODUCT/include/wow_iot" ];then
        mkdir -p $PROJECT_ROOT/build/local/$PRODUCT/include/wow_iot
    fi

    PNAME="$(echo $PRODUCT| tr '[:lower:]' '[:upper:]')"
    if [ "$PNAME" == "DEFAULT" ];then
		echo -e "==>mbuild setup product default (not set).\033[0m"
	else
        echo -e "==>mbuild setup product: $PRODUCT ."
    fi
    if [ "$HOST " == " " ];then
        echo -e "==>mbuild setup toolchain: gcc"
    else
        echo -e "==>mbuild setup toolchain: $HOST-gcc"
    fi
    echo -e "==>mbuild setup ok."
}

function mbuild_quit()
{
    PS1="\u@\h:\W\$"
}

function mbuild_auto()
{
    if [ ! -f "$PROJECT_ROOT/autobuild.sh" ];then
        cp $PROJECT_ROOT/build/templates/autobuild.sh.in $PROJECT_ROOT/autobuild.sh
        echo "Generated $PROJECT_ROOT/autobuild.sh"
    else
        echo "autobuild.sh allready exists!"
    fi
    echo "you can execute autobuild.sh for automatic building now."
}
