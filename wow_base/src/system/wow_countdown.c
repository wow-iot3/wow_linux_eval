#include <time.h>
#include <sys/time.h>

#include "prefix/wow_check.h"
#include "prefix/wow_errno.h"
#include "prefix/wow_keyword.h"
#include "prefix/wow_common.h"

#include "system/wow_countdown.h"

struct __data_aligned__ countdown_t{
    struct timeval time;
};


/*brief    申请倒计时缓存
 *return： 成功返回倒计时操作符 失败返回NULL
 */
__EX_API__ CountDown_T* wow_cdown_new(void)
{
	CountDown_T* cdown = CALLOC(1,CountDown_T);
	CHECK_RET_VAL_P(cdown,NULL,"malloc CountDown_T failed!\n");
	
	cdown->time = (struct timeval){0, 0};
    return cdown;
}

/*brief    释放倒计时缓存
 *param ： pptCdown: 倒计时操作符
 *return： 无
 */
__EX_API__ void wow_cdown_free(CountDown_T** pptCdown)
{
	CHECK_RET_VOID(pptCdown && *pptCdown);

	FREE(*pptCdown);
	*pptCdown = NULL;
}


/*brief    倒计时参数置0
 *param ： ptCdown: 倒计时操作符
 *return： 无
 */
__EX_API__ void wow_cdown_reset(CountDown_T* ptCdown)
{
	CHECK_RET_VOID(ptCdown);
    ptCdown->time = (struct timeval){0, 0};
}

/*brief    倒计时参数置数
 *param ： ptCdown:   倒计时操作符
 *param ： nMsec: 倒计时时长(ms)
 *return： 无
 */
__EX_API__ void wow_cdown_set(CountDown_T* ptCdown, size_t nMsec)
{
	CHECK_RET_VOID(ptCdown);
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {nMsec / 1000, (nMsec % 1000) * 1000};
    timeradd(&now, &interval, &ptCdown->time);
}

/*brief    获取倒计时剩余时间
 *param ： ptCdown:   倒计时操作符
 *return： 返回倒计时剩余时间
 */
__EX_API__ int wow_cdown_remain(CountDown_T* ptCdown)
{
	CHECK_RET_VAL(ptCdown,0);

    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&ptCdown->time, &now, &res);
    return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}

/*brief    倒计时是否超时
 *param ： ptCdown:   倒计时操作符
 *return： 0-未超时  1-超时
 */
__EX_API__ int wow_cdown_is_expired(CountDown_T* ptCdown)
{
	CHECK_RET_VAL(ptCdown,0);

    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&ptCdown->time, &now, &res);
    return ((res.tv_sec < 0) || (res.tv_sec == 0 && res.tv_usec <= 0));
}


