---
title: 1、命名规则
created: '2024-03-08T08:33:24.314Z'
modified: '2024-03-08T09:22:57.610Z'
---

# 1、命名规则
## （1）数据类型
      整数类型使用<stdint.h>内定义格式，约束为：
      int8_t/uint8_t
      int16_t/uint16_t
      int32_t/uint32_t
      int64_t/uint64_t

## （2）数据类型定义
      ///<结构体定义后缀_T,结构体指针_PT;
      typedef struct{
          const FileOps_T *ops;
          size_t size;
      }FileHandle_T,*FileHandle_PT;
      ///<枚举类型为_E;
      typedef enum{
          POLLER_TYPE_NONE         = 0,
          POLLER_TYPE_POLL         = 1,
          POLLER_TYPE_EPOLL        = 2,
          POLLER_TYPE_SELECT       = 3,
        }PollerType_E;
        
      ///<函数指针_f;
      typedef void (*event_call_f)(int sktfd,size_t events,const void* priv);

## （3）宏定义
      ///<约束定义方式1
      #ifndef NULL         
        #define NULL      ((void *)0)
      #endif
      
    ///<约束定义方式2
    宏定义尽可能使用do{……}while(0),防止构造后的宏定义不会受到大括号、分号等的影响
    对于中间处理函数包含break;continue不可以此方式包含
    #define xxx do{……}while(0);

## （4）函数
    平台库外部接口 public:  wow_module_ack；
    平台库内部接口 private: _wow_module_ack; 
    平台库静态接口 static:  module_ack ;

## （5)变量
### 1)全局变量
      static int gn_signal = 0;
      static int gns_signals[5] ={0};
      static Time_T*  gpt_time = NULL;
      static Timer_T  gt_timer = {0};
### 2)函数传递参数
#### 指针类型
    void*       pData;
    void**      ppData;
    DevValue_U* puData;
    Time_T*     ptTime;
    Time_PT     ptTime;
    char*       pcData;
    uint8_t*    pu8Data;
    float*      pfData;
    double*     pdData;
#### 非指针类型
    bool       bRet；
    int        nLen
    size_t     snLen；
    
    int8_t     s8Len;
    uint8_t    u8Len;
    int16_t    s16Len;
    uint16_t   u16Len;
    int32_t    s32Len;
    uint32_t   u32Len;
    int64_t    s64Len;
    uint64_t   u64Len;
    float      fData;
    double     dData;
    
    Status_E    eMode;
    Time_T      tTime;

# 2、编码规范
## （1）编码规范
    [1] 输入参数的有效性在API接口函数实现判断，深层内部函数不在进行判断;
    [2] 在不确定获取内容信息个数时，不在使用数组+个数方式实现，采用链表方式传递;
    [3] 为统一与底层返回值统一，所有函数返回值不允许使用bool类型的true、false;
        返回值只存在成功值与失败值时，定义>=0为成功 <0为失败;
        返回值只存在三种状态时，定义>0为成功 =0临界状态 <0为失败;
    [4] 函数代码块、函数功能尽量小，原则性的行最长小于80字符，最长24行;
    [5] 约束函数中尽可能不是用printf字段，根据需求决定是串口输出还是网口输出;
    [6] switch中应有default;case语句内容使用{}包裹；
    [7] 有返回值的函数的每个分支都应该有返回值，否则其返回结果是未知的;
    [8] 重复的代码片段提取成函数，如果函数比较短考虑定义为 `inline` 函数;
    [9] 在栈上声明的变量必须初始化;
    [10] 文件路径尽可能使用绝对路径，如文件名中包含 ../则会可能造成路径穿越，导致任意文件的读写;
    [11] 比较数据大小时加上最小/最大值的校验;
    [12] 在对指针进行释放后，需要将该指针设置为NULL;防止二次释放
    [13] 可作为子表达式的宏定义应该用括号括起来 #define ABS(x) ((x) < 0? -(x): (x))
          由多个语句组成的宏定义应该用 do-while(0) 括起来  #define SWAP(a, b) do {a ^= b; b ^= a; a ^= b;} while(0)
    [14] 声明头文件之间的依赖或排斥关系，如果 bar.h 依赖 foo.h，在 #include "bar.h" 之前必须 #include "foo.h"，
            可在 bar.h 中设置以下字段，这样如果不满足条件无法通过编译。
            // Header file bar.h
            #ifndef LIBRARY_FOO_H
            #error foo.h should be included first
            #endif
    [15] TODO、FIXME、XXX、BUG 等特殊注释表示代码中存在问题，这种问题不应被遗忘，应有计划地予以解决。
            及时记录问题是一种好习惯，而且最好有署名和日期。
            void foo() {
                    /* TODO:
                    * Some plans...  -- my name, date
                    */
            }
    [16] 头文件中不应使用静态声明
    [17] 由 const 关键字修饰的全局对象已具有静态链接性（internal linkage），不应再用 static 关键字修饰
    [18] 禁止 goto 语句向前跳转
## （2）编码安全
    [1]禁用不安全的字符串函数gets、sprintf、scanf、sscanf、strcpy、strcat
    [2]申请缓存或者数组时长度应为最大输入缓存长度+1;且将最后一个租借设置为结束符\0
    [3]realloc可变长度数组使用的内存量在编译期间不可知，尤其是多次调用时，禁止使用。
        替换方案free后重新使用malloc申请。
    [4]内存数据、内存地址等关键内容数据不应显示在Debug信息中。
    [5]不应该在客户端代码中硬编码（固定密钥）对称加密秘钥，应与服务端行认证秘钥协商，生成对称秘钥。
    [6]函数不可以返回栈上的变量的地址，其内容在函数返回后就会失效。使用堆变量地址。
    [7]当开发中复制粘贴语句，请记得检查每一行代码，不要出现上下两句一模一样的情况
    [8]不得使用明文存储用户密码等敏感信息;
    [9]内存、临时缓存使用关键敏感信息后应及时抹除;
    [10]在计算时需要考虑整数溢出的可能，尤其在进行内存操作时，需要对分配、拷贝等大小进行合法校验;
    [11]在一些涉及大小端数据处理的场景，需要进行大小端判断;
## （3）编码约束
    [1]代码与调试信息展示中不可包含敏感数据。
    [2]避免用明文或弱加密方式传输敏感数据。
    [3]避免敏感数据从内存交换到外存。mlock/munlock
    [4]敏感数据在使用后应被有效清理。
    [5]公共成员或全局对象不应记录敏感数据。
    [6]预判用户输入造成不良后果。
    [8]确保字符串以空字符结尾。
    [9]避免除0等计算异常
## （4）编码风格
      ---缩进：缩进使用制表符，一个制表符等于4个空格。
      ---注释：注释单行不超过80列，特殊情况除外。
      ---布局：库头文件+系统头文件+本地头文件+宏定义+数据结构定义+全局变量+函数定义


