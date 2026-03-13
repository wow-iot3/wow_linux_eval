#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "prefix/wow_check.h"
#include "prefix/wow_common.h"
#include "prefix/wow_errno.h"

#include "wow_base_config.h"

//放入临时缓存目录中
#define DEFAULT_SHM_FILE_PATH "/dev/shm"
	
#define SHM_GET_MMAP_FILENAME(fpath, key) \
		snprintf(fpath, sizeof(fpath), "%s.%zu",DEFAULT_SHM_FILE_PATH,key)
		
#if 0
static void* shm_map_zero(size_t size)
{
	int fd = open("/dev/zero", O_RDWR, 0666);
	CHECK_RET_VAL_P_A(fd > 0,NULL,"open /dev/zero failed!\n");
	
    size_t aligned_size = Align4(size); 
	CHECK_RET_VAL_P(aligned_size > 0,NULL,"param input invalid!\n");
	
	int page_size = sysconf (_SC_PAGE_SIZE);//getpagesize
	// 将申请内存的尺寸, 对齐到内存页的尺寸
	size_t alloc_size = (aligned_size + page_size - 1) / page_size * page_size;

	void* addr  = mmap(NULL, alloc_size, PROT_READ|PROT_WRITE,MAP_SHARED, fd, 0);
	close(fd);
	
	CHECK_RET_VAL_P_A(addr != (void *)MAP_FAILED,NULL,"mmap failed!\n");
	
	return addr;
}
static void shm_unmap_zero(void* addr,size_t size)
{
	munmap(addr,size);
}
#endif

static void* shm_map_file(size_t key,size_t size,bool create)
{
	int ret  = -1;
	void* addr = NULL;
	char shm_name[64] ={0};
#if 0
	size_t h1 = wow_hash_data(file,strlen(file),HASH_DOBBS);
	size_t h2 = wow_hash_data(file,strlen(file),HASH_DOBBS);
	sprintf(shm_name, "/ShM.%xH%x", h1, h2);

	int fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0644);
	CHECK_RET_VAL_P_A(fd > 0,NULL,"shm_open false!\n");
#endif
	SHM_GET_MMAP_FILENAME(shm_name,key);
	int fd = open(shm_name, O_RDWR);
    if (fd >= 0) {
        struct stat st;
		ret = fstat(fd, &st);
		CHECK_RET_VAL_ERRNO_P(ret == 0,-SYSTEM_FSTAT_FAILED,"fstat failed!\n");
		if (st.st_size < size) {
        }
    } else {
		CHECK_RET_VAL_ERRNO_P(create,-SYSTEM_OPEN_FAILED,"open failed!\n");
		mode_t old_mast = umask(0);
        fd = open(shm_name, O_RDWR | O_CREAT, 0666);
        umask(old_mast);
		CHECK_RET_VAL_ERRNO_P(fd > 0,-SYSTEM_OPEN_FAILED,"open failed!\n");
    }
	ret = ftruncate(fd, size);
	CHECK_RET_GOTO_ERRNO_P(ret == 0,out,-SYSTEM_FTRUNCATE_FAILED,"ftruncate failed!\n");
	
	size_t aligned_size = Align4(size); 
	
	int page_size = sysconf (_SC_PAGE_SIZE);//getpagesize
	// 将申请内存的尺寸, 对齐到内存页的尺寸
	size_t alloc_size = (aligned_size + page_size - 1) / page_size * page_size;

	addr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
	CHECK_RET_GOTO_ERRNO_P(addr && addr != (void *)MAP_FAILED,out,-SYSTEM_MMAP_FAILED,"mmap failed!\n");

	close(fd);
	return addr;
out:
	close(fd);
	return NULL;
}

static void shm_unmap_file(size_t key)
{
	char shm_name[64] ={0};

#if 0
	munmap(addr,size);
	size_t h1 = wow_hash_data(file,strlen(file),HASH_DOBBS);
	size_t h2 = wow_hash_data(file,strlen(file),HASH_DOBBS);
		
	sprintf(shm_name, "/ShM.%xH%x", h1, h2);
	shm_unlink(shm_name);
#endif
	SHM_GET_MMAP_FILENAME(shm_name,key);
	unlink(shm_name);
}

static void* shm_map_shm(size_t key,size_t size,bool create)
{
	 int shmid = 0;

	 if(create){
		shmid = shmget(key, size,IPC_CREAT | 0666);
	 }else{
		shmid = shmget(key, size,0666);
	 }
	 CHECK_RET_VAL_ERRNO_P(shmid >= 0,-SYSTEM_SHMGET_FAILED,"shmget failed!\n");
	
	 void* addr = shmat(shmid, (void *)0, 0);	
	 CHECK_RET_VAL_ERRNO_P(addr != (void *)-1,-SYSTEM_SHMAT_FAILED,"shmat failed!\n");

#if 0	 
	 ret = shmctl(shmid, IPC_STAT, &shmbuf);
	 CHECK_RET_VAL_P_A(ret >= 0,NULL,"shmctl false!\n");
	
	 struct shmid_ds shmbuf;
	 shmbuf.shm_perm.uid = getuid();
	 shmbuf.shm_perm.gid = getpid();
	 ret = shmctl(shmid, IPC_SET, &shmbuf);
	 CHECK_RET_VAL_P_A(ret >= 0,NULL,"shmctl false!\n");
	
	 /* Remove the segment once use count hits zero.
	  * We will not attach to this segment again, since it is
	  * anonymous memory, so it is ok to mark it for deletion.
	  */
	 ret =	shmctl(shmid, IPC_RMID, NULL);
	 CHECK_RET_VAL_P_A(ret >= 0,NULL,"shmctl false!\n");
#endif
	return addr;
}
static void shm_unmap_shm(size_t key , void* addr)
{
	int shmid = shmget(key, 0, 0666);
	shmctl(shmid, IPC_RMID, NULL);
	shmdt(addr);
}
  

/*brief    检测共享内存是否有效
 *return： 有效返回true 失败返回false
 */
bool wow_shm_check(size_t key)
{
#if WOW_SHM_MMAP_SUPPORT
	char sname[64] ={0};
	SHM_GET_MMAP_FILENAME(sname,key);	
	return access(sname, F_OK) == 0;
#endif

#if WOW_SHM_SHMGET_SUPPORT
	return shmget(key, 0, 0666) >= 0;
#endif
	return false;
}


/*brief    创建共享内存
 *param ： snKey  ：共享内存标识字符
 *param ： nSize  ：共享内存存储大小
 *return： 成功返回共享内存地址 失败返回NULL
 */
void* wow_shm_create(size_t key,size_t size)
{
#if WOW_SHM_MMAP_SUPPORT
	return shm_map_file(key, size,true);
#endif
	
#if WOW_SHM_SHMGET_SUPPORT
	return shm_map_shm(key, size,true);
#endif
	return NULL;
}

/*brief    附着共享内存
 *param ： snKey  ：共享内存标识字符
 *param ： nSize  ：共享内存存储大小
 *return： 成功返回共享内存地址 失败返回NULL
 */
void* wow_shm_attch(size_t key,size_t size)
{
#if WOW_SHM_MMAP_SUPPORT
	return shm_map_file(key, size,false);
#endif
	
#if WOW_SHM_SHMGET_SUPPORT
	return shm_map_shm(key, size,false);
#endif
	return NULL;
}

/*brief    移除共享内存
 *param ： snKey  ：共享内存标识字符
 *param ： pAddr  ：共享内存地址
 *return： 无
 */
void wow_shm_remove(size_t key,void* addr)
{
#if WOW_SHM_MMAP_SUPPORT	
	return shm_unmap_file(key);
#endif
		
#if WOW_SHM_SHMGET_SUPPORT
	return shm_unmap_shm(key, addr);
#endif
} 


