# wow_base模块说明

wow_base模块主要用于系统关联与基础接口封装库的实现，以供其它库文件与可执行文件调用，主要涉及prefix基础模块、utils通用模块、ipc进程通信模块、file文件模块、list链表模块、network网络模块、system基础模块等。

## main基础模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_base_init | libwowbase库调用初始化操作. | Enabled | No |

## ipc进程通信模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_ipc | 实现进程交互(服务端与客户端)相关接口，包含创建、销毁、发送、接收、通知、广播等功能. | Enabled | No |

## network网络模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_net_iface| 实现网络接口信息的获取，包括获取本机接口的IP、MASK、MAC等. | Enabled | No |
| wow_socket | 打开、关闭、连接、监听、接收、发送消息、接收消息、获取/设置参数、获取本地/对象信息. | Enabled | No |
| wow_tcp | 关联TCP相关接口，包括打开、关闭、连接、监听、接收、发送消息、接收消息. | Enabled | No |
| wow_udp | 关联UDP相关接口，包括打开、关闭、连接、监听、接收、发送消息、接收消息. | Enabled | No |


## prefix基础模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_byte | 处理各种数据类型大小端格式转换问题. | Enabled | No |
| wow_check | 消除在语句执行过程中的判断语句的跳转，让代码看着更更简洁. | Enabled | No |
| wow_check | 常用的简单指令宏定义，包含对齐、内存申请、调试等. | Enabled | No |
| wow_cpu | 用于通过CPU架构来得到大小端模式. | Enabled | No |
| wow_errno | 用于定义整个工程模组错误码. | Enabled | No |
| wow_keyword | 常用的简单宏定义. | Enabled | No |

## utils基础模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_byte_array | 实现类似于QByteArray类，用于处理数组类相关操作，主要涉及数组类的创建、删除、增添、移除、截断、查找、替换等操作. | Enabled | No |
| wow_error | 实现errno相关设置与读取. | Enabled | No |
| wow_hex | 用于处理16进制相关操作，包含打印与数组转换. | Enabled | No |
| wow_math |  关联随机数据的生成. | Enabled | No |
| wow_mem_leak |  查找程序使用过程中是否存在内存泄漏的问题. | Enabled | No |
| wow_type | 处理数据类型的转换，包含字符串、数组(可指定大小端转换)、BCD码. | Enabled | No |
| wow_singleton | 内部使用atomic原子操作，支持接口调用与退出只被执行1次. | Enabled | No |
| wow_str | 用于处理字符串操作，主要涉及字符串替换、字符串合并、字符串分离、字符串查找. | Enabled | No |
| wow_string | 类似于QString类，用于处理字符串操作，主要涉及字符串类创建、销毁、截取左边/右边/中间字段、追加字段、格式转换、获取内容、比较等功能. | Enabled | No |
| wow_ring_buff |  实现数据环形缓冲区相关功能，包括创建、删除、读、写、清除、查看已用空间、查看可用空间. | Enabled | No |


## file文件模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_dir | 用于处理文件夹操作，主要涉否为文件夹、创建文件夹、删除文件夹. | Enabled | No |
| wow_file |  主要用于处理文件操作，主要涉及创建文件、删除文件、重命名、文件比较、获取文件大小、获取文件行数、获取文件信息操作. | Enabled | No |
| wow_hfile |  用于处理文件操作(支持io与fio)，主要涉及文件打开、关闭、读、写、跳转等操作. | Enabled | No |

## list链表模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow_slist | 用于链表相关操作，主要涉及创建、销毁、插入、查找、移除、替换、获取、清空、遍历. | Enabled | No |
| wow_string_list | 用于存储字符串链表，主要涉及创建、销毁、插入、分隔、格式转换. | Enabled | No |
| wow_kv_list | 用于实现键值对的的创建、销毁、设置、获取、更新、打印等功能. | Enabled | No |

## system系统模块
| Feature  | Description   | Default  | Can be disabled |
| -------- | ------------- | -------- | --- |
| wow-atomic |  实现原子增减、设置、获取、比对. | Enabled | No |
| wow-cound | 实现条件变量相关接口，以实现共享数据的同步和互斥. | Enabled | No |
| wow-countdow | 实现定时器操作，主要涉及申请、释放、复位与置数、查询剩余时间、查询是否超时. | Enabled | No |
| wow-lock | 实现原子锁、线程锁、信号量锁的相关操作. | Enabled | No |
| wow-thread | 实现线程的创建、关闭、绑定等工作. | Enabled | No |
| wow-time | 实现时间相关操作，获取时间戳、毫秒、时间设置、格式转换. | Enabled | No |
| wow-timer | 实现定时器相关操作，包括定时器的创建/销毁，添加/移除、启动/停止操作. | Enabled | No |

