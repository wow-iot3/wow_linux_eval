#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


#include "prefix/wow_check.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_common.h"

#include "utils/wow_singleton.h"

#include "system/wow_lock.h"
#include "system/wow_time.h"
#include "file/wow_hfile.h"
#include "file/wow_file.h"
#include "file/wow_dir.h"

#include "hash/wow_hash_map.h"
#include "plugin/wow_log.h"

#include "wow_iot_config.h"
#include "wow_iot_internal.h"

//日志打印级别
#define LOG_LEVEL_ENV       "LIBLOG_LEVEL"//用于配置环境变量
#define LOG_MAX_FILE_NUM    (5)
#define LOG_MAX_FILE_SIZE   (512*1024)
#define LOG_MAX_FILE_BYTE   10	//单文件覆盖写,前10个字节用于存储当前操作位置

/* from /usr/include/sys/syslog.h */
static const char *gi_logLevel_str[] = {
    "EMERG",
    "ALERT",
    "CRIT",
    "ERR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG",
    "VERBOSE",
    NULL
};

typedef struct{
	char	   dir[MAX_PATH_SIZE];///<文件存储路径
	uint32_t   size;	  ///< 文件容量大小
	uint16_t   num;		  ///< 文件文件个数 单文件循环覆盖写 多文件循环写
	uint16_t   sel;		  ///< 文件操作文件
	uint32_t   pos;      ///< 文件当前长度
	File_T*    hfile;    ///<unix文件操作标识			
}LogFileInfo_T;

typedef struct{
	char mod[MAX_NAME_SIZE];	
	int  level;			///<打印级别
	int  prefix;		///<打印前缀信息标识
	LogFileInfo_T  fileInfo;
}LogInfo_T;

typedef struct{
	int  level;			  ///<打印级别
	int  prefix;		  ///<打印前缀信息标识
	uint32_t   size;	  ///< 文件容量大小
	uint32_t   num;		  ///< 文件文件个数 单文件循环覆盖写 多文件循环写
	char dir[MAX_PATH_SIZE];  ///< 日志文件存放路径
}LogDefaultParam_T;


static mutex_lock_t gt_logMutex;
static HashMap_T*	gpt_hashMap = NULL;

static LogDefaultParam_T   gt_defalut_param ={
	.level  = LOG_LEV_WARNING,
	.prefix = LOG_ATT_TIMESTAMP,
	.size   = (256*1024),
	.num    = 1,
	.dir    = WOW_LOG_SAVE_PATH
};


#define LOG_FILENAME(filename,size,dir,name,sel)   snprintf(filename, size, "%.256s/%.64s-%d.log", dir, name, sel)


static File_T* log_init_one_file(LogInfo_T *logInfo)
{
	int ret = -1;
	File_T* hfile = NULL;
    char filename[MAX_PATH_SIZE];
	char writepos[LOG_MAX_FILE_BYTE];

	memset(filename,0,MAX_PATH_SIZE);
	LOG_FILENAME(filename,MAX_PATH_SIZE, logInfo->fileInfo.dir,logInfo->mod,0);

	hfile = wow_hfile_open(FILE_IO,filename,FILE_CREATE);
	CHECK_RET_VAL_P(hfile,NULL,"open %s failed!\n",filename);

	if(wow_hfile_size(hfile) <= 0){//文件不存在
		//前10个字节用于存储写位置
		logInfo->fileInfo.pos = LOG_MAX_FILE_BYTE;
		snprintf(writepos,LOG_MAX_FILE_BYTE,"%08d\n",logInfo->fileInfo.pos);
		ret = wow_hfile_write(hfile,writepos,LOG_MAX_FILE_BYTE); 
		CHECK_RET_VAL_P(ret >= 0, NULL,"file write failed!\n");
	}else{ //文件存在 指定写位置
		memset(writepos,0,LOG_MAX_FILE_BYTE);
		ret = wow_hfile_read(hfile,writepos,LOG_MAX_FILE_BYTE); 
		CHECK_RET_VAL_P(ret  >= 0,NULL,"file read failed!\n");
		logInfo->fileInfo.pos = atoi(writepos);
	}

	return hfile;
}
	
static File_T* log_init_mutil_file(LogInfo_T *logInfo)
{
	File_T* hfile = NULL;
	int sel = 0;
	int select = 0;
    char filename[MAX_PATH_SIZE];
	FileInfo_T stat;
	size_t fileTime=0;
	size_t fileSize;
	
	for(sel = 0; sel < logInfo->fileInfo.num; sel++){
		memset(filename,0,MAX_PATH_SIZE);
		LOG_FILENAME(filename,MAX_PATH_SIZE, logInfo->fileInfo.dir,logInfo->mod,sel);
		memset(&stat,0,sizeof(FileInfo_T));
		if(wow_file_info(filename,&stat) == 0){
			///<最后一次修改该文件的时间 
			if(fileTime < stat.mtime){
				fileTime  = stat.mtime;
				select	  = sel;
				fileSize  = stat.size;    
			}
		}
	}
	
	//最后一个存储文件已经存储完全
	if(fileSize > logInfo->fileInfo.size){	
		select = (select+1)%(logInfo->fileInfo.num);
		memset(filename,0,sizeof(filename));
		LOG_FILENAME(filename,MAX_PATH_SIZE, logInfo->fileInfo.dir, logInfo->mod,select);
		wow_file_remove(filename);
		fileSize = 0;	
	}

	logInfo->fileInfo.pos = fileSize;
	logInfo->fileInfo.sel = select;
	
	//当前文件已打开 关闭
	wow_hfile_close(logInfo->fileInfo.hfile);	
	memset(filename,0,sizeof(filename));
	LOG_FILENAME(filename,MAX_PATH_SIZE, logInfo->fileInfo.dir, logInfo->mod,select);	
    hfile = wow_hfile_open(FILE_IO,filename,FILE_CREATE);
	CHECK_RET_VAL_P(hfile, NULL,"open %s failed!\n", filename);

	return hfile;
}

static void log_write_file(LogInfo_T *logInfo,char *writeBuf, uint32_t len)
{
	int ret = 0;
	char writepos[LOG_MAX_FILE_BYTE];
	
	if(logInfo->fileInfo.hfile == NULL){
		if(logInfo->fileInfo.num == 1){
			logInfo->fileInfo.hfile  = log_init_one_file(logInfo);
		}else{
			logInfo->fileInfo.hfile  = log_init_mutil_file(logInfo);
		}
	}
	CHECK_RET_VOID_P(logInfo->fileInfo.hfile,"open log file failed!!\n");
	uint32_t *pos = &logInfo->fileInfo.pos;
	File_T* hfile = logInfo->fileInfo.hfile;
	
	//写内容
	ret = wow_hfile_seek(hfile,(*pos),SEEK_SET);
	CHECK_RET_VOID_P(ret == 0,"file seek failed!\n");
	
	ret = wow_hfile_write(hfile,writeBuf,len);
	CHECK_RET_VOID_P(ret == len,"file write failed!\n");
	(*pos) += ret;
	
	//写-write—pos
	if(logInfo->fileInfo.num == 1){	
		//写长度
		ret = wow_hfile_seek(hfile,0,SEEK_SET);
		CHECK_RET_VOID_P(ret >= 0,"file seek failed!\n");
		snprintf(writepos,LOG_MAX_FILE_BYTE,"%08d\n",(*pos)); 
		
		ret = wow_hfile_write(hfile,writepos,LOG_MAX_FILE_BYTE);
		CHECK_RET_VOID_P(ret == LOG_MAX_FILE_BYTE,"file write failed!\n");
	}
	
	if((*pos) >logInfo->fileInfo.size){
		
		if(logInfo->fileInfo.num == 1){
			//单文件指向内容开始
			(*pos) = LOG_MAX_FILE_BYTE;	
		}
		else{
			uint16_t *sel = &logInfo->fileInfo.sel;
			char filename[MAX_PATH_SIZE]={0};
			wow_hfile_close(hfile);
			
			(*sel)=((*sel)+1)%(logInfo->fileInfo.num);
			memset(filename,0,sizeof(filename));
			LOG_FILENAME(filename,MAX_PATH_SIZE,logInfo->fileInfo.dir,logInfo->mod,(*sel));
			wow_file_remove(filename);//移除即将操作文件
			
			hfile = wow_hfile_open(FILE_IO,filename,FILE_CREATE);
			CHECK_RET_VOID_P(hfile, "open %s failed\n", filename);

			logInfo->fileInfo.hfile = hfile;
			(*pos) = 0;
		}
	}
}


#define LOG_IOVEC_MAX       (10)
#define LOG_BUF_SIZE        (1024)
#define LOG_TIME_SIZE       (23)
#define LOG_LEVEL_SIZE      (32)
#define LOG_TEXT_SIZE       (256)
#define LOG_WRITE_SIZE      (2048)

/*
 *time: level: process[pid]: [tid] tag: message
 *             [verbose          ]
 */
static void _log_print(LogInfo_T *logInfo,int lvl,const char *file, int line, const char *msg)
{
	uint32_t len = 0;
    char s_time[LOG_TIME_SIZE];
	char s_lvl[LOG_LEVEL_SIZE];
    char s_file[LOG_TEXT_SIZE];
	char write_buf[LOG_WRITE_SIZE];
	
	memset(write_buf,0,LOG_WRITE_SIZE);

	//时间信息
    if (logInfo->prefix & LOG_ATT_TIMESTAMP) {
		snprintf(s_time, sizeof(s_time)-1,"{%s}", wow_get_time_stamp());
		memcpy(&write_buf[len],s_time,strlen(s_time));
		len += strlen(s_time);
    }

	//日志级别信息
	snprintf(s_lvl, sizeof(s_lvl)-1,"[%s]", gi_logLevel_str[lvl]);
	memcpy(&write_buf[len],s_lvl,strlen(s_lvl));
	len += strlen(s_lvl);

	//文件信息
    if (logInfo->prefix & LOG_ATT_FUNCLINE) {
		snprintf(s_file, sizeof(s_file)-1, "[%s:%d] ", file, line);
		memcpy(&write_buf[len],s_file,strlen(s_file));
		len += strlen(s_file);	
    }
	
	//正文内容
	memcpy(&write_buf[len],msg,strlen(msg));
	len += strlen(msg);	
	
	log_write_file(logInfo,write_buf,len);

}



static LogInfo_T* log_new_module(const char* mod)
{
	int ret = -1;
	LogInfo_T *logInfo = NULL;	

	CHECK_RET_VAL_P(mod && strlen(mod) < MAX_NAME_SIZE, NULL, "param input invalid!\n");
	CHECK_RET_VAL_P(gpt_hashMap, NULL, "log modudle not inited!\n");	

	logInfo = wow_hash_map_get(gpt_hashMap,mod);
	CHECK_RET_VAL(logInfo == NULL, logInfo);	

	logInfo = CALLOC(1, LogInfo_T);
	CHECK_RET_VAL_P_A(logInfo, NULL, "malloc LogInfo_T failed!\n");
	
    logInfo->level  = gt_defalut_param.level;
	logInfo->prefix = gt_defalut_param.prefix;
	logInfo->fileInfo.size   = gt_defalut_param.size;
	logInfo->fileInfo.num    = gt_defalut_param.num;
	logInfo->fileInfo.sel    = 0;
	logInfo->fileInfo.pos    = 0;
	
	memcpy(logInfo->mod,mod,strlen(mod));
	memcpy(logInfo->fileInfo.dir,gt_defalut_param.dir,MAX_PATH_SIZE);
	
	ret = wow_hash_map_insert(gpt_hashMap,mod,logInfo);
	CHECK_RET_GOTO_P(ret == 0,out,"add list failed!\n");

    return logInfo;
out:
	FREE(logInfo);
	return NULL;
}

void log_print(const char* mod,int lvl,const char *file, int line,const char *fmt, ...)
{
    va_list ap;
    char buf[LOG_BUF_SIZE] = {0};
    int n = 0;
	
	CHECK_RET_VOID_P(mod,"param input invalid!\n");
	
	wow_mutex_lock(&gt_logMutex); 

	LogInfo_T* logInfo = log_new_module(mod);
	CHECK_RET_GOTO(logInfo && logInfo->level >= lvl, out);
		
    va_start(ap, fmt);
    n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
	
    CHECK_RET_GOTO(n > 0, out);
	printf("[%s]%s",mod,buf);

    _log_print(logInfo,lvl,file, line,buf);
out:
    wow_mutex_unlock(&gt_logMutex);
}



static int log_init(void)
{
	int ret = 0;

	//检查文件夹是否存在 不存在则创建
	if(wow_file_check_exist(gt_defalut_param.dir) == 0){
		ret = wow_dir_create(gt_defalut_param.dir);
		CHECK_RET_VAL_P(ret == 0, -1,"mkdir %s failed!\n",gt_defalut_param.dir);
	} 
	
	gpt_hashMap = wow_hash_map_create(ELEMENT_TYPE_STR,ELEMENT_TYPE_PTR);
	CHECK_RET_VAL_P(gpt_hashMap, -1,"hash_map_init failed!\n");
	
	wow_mutex_lock_init(&gt_logMutex);
	
	return 0;
}

static void hash_map_clear(const void *data, const void *user_data)
{
	LogInfo_T* logInfo = (LogInfo_T*)data;
	CHECK_RET_VOID(logInfo);
	
	wow_hfile_close(logInfo->fileInfo.hfile);
	FREE(logInfo);
}

static void log_exit(void)
{
	wow_mutex_lock_exit(&gt_logMutex);

	CHECK_RET_VOID(gpt_hashMap);

	wow_hash_map_foreach(gpt_hashMap,hash_map_clear,NULL);
	wow_hash_map_destroy(&gpt_hashMap);

}

__IN_API__ int _wow_log_env_init(void)
{
    return wow_singleton_instance(SINGLETON_IOT_MOD_LOG, log_init, log_exit);
}


/*******************************对外接口***********************************************/

/*brief    设置日志文件级别
 *param ： pcMod:  功能模块名称
 *param ： eLevel: 打印级别			//默认LOG_LEV_WARNING
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_log_set_module_level(const char* pcMod,LogLevel_E eLevel)
{	
	CHECK_RET_VAL_P(pcMod && eLevel >= LOG_LEV_EMERG,-1,"param input invalid!\n");	

	LogInfo_T* logInfo = log_new_module(pcMod);
	CHECK_RET_VAL_P(logInfo,-1,"log_new_module failed!\n");

	logInfo->level = eLevel;
	
	return 0;
}

/*brief    设置日志文件打印特性
 *param ： pcMod:  功能模块名称
 *param ： eAttr: 打印特性   //默认0
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_log_set_module_flag(const char* pcMod,LogAttr_E eAttr)
{
	CHECK_RET_VAL_P(pcMod,-1,"param input invalid!\n");

	LogInfo_T* logInfo = log_new_module(pcMod);
	CHECK_RET_VAL_P(logInfo,-1,"log_new_module failed!\n");

	logInfo->prefix = eAttr;
	
	return 0;
}

/*brief    设置日志文件存储目录
 *param ： pcMod:  功能模块名称
 *param ： pcDir: 文件存储路径 //默认"/home/tmp/log/"
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_log_set_module_path(const char* pcMod,char* pcDir)
{
	CHECK_RET_VAL_P(pcMod && pcDir,-1,"param input invalid!\n");

	LogInfo_T* logInfo = log_new_module(pcMod);
	CHECK_RET_VAL_P(logInfo,-1,"log_new_module failed!\n");

	memset(logInfo->fileInfo.dir,0,MAX_PATH_SIZE);
	memcpy(logInfo->fileInfo.dir,pcDir,strlen(pcDir));

	if(wow_file_check_exist(logInfo->fileInfo.dir) == 0){
		int ret = wow_dir_create(logInfo->fileInfo.dir);
		CHECK_RET_VAL_P(ret == 0, -1,"mkdir %s failed!\n",logInfo->fileInfo.dir);
	} 
		
	return 0;
}

/*brief    设置日志文件信息
 *param ： pcMod:  功能模块名称
 *param ： u32Size:  文件存储大小  //256K
 *param ： u32Num:  文件存储个数  //默认1
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_log_set_module_file(const char* pcMod,uint32_t u32Size, uint32_t u32Num)
{
	CHECK_RET_VAL_P(pcMod && u32Size > 0 && u32Num > 0,-1,"param input invalid!\n");
	
	LogInfo_T* logInfo = log_new_module(pcMod);
	CHECK_RET_VAL_P(logInfo,-1,"param input index invalid!\n");

	u32Num = (u32Num >LOG_MAX_FILE_NUM) ?LOG_MAX_FILE_NUM:u32Num;
	u32Size= (u32Size>LOG_MAX_FILE_SIZE) ?LOG_MAX_FILE_SIZE:u32Size;

	logInfo->fileInfo.num  = u32Num;
	logInfo->fileInfo.size = u32Size;
	
	return 0;
}


/*brief    设置日志文件默认目录
 *param ： pcDir: 文件存储路径 "/home/tmp/log/"
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_log_set_default_path(const char* pcDir)
{
	CHECK_RET_VAL_P(pcDir,-1,"param input invalid!\n");

	//检查文件夹是否存在 不存在则创建
	if(wow_file_check_exist(pcDir) == 0){
		int ret = wow_dir_create(pcDir);
		CHECK_RET_VAL_P(ret == 0, -1,"mkdir %s failed!\n",pcDir);
	} 
	
	memset(gt_defalut_param.dir,0,MAX_PATH_SIZE);
	memcpy(gt_defalut_param.dir,pcDir,strlen(pcDir));

	return 0;
}

/*brief    设置日志文件信息
 *param ： u32Size: 文件存储大小  //256K
 *param ： u32Num:  文件存储个数  //默认1
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_log_set_default_file(uint32_t u32Size, uint32_t u32Num)
{
	CHECK_RET_VAL_P(u32Size > 0 && u32Num > 0,-1,"param input invalid!\n");

	u32Num = (u32Num >LOG_MAX_FILE_NUM) ?LOG_MAX_FILE_NUM:u32Num;
	u32Size= (u32Size>LOG_MAX_FILE_SIZE) ?LOG_MAX_FILE_SIZE:u32Size;

	gt_defalut_param.num  = u32Num;
	gt_defalut_param.size = u32Size;

	return 0;
}

/*brief    设置日志文件级别
 *param ： eLevel: 打印级别			//默认LOG_LEV_WARNING
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_log_set_default_level(LogLevel_E eLevel)
{
	CHECK_RET_VAL_P(eLevel >= LOG_LEV_EMERG,-1,"param input invalid!\n");
	gt_defalut_param.level = eLevel;

	return 0;
}

/*brief    设置日志文件目录打印特性
 *param ： eAttr: 打印特性   //默认0
 *return： 成功返回0 失败返回-1
 *
 */
__EX_API__ int wow_log_set_default_flag(LogAttr_E eAttr)
{
	gt_defalut_param.prefix = eAttr;

	return 0;
}

