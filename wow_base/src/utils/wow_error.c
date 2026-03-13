#include <stdio.h>
#include <pthread.h>

static pthread_key_t werrno;
 
// 设置错误码
void wow_set_errno(ssize_t nError) 
{
    pthread_setspecific(werrno, (void*)(ssize_t)nError);
}

// 获取错误码
ssize_t wow_get_errno() 
{
    return (ssize_t)pthread_getspecific(werrno);
}

// 初始化线程局部存储的键
void _wow_error_init(void)
{
    pthread_key_create(&werrno, NULL);
}

// 清除线程局部存储的键
void _wow_error_exit(void)
{
    pthread_key_delete(werrno);

}
 