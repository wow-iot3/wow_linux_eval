# wow_hal模块说明

wow_hal模块主要用于硬件层封装,以实现对不同的硬件平台兼容。主要包括了蓝牙模块、4G/5G模块、WiFi模块、RS232/RS485模块、GPIO模块、DI/DO模块、LED模块、FLASH模块、EEPROM模块、SD卡模块、U盘模块、USB摄像头模块。


## main基础模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_hal_init | libwowhal库调用初始化操作. | Enabled | No |

## 硬件平台配置模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| ubuntu-2004 | ubuntu20平台硬件参数配置. | Enabled | No |

## USART基础模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_usart | 串口接口相关. 涉及串口的打开、关闭、发送、接收、配置 | Enabled | No |
