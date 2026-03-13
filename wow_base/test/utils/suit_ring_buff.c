#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "greatest/greatest.h"
#include "wow_test_config.h"

#include "utils/wow_ring_buff.h"

#define MOD_TAG "[ringbuffer]"

#if SUIT_RUN_ERROR_TEST
TEST test_ringbuff_error(void)
{
    printf(MOD_TAG"suit_ringbuffer-----test_ringbuffer_error\n");

	int ret = 0;
	
    RingBuffer_T* test_rbuf_00 = NULL;
	RingBuffer_T* test_rbuf_01 = NULL;
	RingBuffer_T* test_rbuf_02 = NULL;
	
    test_rbuf_00 = wow_ringbuf_create(1024);
    GREATEST_ASSERT(test_rbuf_00);

	wow_ringbuf_destroy(&test_rbuf_00);

    test_rbuf_01 = wow_ringbuf_create(2048);
    GREATEST_ASSERT(test_rbuf_01);

    test_rbuf_02 = wow_ringbuf_create(1024);
    GREATEST_ASSERT(test_rbuf_02);
	
	ret = wow_ringbuf_write(test_rbuf_01, NULL,2);
	GREATEST_ASSERT(ret != 0);
	ret = wow_ringbuf_write(test_rbuf_01, "AAA",0);
	GREATEST_ASSERT(ret != 0);
	ret = wow_ringbuf_read(test_rbuf_01, NULL,0);
	GREATEST_ASSERT(ret != 0);

    wow_ringbuf_destroy(&test_rbuf_01);
    wow_ringbuf_destroy(&test_rbuf_02);
	wow_ringbuf_destroy(&test_rbuf_02);

	ret = wow_ringbuf_getFree(test_rbuf_01);
	GREATEST_ASSERT(ret != 0);
	ret = wow_ringbuf_getUsed(test_rbuf_01);
	GREATEST_ASSERT(ret != 0);	
	
    PASS();
}
#endif


TEST test_ringbuff_fun(void)
{
    printf(MOD_TAG"suit_ringbuffer-----test_ringbuffer_fun\n");
   
    const char *tmp = "hello world";
    ssize_t ret = 0;
    size_t len = 0;
	int i = 0;

	RingBuffer_T* rbuf = wow_ringbuf_create(1024);
	
    for (i = 0; i < 200; i++) {
        ret = wow_ringbuf_write(rbuf, tmp, strlen(tmp));
        if(ret == 0) break;
		len += ret;
    }

	GREATEST_ASSERT(wow_ringbuf_getFree(rbuf) == 1);
	GREATEST_ASSERT(wow_ringbuf_getUsed(rbuf) == 1023);
	
    char tmp2[15] = {0};
    memset(tmp2, 0, sizeof(tmp2));
    wow_ringbuf_read(rbuf, tmp2, sizeof(tmp2)-1);
	GREATEST_ASSERT(!strncmp("hello worldhel",tmp2, sizeof(tmp2)-1));

    memset(tmp2, 0, sizeof(tmp2));
    wow_ringbuf_read(rbuf, tmp2, sizeof(tmp2)-1);
    GREATEST_ASSERT(!strncmp("lo worldhello ",tmp2, sizeof(tmp2)-1));
	
	GREATEST_ASSERT(wow_ringbuf_getFree(rbuf) == 29);
	GREATEST_ASSERT(wow_ringbuf_getUsed(rbuf) == 995);

	for (i = 0; i < 200; i++) {
        ret = wow_ringbuf_write(rbuf, tmp, strlen(tmp));
		if(ret == 0) break;
		len += ret;
    }

   
	for(i = 0; i < 200; i++){
		memset(tmp2, 0, sizeof(tmp2));
		ret = wow_ringbuf_read(rbuf, tmp2, sizeof(tmp2)-1);
		if(ret <= 0) break;
	}

	GREATEST_ASSERT(wow_ringbuf_getFree(rbuf) == 1024);
	GREATEST_ASSERT(wow_ringbuf_getUsed(rbuf) == 0);

	wow_ringbuf_destroy(&rbuf);
    PASS();
}

SUITE(suit_ring_buff)
{
#if SUIT_RUN_ERROR_TEST
    RUN_TEST(test_ringbuff_error);
#endif
    RUN_TEST(test_ringbuff_fun);
}



