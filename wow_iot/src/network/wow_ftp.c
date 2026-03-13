#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "file/wow_hfile.h"
#include "file/wow_dir.h"
#include "file/wow_file.h"

#include "network/wow_socket.h"
#include "network/wow_ftp.h"

#define FTP_DATACONN            150
#define FTP_NOOPOK              200
#define FTP_TYPEOK              200
#define FTP_PORTOK              200
#define FTP_STRUOK              200
#define FTP_MODEOK              200
#define FTP_ALLOOK              202
#define FTP_STATOK              211
#define FTP_STATFILE_OK         213
#define FTP_HELP                214
#define FTP_SYSTOK              215
#define FTP_GREET               220
#define FTP_GOODBYE             221
#define FTP_TRANSFEROK          226
#define FTP_PASVOK              227
/*#define FTP_EPRTOK              228*/
#define FTP_EPSVOK              229
#define FTP_LOGINOK             230
#define FTP_CWDOK               250
#define FTP_RMDIROK             250
#define FTP_DELEOK              250
#define FTP_RENAMEOK            250
#define FTP_PWDOK               257
#define FTP_MKDIROK             257
#define FTP_GIVEPWORD           331
#define FTP_RESTOK              350
#define FTP_RNFROK              350
#define FTP_TIMEOUT             421
#define FTP_BADSENDCONN         425
#define FTP_BADSENDNET          426
#define FTP_BADSENDFILE         451
#define FTP_BADCMD              500
#define FTP_COMMANDNOTIMPL      502
#define FTP_NEEDUSER            503
#define FTP_NEEDRNFR            503
#define FTP_BADSTRU             504
#define FTP_BADMODE             504
#define FTP_LOGINERR            530
#define FTP_FILEFAIL            550
#define FTP_NOPERM              550
#define FTP_UPLOADFAIL          553

#define MAX_RETRY_CNT   300
#define MAX_RECV_BUF_ONE_FRAME (16*1024)
#define DEST_DEFAULT_PATH "/work/"
#define FTP_CONNECT_TIMEOUT_MS (5000)

static int ftp_exe_cmdSock(int cmdSock,char* recv,int len,char *fmt,...)  
{   
    va_list vVaStartUse;   
    int ftpLength;    
	char ftpbuf[1024];
	int ftpCmdRet = -1;
	int cnt = 0;

    if (fmt) {   
        va_start(vVaStartUse,fmt);   
        ftpLength = vsprintf(ftpbuf,fmt,vVaStartUse);   
		va_end(vVaStartUse);
        ftpbuf[ftpLength++] = '\r';   
        ftpbuf[ftpLength++]='\n';   
        wow_socket_tsend(cmdSock,(const uint8_t*)ftpbuf,ftpLength);  //sendto
    }    
	
	memset(ftpbuf,0,sizeof(ftpbuf));
    do {   
        ftpCmdRet = wow_socket_trecv(cmdSock, (uint8_t*)ftpbuf, sizeof(ftpbuf));
        if (ftpCmdRet == -1 ){
            usleep(100*1000);
            if(++cnt > MAX_RETRY_CNT) break;//reach max retry count
        }else{
			break;
		}

    } while (!isdigit(ftpbuf[0]) || ftpbuf[3] != ' ');    
 
	if(recv != NULL){
		int datalen = (len > sizeof(ftpbuf))?sizeof(ftpbuf):len; 
		memcpy(recv,ftpbuf,datalen);
	}
    sscanf(ftpbuf,"%d",&ftpCmdRet);    

    return ftpCmdRet;  
}   

int xatou_range(char* buf,int low,int top)
{	
	int i = 0;
	int j = 0;
	int len = strlen(buf);
	int retval = 0;
	char mod[3] = {1,10,100};
	if((!len) || (len > 3)) return -1;
	for(i = len-1; i >= 0; i--){
		retval += (buf[i] - '0')*mod[j++];
	}
	retval = (retval < low)?low:retval;
	retval = (retval > top)?top:retval;
	return retval;
}

int parsePasvEpsv(char *buf)
{
	char *ptr;
	int port;
	/* Response is "227 garbageN1,N2,N3,N4,P1,P2[)garbage]"
	 * Server's IP is N1.N2.N3.N4 (we ignore it)
	 * Server's port for data connection is P1*256+P2 */
	//printf("_wowParsePasvEpsv buf:%s",buf);
	ptr = strrchr(buf, ')');
	if (ptr) *ptr = '\0';

	ptr = strrchr(buf, ',');
	if (!ptr) return -1;
	*ptr = '\0';
	port = xatou_range(ptr + 1, 0, 255);

	ptr = strrchr(buf, ',');
	if (!ptr) return -1;
	*ptr = '\0';
	port += xatou_range(ptr + 1, 0, 255) * 256;
	return port;
}


/*brief    连接FTP服务器
 *param ： host: FTP服务器IP地址
 *return： 成功返回socket操作符 失败-1
 */
static int ftp_connect_server(const char *host,uint16_t port)
{
	int ret  = -1;
	int ftpCmdSock = -1;

	//建立socket（命令通道）   设定相应的socket协议和地址  
	ftpCmdSock = wow_socket_open(SOCKET_TYPE_TCP); 
	CHECK_RET_VAL_P(ftpCmdSock != -1,-1, "open socket failed!\n");		

	ret = wow_socket_connect(ftpCmdSock,host,port,FTP_CONNECT_TIMEOUT_MS); 
	CHECK_RET_GOTO_P(ret == 0,out,"wow_socket_connect is failed!\n")

	ret = wow_socket_setopt(ftpCmdSock,SOCKET_CTRL_SET_BLOCK,0);
	CHECK_RET_GOTO_P(ret == 0, out,"wow_socket_block is failed!\n");

	return ftpCmdSock;
out:
	wow_socket_close(ftpCmdSock); 
	return -1;
}

/*brief    查找是该以太网名称是否存在否存在
 *param ： cmdSock: 命令通道socket操作符
 *param ： user:	用户名
 *param ： pwd:		密码  
 *return： 成功返回0 失败返回-1
 */
static int ftp_login_server(int cmdSock,const char *user,const char *pwd)
{
	int ret = -1;
	
	ret = ftp_exe_cmdSock(cmdSock,NULL,0,NULL);
	CHECK_RET_VAL_P(ret == FTP_GREET,-1,"ftp_exe_cmdSock failed!\n");
	
	/*  Login to the server */
	ret = ftp_exe_cmdSock(cmdSock,NULL,0,"USER %s", user);
	switch (ret) {
		case FTP_LOGINOK:
			break;
		case FTP_GIVEPWORD:
			ret = ftp_exe_cmdSock(cmdSock,NULL,0,"PASS %s", pwd);
			CHECK_RET_VAL_P(ret == FTP_LOGINOK,-1,"ftp_exe_cmdSock PASS failed!\n");
			break;
		default:
			printf("ftp_exe_cmdSock USER failed!!\n");
			return -1;
	}

	ret = ftp_exe_cmdSock(cmdSock,NULL,0,"TYPE I", NULL);
	CHECK_RET_VAL_P(ret == FTP_TYPEOK,-1,"ftp_exe_cmdSock TYPE failed!\n");

	return 0;
}

/*brief    获取FTP文件信息
 *param ： cmdSock:   命令通道socket操作符
 *param ： pcSrcFile：源文件路径
 *return： 成功返回文件大小，失败返回-1
 */
static int ftp_get_fileSize(int cmdSock,const char *srcFile)
{
	char recvbuf[128] = {0};
	int ftpFileSize = 0;

	CHECK_RET_VAL_P(srcFile,-1,"srcFile is NULL!\n");

	int ret = ftp_exe_cmdSock(cmdSock,recvbuf,sizeof(recvbuf),"SIZE %s", srcFile);
	CHECK_RET_VAL(ret == FTP_STATFILE_OK, -1);
	
	sscanf(&recvbuf[4],"%d",&ftpFileSize);   
	
    return ftpFileSize;  
}

static int ftp_mk_dir(int cmdSock,const char * dir)
{
	int ret = 0;
	
	CHECK_RET_VAL_P(dir,-1,"dir is NULL!\n");

	ret = ftp_exe_cmdSock(cmdSock,NULL,0,"CWD %s", dir);
	CHECK_RET_VAL(ret != FTP_CWDOK,0);
	
	ret = ftp_exe_cmdSock(cmdSock,NULL,0,"MKD %s", dir);
	CHECK_RET_VAL_P(ret == FTP_MKDIROK, -1,"ftp_exe_cmdSock CWD failed!\n");
	
    return 0; 
}

static int ftp_delete_file(int cmdSock,const char * srcFile)
{
	int ret = -1;   	 	
	char cmd[256] = {0};

	snprintf(cmd,255,"DELE %s\r\n",srcFile);
	ret = ftp_exe_cmdSock(cmdSock,NULL,0,cmd);
	CHECK_RET_VAL_P(ret == FTP_CWDOK,-1,"ftp_exe_cmdSock DELE failed!\n");

	return 0;
}

/*brief    创建FTP数据通道 
 *param ： cmdSock:   命令通道socket操作符
 *return： 成功返回socket操作符 失败-1
 */
static int ftp_create_dataChan(int cmdSock,const char* host)
{
	int ret = -1;
	int port = 0;
	char recvbuf[128] = {0};
	int ftpDataSock = -1; 
	
	ret = ftp_exe_cmdSock(cmdSock,recvbuf,sizeof(recvbuf),"PASV");
	CHECK_RET_VAL_P(ret == FTP_PASVOK,-1,"ftp_open_cmdSock PASV failed!\n");

	port = parsePasvEpsv(recvbuf);
	CHECK_RET_VAL_P(port != -1,-1,"port_num PASV is invalid!\n");


	ftpDataSock = wow_socket_open(SOCKET_TYPE_TCP); 
	CHECK_RET_VAL_P(ftpDataSock != -1,-1, "socket socket failed!\n");	

	ret = wow_socket_connect(ftpDataSock,host,port,FTP_CONNECT_TIMEOUT_MS); 
	CHECK_RET_GOTO_P(ret == 0,out,"wow_socket_connect failed!\n");

	
	wow_socket_setopt(ftpDataSock,SOCKET_CTRL_SET_BLOCK,1);
	
	return ftpDataSock;
out:
	wow_socket_close(ftpDataSock);
	return -1;
}

/*brief    下载FTP服务器中文件
 *param ： cmdSock:  命令通道socket操作符
 *param ： host:     FTP服务器IP地址
 *param ： srcFile:  源文件在FTP服务器中的路径
 *param ： destFile: 下载文件的存储路径
 *return： 成功返回0 失败返回-1
 */
static int ftp_download_file(int cmdSock,const char *host,const char *srcFile,const char *destFile)
{
	int ret = -1;
	int count = 0;
	int srcFileSize = 0; 
	int destFileSize = 0;   
	int dataSock  = 0;
	char name[64] = {0};
	char dest[128] = {0};
	File_T* hfile = NULL; 
	int nread  = 0; 
	char  ftpBuf[MAX_RECV_BUF_ONE_FRAME] = {0};
	char* ptr = NULL;
	
	CHECK_RET_VAL_P(host && srcFile,-1,"param inout is invalid!\n");

	dataSock = ftp_create_dataChan(cmdSock,host);
	CHECK_RET_VAL_P(dataSock > 0,-1,"ftp_create_dataChan is failed!\n");
	
	srcFileSize= ftp_get_fileSize(cmdSock,srcFile);
	CHECK_RET_GOTO_P(srcFileSize > 0,out1,"ftp_get_fileSize failed!\n");

	ret = ftp_exe_cmdSock(cmdSock,NULL,0,"RETR %s",srcFile);
	CHECK_RET_GOTO_P(ret <= FTP_DATACONN,out1,"ftp_exe_cmdSock RETR failed!\n");
	
	if(destFile == NULL){//目的地址未设置 保存于默认路径下
		char* p = strrchr(srcFile,'/');
		if(p == NULL){
			memcpy(name,srcFile,sizeof(name));
		}else{
			memcpy(name,p+1,sizeof(name));
		}
		wow_dir_create(DEST_DEFAULT_PATH);
		snprintf(dest,127,DEST_DEFAULT_PATH"%s",name);
		hfile = wow_hfile_open(FILE_FIO,dest,FILE_RDONLY | FILE_CREATE);
	}else{
		hfile = wow_hfile_open(FILE_FIO,destFile,FILE_RDONLY | FILE_CREATE); 
	}
	CHECK_RET_GOTO_P(hfile,out1,"open file failed!\n");

	do {
		nread = wow_socket_trecv(dataSock, (uint8_t*)ftpBuf, sizeof(ftpBuf));
		if((nread == -1) && (errno == EINTR || errno == EAGAIN)){
			if(count++ < 60) {
				sleep(2);
				continue;
			} else{
				break;
			}  
		}
		
		CHECK_RET_BREAK(nread > 0);
		ptr = ftpBuf;
		do {
			/* write data in a loop until we block */
			ret = wow_hfile_write(hfile, ptr, nread);
			CHECK_RET_BREAK_P(ret >= 0,"wow_hfile_write failed!\n");
			ptr += ret;
			nread -= ret;
		} while(nread);

	} while(ret > 0);

	wow_hfile_close(hfile); 
	wow_socket_close(dataSock);

	ret = wow_file_size(destFile,&destFileSize);
	CHECK_RET_VAL_P(ret == 0 && srcFileSize == destFileSize,-1,"download file size not match src file.");

	return 0;
out1:
	wow_socket_close(dataSock); 
	return -1;
}

/*brief    上传FTP服务器文件
 *param ： cmdSock:  命令通道socket操作符
 *param ： host:     FTP服务器IP地址
 *param ： srcFile:  源文件在FTP服务器中的路径
 *param ： destFile: 下载文件的存储路径
 *return： 成功返回0 失败返回-1
 */
static int ftp_upload_file(int cmdSock,const char *host,const char *srcFile,const char *destFile)
{
	int ret = -1;
	File_T* hfile = NULL; 
	int nread  = 0;  
	int dataSock = -1;
	int filesize = 0;
	char* ptr = NULL;
	char recvbuf[128] = {0};
	char ftpBuf[MAX_RECV_BUF_ONE_FRAME] = {0};

	CHECK_RET_VAL_P(host && srcFile ,-1,"param inout is invalid!\n");

	dataSock = ftp_create_dataChan(cmdSock,host);
	CHECK_RET_VAL_P(dataSock > 0,-1,"ftp_create_dataChan is failed!\n");

	ret = ftp_exe_cmdSock(cmdSock,NULL,0,"STOR %s",destFile);
	CHECK_RET_GOTO_P(ret == FTP_DATACONN,out1,"ftp_exe_cmdSock STOR failed!\n");

	ret = wow_file_size(srcFile,&filesize);
	CHECK_RET_GOTO_P(ret == 0 && filesize > 0,out1,"srcfile file size is 0!\n");

    hfile = wow_hfile_open(FILE_FIO,srcFile,FILE_RDONLY);   
	CHECK_RET_GOTO_P(hfile,out2,"open srcfile failed!\n");
	
	do {
		nread = wow_hfile_read(hfile,ftpBuf,sizeof(ftpBuf));
		CHECK_RET_BREAK(nread > 0);
		ptr = ftpBuf;

		do {
			/* write data in a loop until we block */
			ret = wow_socket_tsend(dataSock, (uint8_t*)ptr, nread);
			CHECK_RET_BREAK_P(ret >= 0,"wow_socket_send failed!\n");
			ptr += ret;
			nread -= ret;
		} while(nread);

	} while(ret > 0);
		
	wow_hfile_close(hfile); 
	wow_socket_close(dataSock);
	
	ret = ftp_exe_cmdSock(cmdSock,recvbuf,sizeof(recvbuf),"PASV");
	CHECK_RET_VAL_P(ret == FTP_TRANSFEROK,-1,"ftp_exe_cmdSock PASV failed!\n");

	return 0;

out2:
	wow_hfile_close(hfile);
out1:
	wow_socket_close(dataSock);
	return -1;
}


struct ftp_t{
	int cmd_skt;	
	FtpInfo_T param;
}__data_aligned__;


/*brief    登录FTP服务器
 *param ： ptFtpInfo:  ftp连接参数 
 *return： 成功返回ftp操作符 失败返回NULL
 */ 
__EX_API__ Ftp_T* wow_ftp_login(FtpInfo_T* ptFtpInfo)
{
	int ret  = -1;
	int socket = -1;

	CHECK_RET_VAL_P(ptFtpInfo,NULL,"param input struct invalid.\n");

	socket = ftp_connect_server(ptFtpInfo->host,ptFtpInfo->port);
	CHECK_RET_VAL_P(socket > 0,NULL,"connect ftp server failed.\n");

	ret = ftp_login_server(socket,ptFtpInfo->user,ptFtpInfo->pwd);
	CHECK_RET_GOTO_P(ret == 0, out,"wow_socket_block failed!\n");

	Ftp_T* ftp = CALLOC(1,Ftp_T);
	CHECK_RET_GOTO_P(ftp, out,"malloc Ftp_T failed!\n");

	ftp->cmd_skt = socket;
	memcpy(&ftp->param,ptFtpInfo,sizeof(FtpInfo_T));
	return ftp;
out:
	wow_socket_close(socket); 
	return NULL;
}

/*brief    删除FTP服务器中文件
 *param ： ptFtp:	   ftp操作符
 *param ： pcSrcFile:  文件路径
 *return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_ftp_file_delete(Ftp_T* ptFtp,const char *pcSrcFile)
{
	CHECK_RET_VAL_P(ptFtp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid.\n");
	CHECK_RET_VAL_P(pcSrcFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid.\n");
	
	return ftp_delete_file(ptFtp->cmd_skt,pcSrcFile);
}


/*brief    获取FTP服务器中文件大小
 *param ： ptFtp:	   ftp操作符
 *param ： pcSrcFile:  文件路径
 *return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_ftp_file_size(Ftp_T* ptFtp,const char *pcSrcFile)
{
	CHECK_RET_VAL_P(ptFtp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid.\n");
	CHECK_RET_VAL_P(pcSrcFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid.\n");

	return ftp_get_fileSize(ptFtp->cmd_skt,pcSrcFile);
}

/*brief    在FTP服务器中创建文件夹
 *param ： ptFtp:	   ftp操作符
 *param ： pcDirPath:  要创建的文件夹路径
 *return： 成功返回0 失败返回< 0
 */
__EX_API__ int wow_ftp_mkdir(Ftp_T* ptFtp,const char *pcDirPath)
{
	CHECK_RET_VAL_P(ptFtp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid.\n");
	CHECK_RET_VAL_P(pcDirPath,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid.\n");

	return ftp_mk_dir(ptFtp->cmd_skt,pcDirPath);
}


/*brief    下载FTP服务器文件
 *param ： ptFtp:	   ftp操作符
 *param ： pcSrcFile:  源文件在FTP服务器中的路径
 *param ： pcDestFile: 下载文件的存储路径(默认路径为/work/xxx)
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_ftp_file_download(Ftp_T* ptFtp,const char *pcSrcFile,const char *pcDestFile)
{

	CHECK_RET_VAL_P(ptFtp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid.\n");
	CHECK_RET_VAL_P(pcSrcFile && pcDestFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid.\n");

	return ftp_download_file(ptFtp->cmd_skt,ptFtp->param.host,pcSrcFile,pcDestFile);
}

 /*brief   上传FTP服务器文件
  *param ： ptFtp:	   ftp操作符
  *param ： pcSrcFile:  上传源文件的本地路径路径
  *param ： pcDestFile: FTP服务器上传路径
  *return： 成功返回0 失败返回<0
 */
__EX_API__ int wow_ftp_file_upload(Ftp_T* ptFtp,const char *pcSrcFile,const char *pcDestFile)
{

	CHECK_RET_VAL_P(ptFtp,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invalid.\n");
	CHECK_RET_VAL_P(pcSrcFile && pcDestFile,-PARAM_INPUT_DATA_IS_NULL,"param input data invalid.\n");

	return ftp_upload_file(ptFtp->cmd_skt,ptFtp->param.host,pcSrcFile,pcDestFile);
}

 /*brief   退出FTP服务器
  *param ： ptFtp:	   ftp操作符
  *return： 无
 */
__EX_API__ void wow_ftp_exit(Ftp_T** pptFtp)
{
	CHECK_RET_VOID_P(pptFtp && *pptFtp,"param input struct invalid.\n");

	Ftp_T* ftp = *pptFtp;

	wow_socket_close(ftp->cmd_skt);
	FREE(ftp);

	*pptFtp = NULL;
}

