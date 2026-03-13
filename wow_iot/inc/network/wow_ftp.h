#ifndef __WOW_IOT_NETWORK_WOW_FTP_H_
#define __WOW_IOT_NETWORK_WOW_FTP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "list/wow_string_list.h"

typedef struct{
	char     host[128]; 	//FTP服务器IP地址
	uint16_t port;
	char     user[64];
	char     pwd[64];
}FtpInfo_T;

typedef struct ftp_t Ftp_T;


/*brief    登录FTP服务器
 *param ： ptFtpInfo:  ftp连接参数 
 *return： 成功返回ftp操作符 失败返回NULL
 */ 
Ftp_T* wow_ftp_login(FtpInfo_T* ptFtpInfo);

/*brief    删除FTP服务器中文件
 *param ： ptFtp:	   ftp操作符
 *param ： pcSrcFile:  文件路径
 *return： 成功返回0 失败返回< 0
 */
int wow_ftp_file_delete(Ftp_T* ptFtp,const char *pcSrcFile);

/*brief    获取FTP服务器中文件大小
 *param ： ptFtp:	   ftp操作符
 *param ： pcSrcFile:  文件路径
 *return： 成功返回0 失败返回< 0
 */
int wow_ftp_file_size(Ftp_T* ptFtp,const char *pcSrcFile);

/*brief    在FTP服务器中创建文件夹
 *param ： ptFtp:	   ftp操作符
 *param ： pcDirPath:  要创建的文件夹路径
 *return： 成功返回0 失败返回< 0
 */
int wow_ftp_mkdir(Ftp_T* ptFtp,const char *pcDirPath);

/*brief    下载FTP服务器文件
 *param ： ptFtp:	   ftp操作符
 *param ： pcSrcFile:  源文件在FTP服务器中的路径
 *param ： pcDestFile: 下载文件的存储路径(默认路径为/work/xxx)
 *return： 成功返回0 失败返回-1
 */
int wow_ftp_file_download(Ftp_T* ptFtp,const char *pcSrcFile,const char *pcDestFile);

 /*brief   上传FTP服务器文件
  *param ： ptFtp:	   ftp操作符
  *param ： pcSrcFile:  上传源文件的本地路径路径
  *param ： pcDestFile: FTP服务器上传路径
  *return： 成功返回0 失败返回<0
 */
int wow_ftp_file_upload(Ftp_T* ptFtp,const char *pcSrcFile,const char *pcDestFile);

 /*brief   退出FTP服务器
  *param ： ptFtp:	   ftp操作符
  *return： 无
 */
void wow_ftp_exit(Ftp_T** pptFtp);


#ifdef __cplusplus
}
#endif

#endif