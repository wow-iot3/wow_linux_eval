# 新增 Product 平台操作指南

## 1. 整体流程概览

```
┌──────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│  1. profile.sh   │────▶│  2. wow_hal BSP  │────▶│  3. hal_dev_conf │
│  定义平台/工具链  │     │  板级支持包实现   │     │  设备节点配置     │
└──────────────────┘     └──────────────────┘     └──────────────────┘
         │                                                  │
         ▼                                                  ▼
┌──────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│  4. 源码中使用    │────▶│  5. 构建验证      │────▶│  6. 运行验证      │
│  PRODUCT_XXX 宏  │     │  mbuild_setup    │     │  目标板测试       │
└──────────────────┘     └──────────────────┘     └──────────────────┘
```

## 2. 涉及文件清单

| 序号 | 文件路径 | 操作 | 必须 |
|:---:|----------|:----:|:---:|
| 1 | `build/profile.sh` | 修改 | ✅ |
| 2 | `wow_hal/src/board/<platform>.c` | 新增 | ✅ |
| 3 | `wow_hal/src/hal_dev_conf.h` | 修改 | ✅ |
| 4 | `wow_hal/src/hal_env.h` | 修改 | ⚠️ 视情况 |
| 5 | 业务源码中的 `#ifdef PRODUCT_XXX` 分支 | 修改 | ⚠️ 视情况 |
| 6 | `wow_hal/wow_hal_src.mk` | 修改 | ⚠️ 视情况 |

---

## 3. 详细步骤

### Step 1：在 `profile.sh` 中注册新平台

编辑 `build/profile.sh`，在 `set_product()` 函数的 `case` 语句中添加新分支。

#### 3.1.1 确定三要素

| 要素 | 变量 | 说明 | 如何获取 |
|------|------|------|----------|
| **产品标识** | `PRODUCT` | 唯一大写标识，自动生成 `-DPRODUCT_XXX` 宏 | 自行命名，需唯一 |
| **工具链前缀** | `HOST` | 交叉编译器 `xxx-gcc` 去掉 `-gcc` 后的前缀 | `which arm-linux-gnueabihf-gcc` |
| **编译标志** | `FLAGS` | 平台特定的 CPU 架构 / FPU / ABI 参数 | 参考芯片/BSP 文档 |

#### 3.1.2 添加 case 分支

```bash
# build/profile.sh

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
        echo -e " 5: IMX6ULL"            # ← 新增菜单项
        echo -e "----------------------------------"
        echo -e "input you selection:\033[0m"
        read n
    fi

    case $n in
    # ... 已有平台省略 ...

    "5")                                   # ← 新增 case 分支
        PRODUCT=IMX6ULL                    # 产品标识（大写）
        HOST=arm-linux-gnueabihf           # 交叉编译器前缀
        FLAGS="-march=armv7-a -mfpu=neon -mfloat-abi=hard"  # 平台编译标志
        ;;

    # ... 99: 自定义平台保持不变 ...
    esac
}
```


#### 3.1.4 宏定义传递链路

```
profile.sh                    mbuild-core.sh                 main.mk                    C 源码
PRODUCT=IMX6ULL  ──────▶  PNAME=$(echo $PRODUCT|        DEFAULT_CC_FLAGS +=         #ifdef PRODUCT_IMX6ULL
                           tr '[a-z]' '[A-Z]')  ──────▶  -DPRODUCT_IMX6ULL  ──────▶    // IMX6ULL 平台代码
                                                                                     #endif
```

> ⚠️ **注意**：`main.mk` 中有如下逻辑，当 `PRODUCT=DEFAULT` 时不会生成 `PRODUCT_` 宏前缀：
> ```makefile
> ifeq ($(PRODUCT),DEFAULT)
> DEFAULT_CC_FLAGS := -D$(BUILD_SYSTEM) -Wall -g
> else
> DEFAULT_CC_FLAGS := -DPRODUCT_$(PRODUCT) -D$(BUILD_SYSTEM) -Wall -g
> endif
> ```
> 因此产品标识 **不要** 命名为 `DEFAULT`。

---

#### Step 2：创建板级支持包（BSP）

在 `wow_hal/src/board/` 下新建平台对应的 BSP 文件。

##### 3.2.1 新建 BSP 文件

```bash
touch wow_hal/src/board/imx6ull.c
```

##### 3.2.2 BSP 文件模板

```c
/**
 * @file    imx6ull.c
 * @brief   i.MX6ULL 板级支持包 (BSP)
 * @note    通过 PRODUCT_IMX6ULL 宏条件编译
 */

#ifdef PRODUCT_IMX6ULL

#include "wow_hal_init.h"
#include "hal_dev_conf.h"

/**
 * @brief  板级硬件初始化
 * @return 0:成功, <0:失败
 */
int wow_hal_board_init(void)
{
    /* GPIO 初始化 */
    /* 时钟配置 */
    /* 外设使能 */
    return 0;
}

/**
 * @brief  板级硬件去初始化
 */
void wow_hal_board_deinit(void)
{
    /* 资源释放 */
}

#endif /* PRODUCT_IMX6ULL */
```

##### 3.2.3 条件编译隔离

所有 BSP 文件共同参与编译，通过 `#ifdef PRODUCT_XXX` 确保**同一时刻只有一个 BSP 文件生效**：

```
wow_hal/src/board/
├── ubuntu-2004.c      #ifdef PRODUCT_UBUNTU   ... #endif
├── imx6ull.c          #ifdef PRODUCT_IMX6ULL  ... #endif   ← 新增
└── (将来更多平台)
```

---

#### Step 3：配置设备节点

编辑 `wow_hal/src/hal_dev_conf.h`，添加新平台的设备路径定义。

```c
/* wow_hal/src/hal_dev_conf.h */

#ifndef __HAL_DEV_CONF_H__
#define __HAL_DEV_CONF_H__

/* ========== Ubuntu 20.04 平台 ========== */
#ifdef PRODUCT_UBUNTU
    #define HAL_USART_DEV_0     "/dev/ttyS0"
    #define HAL_USART_DEV_1     "/dev/ttyS1"
    #define HAL_USART_DEV_2     "/dev/ttyUSB0"
#endif

/* ========== ARM_A 平台 ========== */
#ifdef PRODUCT_ARM_A
    #define HAL_USART_DEV_0     "/dev/ttyS0"
    #define HAL_USART_DEV_1     "/dev/ttyS1"
#endif

/* ========== IMX6ULL 平台 (新增) ========== */
#ifdef PRODUCT_IMX6ULL
    #define HAL_USART_DEV_0     "/dev/ttymxc0"      /* 调试串口 */
    #define HAL_USART_DEV_1     "/dev/ttymxc1"      /* RS485 通信口 */
    #define HAL_USART_DEV_2     "/dev/ttymxc2"      /* RS232 扩展口 */
    
    /* 其他设备节点 (按需) */
    // #define HAL_SPI_DEV_0    "/dev/spidev0.0"
    // #define HAL_I2C_DEV_0    "/dev/i2c-0"
    // #define HAL_GPIO_CHIP    "/dev/gpiochip0"
#endif

#endif /* __HAL_DEV_CONF_H__ */
```

---

#### Step 4：更新 HAL 构建文件（如需要）

如果新增了 BSP 文件，检查 `wow_hal/wow_hal_src.mk` 中的 `LOCAL_SRC_FILES` 是否已包含 `board/` 目录下所有文件。

```makefile
# wow_hal/wow_hal_src.mk

LOCAL_SRC_FILES := \
    $(LOCAL_PATH)/src/wow_hal_init.c \
    $(LOCAL_PATH)/src/board/ubuntu-2004.c \
    $(LOCAL_PATH)/src/board/imx6ull.c \        # ← 新增 BSP 文件
    $(LOCAL_PATH)/src/usart/wow_usart.c
```

> 💡 **小技巧**：由于每个 BSP 文件通过 `#ifdef PRODUCT_XXX` 保护，所有 BSP 文件可以同时列入编译列表，不会产生符号冲突。非当前平台的 BSP 文件编译后为空 `.o`。

---

#### Step 6：验证构建

##### 6.1 安装交叉编译工具链

```bash
# 以 IMX6ULL 为例（arm-linux-gnueabihf）
sudo apt-get install -y gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# 验证安装
arm-linux-gnueabihf-gcc --version
```

##### 6.2 初始化并选择新平台

```bash
# 初始化 mbuild 环境
. build/envsetup.sh

# 选择新平台（交互式，选择编号 5）
mbuild_setup
# 或直接指定编号
mbuild_setup 5
```

预期输出：

```
==>mbuild setup product: IMX6ULL .
==>mbuild setup toolchain: arm-linux-gnueabihf-gcc
==>mbuild setup ok.
```

##### 6.3 编译第三方库

```bash
source 3rdparty/autobuild.sh
mbuild_automake
```

##### 6.4 编译核心库

```bash
mbuild_remake wow_base wow_base_src shared
mbuild_remake wow_hal  wow_hal_src  shared
mbuild_remake wow_iot  wow_iot_src  shared
```

##### 6.5 检查产物

```bash
ls -la build/local/IMX6ULL/lib/
# 应输出：
# libwowbase.so -> libwowbase.so.0.0.2
# libwowbase.so.0.0.2
# libwowhal.so  -> libwowhal.so.0.0.2
# libwowhal.so.0.0.2
# libwowiot.so  -> libwowiot.so.0.0.2
# libwowiot.so.0.0.2

ls -la output/IMX6ULL/
# 应输出最终发布文件

# 验证二进制文件架构
file output/IMX6ULL/libwowbase.so
# 预期：ELF 32-bit LSB shared object, ARM, EABI5, ...
```

##### 6.6 编译并运行测试（目标板）

```bash
# 编译测试程序
mbuild_remake wow_base wow_base_test
mbuild_remake wow_hal  wow_hal_test

# 部署到目标板
scp -r output/IMX6ULL/* root@192.168.1.100:/opt/wow/

# 目标板上执行
ssh root@192.168.1.100
cd /opt/wow
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
./wow_base_test
./wow_hal_test
```

---

### 4. 目录产物验证

新平台构建成功后，会自动生成以下隔离的目录结构：

```
build/local/IMX6ULL/           # ← 新平台独立目录
├── bin/
│   ├── wow_base_test
│   └── wow_hal_test
├── lib/
│   ├── libwowbase.so → libwowbase.so.0.0.2
│   ├── libwowbase.so.0.0.2
│   ├── libwowhal.so → libwowhal.so.0.0.2
│   ├── libwowhal.so.0.0.2
│   ├── libwowiot.so → libwowiot.so.0.0.2
│   └── libwowiot.so.0.0.2
├── include/
│   ├── wow_base/
│   ├── wow_hal/
│   └── wow_iot/
└── objects/
    ├── wow_base/
    ├── wow_hal/
    └── wow_iot/

output/IMX6ULL/                # ← 最终发布目录
├── libwowbase.so
├── libwowhal.so
├── libwowiot.so
├── wow_base_test
└── wow_hal_test
```

> 不同平台的产物 **完全隔离**，可同时维护多个平台的构建结果，互不干扰。

---
