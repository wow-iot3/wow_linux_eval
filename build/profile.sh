#!/bin/sh
###############################################################################
##  Author: cblock@126.com  FergusZeng 2016 ShenZhen,China 
##  本文件为工程自定义脚本,可按需求来自定义工程类型。
##  祝使用愉快！
###############################################################################

:<< EOF # 这里开始是注释~~~
set_product 主要设置四个变量:
PRODUCT --- 标识产品型号，在源码中可以使用此处定义的产品名称作为宏定义，达到隔离代码的目的。
				  #ifdef PRODUCT_FOO
                  ...
                  #else
                  ...
                  #endif
HOST    --- 目标程序运行的主机的编译工具
FLAGS   --- 特定编译器的标志,会自动添加到CC_FLAGS中去
EOF
function set_product()
{
	if [ $# -eq 1 ];then
		n=$1
	else
		echo -e "\033[33m\033[1m"
		echo -e "----------product list-----------"
		echo -e " 0: Ubuntu-20.04"
		echo -e " 1: ARM_A"
		echo -e " 2: ARM_B"
		echo -e " 3: ARM_C"
		echo -e " 4: AARCH64"
		echo -e "----------------------------------"
		echo -e "input you selection:\033[0m"
		read n
	fi

    case $n in
	"0")
		PRODUCT=UBUNTU
		HOST=
		FLAGS= 
		;;
	"1")
		PRODUCT=ARM_A
		HOST=arm-linux
		FLAGS=
		;;
	"2")
		PRODUCT=ARM_B
		HOST=arm-linux-gnueabihf
		FLAGS=
		;;
	"3")
		PRODUCT=ARM_C
		HOST=arm-linux-gnueabihf
		FLAGS=
		;;
	"4")
		PRODUCT=AARCH64
		HOST=aarch64-linux-gnu
		FLAGS=
		;;
	"99")
	    while true
	    do
			echo -e "please input your product:"
    		read product
    		echo -e "please input your toolchain:"
    		read compile
    		echo -e "you specify the product: $product toolchain: $compile. Are you sure?(Y/n)"
    		read sure
    		if [ $sure = "Y" -o  $sure = "y" ];then
				PRODUCT=$product
    		    HOST=$compile
    		    break
            else
                echo -e "\033[31m\033[1myou must specify a build toolchain($compile) for product:$product\033[0m"    		
    		fi
	    done
		;;	
	esac
}
