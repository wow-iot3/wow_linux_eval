#ifndef __WOW_BASE_UTILS_WOW_MEM_LEAK_H_
#define __WOW_BASE_UTILS_WOW_MEM_LEAK_H_



/*brief   malloc内部调试接口
 *param : snSize:分配缓存大小
 *param : snLine:行信息
 *param : pcFile:文件信息
 *param : pcFunc:文件信息
 *return: 成功返回申请缓存 失败返回NULL
 */
void* wow_mleak_malloc(size_t snSize, size_t snLine, const char* pcFile, const char* pcFunc);

/*brief   calloc内部调试接口
 *param : snCount:分项个数
 *param : snSize:分项大小
 *param : snLine:行信息
 *param : pcFile:文件信息
 *param : pcFunc:文件信息
 *return: 成功返回申请缓存 失败返回NULL
 */
void* wow_mleak_calloc(size_t snCount, size_t snSize, size_t snLine, const char* pcFile, const char* pcFunc);

/*brief   ralloc内部调试接口
 *param : pBuff:缓存
 *param : snSize:缓存大小
 *param : snLine:行信息
 *param : pcFile:文件信息
 *param : pcFunc:文件信息
 *return: 成功返回申请缓存 失败返回NULL
 */
void* wow_mleak_realloc(void* pBuff, size_t snSize, size_t snLine, const char* pcFile, const char* pcFunc);

/*brief   free内部调试接口
 *param : pBuff:缓存
 *return: 无
 */
void wow_mleak_free(void* pBuff);


/*brief   打印内存泄漏信息
 *return: 无
 */
void wow_mleak_print(void);


#endif 
