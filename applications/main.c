/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     misonyo   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "ejdLeds.h"

int main(void)
{
    /* set LED0 pin mode to output */
    ejdLedApplication();
    led_on(1);

    while (1)
    {
        rt_thread_mdelay(500);
        led_tog(1);
    }

    return RT_EOK;
}
