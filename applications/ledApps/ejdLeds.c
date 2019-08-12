/**
 * @file ejdLeds.c
 * @brief binsonli
 * @author  lenovo <binsonli666@gmaial.com>
 * @version 1.0.0
 * @date 2019-08-12
 */

#include "ejdLeds.h"

static rt_thread_t ledTid = RT_NULL;
/* defined the LED0 pin: PC4 */
#define LED0_PIN    GET_PIN(C, 4)

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

/* led 线程的入口函数 */
static void led_threads(void *parameter)
{
    rt_uint32_t count = 0;
    led_add_device(LED0_PIN);
		led_blink(1, 100, 100);
    while (1) {
        /* 线程1采用低优先级运行，一直打印计数值 */
        // rt_kprintf("thread1 count: %d\n", count ++);
        led_process(10);
        rt_thread_mdelay(10);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief ejdLedApplication
 *
 * @return
 */
/* --------------------------------------------------------------------------*/
int ejdLedApplication(void)
{
    /* 创建线程1，名称是thread1，入口是thread1_entry*/
    ledTid = rt_thread_create("ejd_Leds",
                              led_threads, RT_NULL,
                              THREAD_STACK_SIZE,
                              THREAD_PRIORITY, THREAD_TIMESLICE);

    /* 如果获得线程控制块，启动这个线程 */
    if (ledTid != RT_NULL) {
        rt_thread_startup(ledTid);
    }

    return 0;
}


#ifdef RT_USING_TC
static void _tc_cleanup()
{
    /* 调度器上锁，上锁后，将不再切换到其他线程，仅响应中断 */
    rt_enter_critical();

    /* 删除线程 */
    if (ledTid != RT_NULL && ledTid->stat != RT_THREAD_CLOSE)
        rt_thread_delete(ledTid);

    /* 调度器解锁 */
    rt_exit_critical();

    /* 设置TestCase状态 */
    tc_done(TC_STAT_PASSED);
}

int _tc_ejdLed_simple()
{
    /* 设置TestCase清理回调函数 */
    tc_cleanup(_tc_cleanup);
    ejdLedApplication();

    /* 返回TestCase运行的最长时间 */
    return 100;
}
/* 输出函数命令到finsh shell中 */
FINSH_FUNCTION_EXPORT(_tc_ejdLed_simple, a dynamic thread example);
#endif
