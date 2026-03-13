#ifndef __WOW_BASE_SYSTEM_WOW_COUNTDOWN_H_
#define __WOW_BASE_SYSTEM_WOW_COUNTDOWN_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef struct countdown_t CountDown_T;


/*brief    申请倒计时缓存
 *return： 成功返回倒计时操作符 失败返回NULL
 */
CountDown_T* wow_cdown_new(void);

/*brief    释放倒计时缓存
 *param ： pptCdown: 倒计时操作符
 *return： 无
 */
void wow_cdown_free(CountDown_T** pptCdown);

/*brief    倒计时参数置0
 *param ： ptCdown: 倒计时操作符
 *return： 无
 */
void wow_cdown_reset(CountDown_T* ptCdown);

/*brief    倒计时参数置数
 *param ： ptCdown:   倒计时操作符
 *param ： timeout: 倒计时时长(ms)
 *return： 无
 */
void wow_cdown_set(CountDown_T* ptCdown, size_t timeout);

/*brief    获取倒计时剩余时间
 *param ： ptCdown:   倒计时操作符
 *return： 返回倒计时剩余时间
 */
int wow_cdown_remain(CountDown_T* ptCdown);

/*brief    倒计时是否超时
 *param ： ptCdown:   倒计时操作符
 *return： 0-未超时  1-超时
 */
int wow_cdown_is_expired(CountDown_T* ptCdown);

#ifdef __cplusplus
}
#endif

#endif

