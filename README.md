 # WOW Linux Eval

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux-green.svg)]()
[![Language](https://img.shields.io/badge/Language-C99-orange.svg)]()

## 一、项目介绍

**WOW Linux Eval** 是一套面向嵌入式 Linux 的物联网（IoT）基础平台框架，采用纯 C 语言（C99）编写，提供从硬件抽象、操作系统封装、通用数据结构、配置管理、工业通信协议到应用服务的完整软件栈。

项目采用 **四层分层架构**，覆盖数据采集、数据处理、数据转发、数据存储等物联网核心功能需求，最终输出动态共享库供上层应用链接调用。

### 架构总览

```
┌──────────────────────────────────────────────────────────┐
│            应用层：wow_app / wow_daemon / wow_show         │
├──────────────────────────────────────────────────────────┤
│         IoT 业务层：wow_iot (协议/配置/数据库/加密/...)     │
├──────────────────────────────────────────────────────────┤
│         基础平台层：wow_base (文件/网络/线程/数据结构/...)   │
├──────────────────────────────────────────────────────────┤
│         硬件抽象层：wow_hal (BSP / USART / ...)            │
├──────────────────────────────────────────────────────────┤
│         第三方库：libev 4.33 / mbedTLS 2.13.1              │
├──────────────────────────────────────────────────────────┤
│         Linux Kernel / POSIX API                          │
└──────────────────────────────────────────────────────────┘
```

### 模块说明

| 模块 | 说明 | 状态 |
|------|------|:----:|
| **wow_base** | 系统关联与基础接口封装库（文件、网络、线程、锁、数据结构、工具函数等） | ✅ |
| **wow_hal** | 硬件抽象层封装库，实现对不同硬件平台的兼容（BSP + 串口驱动） | ✅ |
| **wow_iot** | 物联网业务组件库（Modbus/IEC104 协议栈、SQLite 数据库、INI/JSON/XML 配置、CRC/RSA 加密、日志系统等） | ✅ |
| **wow_app** | 工程应用逻辑实现 | 🚧 |
| **wow_show** | 展示工程的各设备数据（LCD/Web 等） | 📋 |
| **wow_daemon** | 守护进程（进程监控/看门狗/服务生命周期管理） | 📋 |
| **wow_tools** | 简单应用工具集（如 CRC 追加工具） | ✅ |

> ✅ 已实现 &emsp; 🚧 开发中 &emsp; 📋 规划中

---

## 二、功能特性

### 🔌 工业协议支持

| 协议 | 传输层 | 主站 (Master) | 从站 (Slave) |
|------|--------|:---:|:---:|
| Modbus RTU | 串口 (RS485) | ✅ | ✅ |
| Modbus TCP | TCP/IP | ✅ | ✅ |
| IEC 60870-5-101 | 串口 | ✅ | ✅ |
| IEC 60870-5-104 | TCP/IP | ✅ | ✅ |

### 🧩 核心组件一览

| 分类 | 组件 |
|------|------|
| **数据结构** | 单向链表、KV 链表、字符串链表、环形缓冲区、哈希映射、哈希集合、位图、布隆过滤器、迭代器 |
| **配置解析** | INI（iniparser）、JSON（cJSON）、XML（ezxml） |
| **数据库** | SQLite3（内嵌源码，支持日志型/键值型数据表） |
| **加密/编码** | CRC8/16/32、RSA（mbedTLS）、Base64 |
| **网络通信** | TCP/UDP Socket 封装、FTP 客户端、NTP 时间同步 |
| **系统服务** | 线程、互斥锁、原子锁、条件变量、定时器、共享内存、IPC |
| **调试工具** | 日志系统、交互式 Shell、内存泄漏检测、调试信息输出 |
| **缓存** | 高性能共享内存缓存（进程间 Hash 索引 + 对象池） |

### 🏗️ 技术栈

| 类别 | 选型 |
|------|------|
| 编程语言 | C (C99) |
| 目标平台 | 嵌入式 Linux（已适配 X86、ARM_A、ARM_B） |
| 构建系统 | 自研 mbuild（GNU Make + Shell） |
| 第三方库 | libev 4.33（事件驱动）、mbedTLS 2.13.1（TLS/加密） |
| 内嵌库 | SQLite3、cJSON、ezxml、iniparser |
| 单元测试 | greatest 框架 |

---

## 三、目录结构

```
wow_linux_eval/
├── 3rdparty/           # 第三方库源码包及构建脚本
│   ├── libev-4.33.tar.gz
│   ├── libev.mk
│   ├── mbedtls-2.13.1.tar.gz
│   ├── libmbedtls.mk
│   └── autobuild.sh
├── build/              # 构建系统 (mbuild)
│   ├── envsetup.sh     #   环境初始化脚本
│   ├── profile.sh      #   平台配置 (X86/ARM_A/ARM_B/...)
│   ├── core/           #   构建核心规则 (.mk)
│   └── templates/      #   模块模板 (.mk.in)
├── doc/                # 项目文档
│   ├── code-rule.md    #   编码规范
│   ├── source-tree.md  #   源码树说明
│   ├── change-log.md   #   变更记录
│   ├── wow_base/       #   基础库专题文档
│   └── wow_blog/       #   模块 API 使用说明 (25+ 篇)
├── files/              # 资源文件
│   ├── config/         #   配置文件示例 (INI/JSON/XML)
│   └── database/       #   SQLite3 测试数据库
├── wow_base/           # 基础平台层
│   ├── inc/            #   公开头文件
│   ├── src/            #   源码实现
│   └── test/           #   单元测试 (20+ 套件)
├── wow_hal/            # 硬件抽象层
│   ├── inc/
│   ├── src/
│   │   └── board/      #   板级支持包 (BSP)
│   └── test/
├── wow_iot/            # IoT 业务组件层
│   ├── inc/
│   ├── src/
│   └── test/           #   单元测试 (22+ 套件)
├── wow_daemon/         # 守护进程 (规划中)
├── wow_show/           # 展示模块 (规划中)
├── wow_tools/          # 工具集
│   └── appendcrc/      #   CRC 追加工具
├── LICENSE
└── README.md
```

---

## 四、环境要求

### 4.1 系统要求

- **操作系统**：Linux（推荐 Ubuntu 18.04+）
- **编译器**：GCC 7.0+（支持 C99）
- **构建工具**：GNU Make 4.0+、Bash

### 4.2 依赖安装（以 Ubuntu/Debian 为例）

```bash
# 基础编译工具
sudo apt-get update
sudo apt-get install -y build-essential gcc make

# 可选：交叉编译工具链（ARM 平台）
# sudo apt-get install -y gcc-arm-linux-gnueabihf

# 可选：SQLite3 命令行工具（调试用）
sudo apt-get install -y sqlite3

# 可选：其他开发依赖
sudo apt-get install -y pkg-config autoconf automake libtool
```

> **注意**：项目所有核心第三方库（libev、mbedTLS、SQLite3、cJSON、ezxml）均已内嵌源码，无需系统级安装。

---

## 五、编译指南

本项目构建系统基于自研 **mbuild**，参考 [appkit](https://gitee.com/newgolo/appkit) 修改实现。

### 5.1 快速编译（完整流程）

```bash
# 1. 初始化构建环境
. build/envsetup.sh

# 2. 选择目标硬件平台
mbuild_setup
# 交互式选择: X86 / ARM_A / ARM_B 等
# 可通过 build/profile.sh 的 setproduct 函数添加新平台

# 3. 编译工具集
mbuild_remake wow_tools/appendcrc append_crc

# 4. 编译第三方依赖库 (libev + mbedTLS)
source 3rdparty/autobuild.sh
mbuild_automake

# 5. 编译核心动态库
mbuild_remake wow_base wow_base_src    # → libwowbase.so
mbuild_remake wow_hal  wow_hal_src     # → libwowhal.so
mbuild_remake wow_iot  wow_iot_src     # → libwowiot.so

# 6. 编译单元测试程序
mbuild_remake wow_base wow_base_test   # → wow_base_test
mbuild_remake wow_hal  wow_hal_test    # → wow_hal_test
mbuild_remake wow_iot  wow_iot_test    # → wow_iot_test

# 7. 编译应用程序 (如已实现)
mbuild_remake wow_app  wow_app_src     # → wow_app
```

### 5.2 分步说明

#### Step 1：环境初始化

```bash
. build/envsetup.sh
```

该脚本将注册 `mbuild_setup`、`mbuild_remake`、`mbuild_automake` 等构建命令到当前 Shell 会话。

#### Step 2：选择硬件平台

```bash
mbuild_setup
```

当前支持的平台：

| 平台标识 | 说明 | 编译器 |
|----------|------|--------|
| `X86` | x86_64 本地开发环境 | gcc |
| `ARM_A` | ARM 平台 A | arm-linux-gnueabihf-gcc |
| `ARM_B` | ARM 平台 B | 对应交叉编译器 |

> 💡 **新增平台**：编辑 `build/profile.sh` 中的 `setproduct` 函数，添加新的平台配置项。

#### Step 3：编译第三方库

```bash
source 3rdparty/autobuild.sh
mbuild_automake
```

将从 `3rdparty/` 目录下解包并编译 libev 4.33 和 mbedTLS 2.13.1。

#### Step 4：编译核心库

```bash
mbuild_remake wow_base wow_base_src    # 基础平台库
mbuild_remake wow_hal  wow_hal_src     # 硬件抽象库
mbuild_remake wow_iot  wow_iot_src     # IoT 业务库
```

### 5.3 编译产物

所有编译产物输出至 `output/<平台>/` 目录：

| 产物 | 类型 | 说明 |
|------|------|------|
| `libwowbase.so` | 动态库 | 基础平台层 |
| `libwowhal.so` | 动态库 | 硬件抽象层 |
| `libwowiot.so` | 动态库 | IoT 业务层 |
| `wow_base_test` | 可执行文件 | wow_base 单元测试 |
| `wow_hal_test` | 可执行文件 | wow_hal 单元测试 |
| `wow_iot_test` | 可执行文件 | wow_iot 单元测试 |
| `append_crc` | 可执行文件 | CRC 校验追加工具 |

---

## 六、运行测试

### 6.1 X86 环境测试

```bash
# 设置动态库搜索路径
export LD_LIBRARY_PATH=./output/X86/:$LD_LIBRARY_PATH

# 添加执行权限
chmod +x ./output/X86/*

# 运行基础库测试
./output/X86/wow_base_test

# 运行硬件抽象层测试
./output/X86/wow_hal_test

# 运行 IoT 业务层测试
./output/X86/wow_iot_test
```

### 6.2 ARM 目标板测试

```bash
# 将产物拷贝至目标板（以 scp 为例）
scp -r ./output/ARM_A/* user@target_ip:/opt/wow/

# 在目标板上执行
ssh user@target_ip
cd /opt/wow
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
./wow_base_test
./wow_iot_test
```

### 6.3 测试覆盖

| 模块 | 测试套件数 | 覆盖范围 |
|------|:---:|------|
| wow_base | 20+ | 文件(3)、链表(3)、网络(4)、系统(6)、工具(7) |
| wow_hal | 1 | 串口通信 |
| wow_iot | 22+ | 算法(3)、配置(3)、数据库(2)、加密(2)、哈希(4)、网络(2)、插件(3)、协议(6) |

测试框架采用 [greatest](https://github.com/silentbicycle/greatest)（单头文件 C 语言单元测试框架）。

---

## 七、快速开始

### 7.1 在你的项目中使用

```c
#include "wow_base_init.h"
#include "wow_iot_init.h"
#include "wow_string.h"
#include "wow_json.h"
#include "wow_log.h"
#include "wow_modbus_m.h"

int main(void)
{
    /* 初始化基础库 */
    wow_base_init();
    wow_iot_init();

    /* 使用日志 */
    wow_log_info("WOW-IoT7 started");

    /* 解析 JSON 配置 */
    // wow_json_load("config.json");

    /* 创建 Modbus 主站连接 */
    // wow_modbus_m_create(...);

    return 0;
}
```

### 7.2 新增硬件平台

1. 在 `wow_hal/src/board/` 下新增 BSP 文件（如 `imx6ull.c`）
2. 修改 `wow_hal/src/hal_dev_conf.h` 配置设备节点路径
3. 在 `build/profile.sh` 的 `setproduct` 中添加平台配置
4. 运行 `mbuild_setup` 选择新平台并编译

### 7.3 新增通信协议

1. 在 `wow_iot/inc/protocol/` 下新增协议头文件
2. 在 `wow_iot/src/protocol/` 下实现编解码逻辑
3. 复用 `wow_proto_port` 传输层适配接口
4. 在 `wow_iot/test/protocol/` 下添加测试套件

---

## 八、文档资源

| 文档 | 路径 | 说明 |
|------|------|------|
| 编码规范 | `doc/code-rule.md` | 命名规则、代码风格 |
| 源码树说明 | `doc/source-tree.md` | 目录结构详解 |
| 变更记录 | `doc/change-log.md` | 版本变更历史 |
| API 使用文档 | `doc/wow_blog/` | 各模块 API 说明（25+ 篇） |
| 基础库专题 | `doc/wow_base/` | 线程/锁/信号量等技术专题 |
| 第三方库说明 | `doc/3rdparty/` | libev 使用指南 |
| 单元测试指南 | `doc/greatest单元测试框架.md` | greatest 框架使用方法 |

---

## 九、设计原则

- **分层解耦**： Base →HAL → IoT → App 四层架构，职责单一，层间通过头文件接口通信
- **模块隔离**：每个模块拥有独立的 `inc/`（公开 API）和 `src/`（私有实现），`*_internal.h` 不对外暴露
- **前缀命名**：所有对外符号以 `wow_` 开头，避免命名冲突
- **依赖可控**：第三方库最小化，核心库（SQLite3/cJSON/ezxml）源码内嵌
- **测试驱动**：每个功能模块配备对应的 `suit_*.c` 单元测试

---

## 十、许可证

本项目基于 [MIT License](LICENSE) 开源。

---



