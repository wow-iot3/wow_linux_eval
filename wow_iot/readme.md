# wow_iot模块说明

wow_iot模块主要用于系统关联与基础接口封装库的实现，以供其它库文件与可执行文件调用，主要涉及algorith算法模块、config配置模块、database数据库模块、datastruct数据结构模块、plugin插件模块、encode编码模块、encrypt加密模块、hash哈希模块、protocol协议模块。

## main基础模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_iot_init | libwowiot库调用初始化操作. | Enabled | No |

## algorith算法模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_bloomfilter | 实现布隆滤波器功能. | Enabled | No |
| wow_bitmap | 实现位图相关操作，包括申请、释放、设置、获取等. | Enabled | No |

 database数据库模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_database | 实现数据库相关接口，向下兼容多种数据库. | Enabled | No |
| wow_database_val | 实现数据库值获取与格式转换. | Enabled | No |

## config配置模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_xml | 实现xml文件的创建、销毁、存储、查找、添加等功能. | Enabled | No |
| wow_json | 实现json文件的创建、销毁、存储、查找、添加等功能;支持object与array. | Enabled | No |
| wow_ini | 实现ini文件的存储、查找、添加等功能. | Enabled | No |

## datastruct数据结构模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_itoritor | 实现迭代器接口功能. | Enabled | No |

## element元素模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_element | 实现处理各类元素操作. | Enabled | No |

## hash哈希模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_hash | 实现对整形与数组的hash计算. | Enabled | No |
| wow_hash_map | 实现对hash_map的相关接口 包括创建、插入、移除、销毁等.  | Enabled | No |
| wow_hash_set | 实现对hash_set的相关接口 包括创建、插入、移除、销毁等.  | Enabled | No |

## encrypt加密模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_crc | 实现对crc-8、crc-16、crc-32各种模式的计算. | Enabled | No |
| wow_rsa | 实现RSA秘钥的获取，数据的加迷/解密，文件的签名/验证. | Enabled | No |

## network插件模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_ftp | 实现ftp客户端，支持文件下载、上传等操作. | Enabled | No |
| wow_ntpdate | 实现ntpdate时间同步功能. | Enabled | No |

## plugin插件模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_debug| 实现按模块信息打印，并支持打印级别设置与shell控制台调试信息开启/关闭. | Enabled | No |
| wow_shell| 实现串口控制台与网口控制台对程序运行操作进行相关控制. | Enabled | No |
| wow_log  | 实现日志的记录功能 同时可关联wow_debug/wow_check接口. | Enabled | No |


## protocol插件模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_proto_data| 实现协议数据处理相关接口封装. | Enabled | No |
| wow_proto_port| 实现协议硬件接口的相关接口封装. | Enabled | No |
| wow_modbus_m| 实现modbus-rtu与modbus-tcp主机相关接口. | Enabled | No |
| wow_modbus_s| 实现modbus-rtu与modbus-tcp从机相关接口. | Enabled | No |
| wow_iec04m| 实现IEC04协议的总招，遥测，遥控等功能. | Enabled | No |
| wow_iec04s| 实现IEC04协议的总招，遥测，遥控,突变上报等功能. | Enabled | No |
| wow_iec01m| 实现IEC01协议的总招，遥测，遥控等功能. | Enabled | No |
| wow_iec01s| 实现IEC01协议的总招，遥测，遥控,突变上报等功能. | Enabled | No |


## shmcache插件模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_shmcache| 实现共享内存的创建、销毁、设置、获取、打印等功能. | Enabled | No |
