#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>    
#include <sys/time.h>   
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h> 
#include <fcntl.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"
#include "prefix/wow_cpu.h"

#include "system/wow_atomic.h"
#include "system/wow_lock.h"


/*********************************************atomic**************************************************/
__EX_API__ int wow_atomic_lock(atomic_lock_t *lock)
{
	CHECK_RET_VAL_P(lock,-1,"param input invalid!\n");
	int value = 1;

    size_t ncpu = wow_cpu_count();
    while (1)
    {
        if (*lock == 0 && atomic_compare_and_swap_bool(lock, 0, value)) return 0;
        if (ncpu > 1)
        {
            size_t i, n;
            for (n = 1; n < 2048; n <<= 1){
                for (i = 0; i < n; i++) ;
                if (*lock == 0 && atomic_compare_and_swap_bool(lock, 0, value))  return 0;
            }
        }
        sched_yield();
    }
	return -1;
}


__EX_API__ int wow_atomic_trylock(atomic_lock_t *lock)
{
	CHECK_RET_VAL_P(lock,-1,"param input invalid!\n");
	
	if(*(lock) == 0 && atomic_compare_and_swap_bool(lock, 0, 1)) return 0;
	
    return -1;

}

__EX_API__ int wow_atomic_unlock(atomic_lock_t *lock)
{
	CHECK_RET_VAL_P(lock,-1,"param input invalid!\n");
	
	*(lock) = 0;
    return 0;
}

/*********************************************mutex**************************************************/
__EX_API__ int wow_mutex_lock_init(mutex_lock_t *lock)
{
	CHECK_RET_VAL_P(lock,-1,"param input invalid!\n");

	int ret = -1;
    pthread_mutexattr_t  attr;

	ret = pthread_mutexattr_init(&attr);
	CHECK_RET_VAL_P_A(ret == 0, -1, "pthread_mutexattr_init failed\n");

	ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	CHECK_RET_VAL_P_A(ret == 0, -1, "pthread_mutexattr_settype failed\n");
	
	ret = pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);
	CHECK_RET_VAL_P_A(ret == 0, -1,"pthread_mutexattr_setpshared failed!\n");

    pthread_mutex_init(lock, NULL);

	ret = pthread_mutexattr_destroy(&attr);
	CHECK_RET_VAL_P_A(ret == 0, -1, "pthread_mutexattr_destroy failed\n");
	
    return 0;
}

__EX_API__ int wow_mutex_lock_exit(mutex_lock_t *lock)
{
    CHECK_RET_VAL_P(lock,-1,"param input invalid!\n");

    pthread_mutex_destroy(lock);

	return  0;
}


__EX_API__ int wow_mutex_trylock(mutex_lock_t *lock)
{
    CHECK_RET_VAL_P(lock,-1,"param input invalid!\n");

	return pthread_mutex_trylock(lock);
}

int wow_mutex_lock(mutex_lock_t *lock)
{
    CHECK_RET_VAL_P(lock,-1,"param input invalid!\n");

	int ret = pthread_mutex_lock(lock);
	CHECK_RET_VAL_P_A(ret == 0 || errno == 0, -1,"pthread_mutex_lock failed!\n");
	
	return 0;
}

int wow_mutex_unlock(mutex_lock_t *lock)
{
    CHECK_RET_VAL_P(lock,-1,"param input invalid!\n");

	int ret = pthread_mutex_unlock(lock);
	CHECK_RET_VAL_P_A(ret == 0 || errno == 0, -1,"pthread_mutex_unlock failed!\n");
	
    return 0;
}


/*********************************************sem**************************************************/
typedef struct{
	int semid;
	struct sembuf sb;
}SemLock_T;

sem_lock_t wow_sem_new(size_t key)
{
	SemLock_T* sem = CALLOC(1, SemLock_T);
	CHECK_RET_VAL_P(sem,NULL,"malloc SemLock_T failed");
	
	sem->semid = semget(key,1,0666|IPC_CREAT);	
	CHECK_RET_GOTO_P_A(sem->semid >= 0, out,"semget failed!\n");

	semctl(sem->semid, 0, SETVAL, 1); //初始化信号量为1

	return (sem_lock_t)sem;
out:
	FREE(sem);
	return NULL;
}

int wow_sem_lock(sem_lock_t self) 
{
	SemLock_T* sem = (SemLock_T*)self;
	CHECK_RET_VAL_P(sem,-1,"param input invalid!\n");
	
	sem->sb.sem_num = 0;
	sem->sb.sem_op  = -1;
	sem->sb.sem_flg = SEM_UNDO;

	return semop(sem->semid,&sem->sb,1);
}

int wow_sem_unlock(sem_lock_t self) 
{
	SemLock_T* sem = (SemLock_T*)self;
	CHECK_RET_VAL_P(sem,-1,"param input invalid!\n");
	
	sem->sb.sem_num = 0;
	sem->sb.sem_op  = 1;
	sem->sb.sem_flg = SEM_UNDO;

	return semop(sem->semid,&sem->sb,1);

}

int wow_sem_free(sem_lock_t self)
{
	SemLock_T* sem = (SemLock_T*)self;
	CHECK_RET_VAL_P(sem,-1,"param input invalid!\n");

	semctl(sem->semid, 0, IPC_RMID, 0); 
	FREE(sem);

	return 0;
}

/*********************************************file**************************************************/
static int file_lock_exec(int fd, int cmd, int type)
{
    struct flock lock;
    int ret;

    memset(&lock, 0, sizeof(lock));
    lock.l_type = type;
    lock.l_whence = SEEK_SET;
    do
    {
        if ((ret=fcntl(fd, cmd, &lock)) != 0){
            ret = errno != 0 ? errno : ENOMEM;
        }
    } while (ret == EINTR);

    return ret;
}

int wow_file_read_lock(int fd)
{
    return file_lock_exec(fd, F_SETLKW, F_RDLCK);
}

int wow_file_write_lock(int fd)
{
    return file_lock_exec(fd, F_SETLKW, F_WRLCK);
}

int wow_file_unlock(int fd)
{
    return file_lock_exec(fd, F_SETLKW, F_UNLCK);
}
