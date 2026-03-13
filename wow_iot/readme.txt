
(1)现框架将调试系统拆分为log与debug模块；
log模块对信息内容进行文件存储，主要用于记录程序运行的主要信息与错误；
debug模块为shell可控单元，可开启与关闭调试信息；主要进行调试使用；

现功能不支持将显示的所有信息进行log存储的功能(即显示存储LOG的功能)；
如需实现须统一log与debug接口，统一调用log接口 通过中间层FIFO实现；


(2)modbus现有框架 约束MODBUS-RTU使用串口物理连接  MODBUS-TCP使用网口物理连接



未定义相关接口时编译错误
/usr/bin/ld: /home/wow/open/wow-iot7/build/local/UBUNTU/lib/libwowiot.so: undefined reference to `__start__shell_ex_cmd'
/usr/bin/ld: /home/wow/open/wow-iot7/build/local/UBUNTU/lib/libwowiot.so: undefined reference to `__stop__shell_ex_cmd'



(3) shmcach需要验证回收机制(时间超时部分修改)
