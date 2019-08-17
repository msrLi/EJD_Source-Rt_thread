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
#include "printerServer.h"
#include "wifiServer.h"


PrinterProcessCore *serverCore = NULL;
WifiServerCore  *wifiServer = NULL;


#define WIFI_POWER_PIN    GET_PIN(E, 3)
#define WIFI_RESET_PIN    GET_PIN(E, 2)

int main(void)
{
    /* set LED0 pin mode to output */
    ejdLedApplication();

    serverCore = new PrinterProcessCore();
    serverCore->construct();

    wifiServer = new WifiServerCore([](uint8_t *date, rt_size_t &size)->void {

    });
    wifiServer->construct();

    while (1) {
        rt_thread_mdelay(300);

    }

    return RT_EOK;
}
