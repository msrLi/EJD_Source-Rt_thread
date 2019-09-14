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
#include "wifiServer.h"
#include "transferCore.h"
#include "fal.h"


TransferCore *transferCore = NULL;


#define WIFI_POWER_PIN    GET_PIN(E, 3)
#define WIFI_RESET_PIN    GET_PIN(E, 2)

int main(void)
{
    /* set LED0 pin mode to output */
    fal_init();
    rt_kprintf("Version = V1.0.4\n");
    ejdLedApplication();
    transferCore = new TransferCore();
    transferCore->construct();
#if 0
    serverCore = new PrinterProcessCore();
    serverCore->construct();

    wifiServer = new WifiServerCore([](uint8_t *date, rt_size_t &size)->void {

    });
    wifiServer->construct();
#endif
    while (1) {
        rt_thread_mdelay(300);

    }

    return RT_EOK;
}
