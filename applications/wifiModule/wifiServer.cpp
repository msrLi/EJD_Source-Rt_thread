/**
 * @file wifiServer.cpp
 * @brief
 * @author  lenovo <xxx@lenovo.com>
 * @version 1.0.0
 * @date 2019-08-14
 */

/* Copyright(C) 2009-2019, Lenovo Inc.
 * All right reserved
 *
 */
#include "wifiServer.h"

#define WIFI_POWER_PIN    GET_PIN(E, 3)
#define WIFI_RESET_PIN    GET_PIN(E, 2)

#define WIFI_TRANSFER_UART_NAME       "uart3"

WifiServerCore::WifiServerCore():
    mTransferDev(NULL)
{
    powerPin  = WIFI_POWER_PIN;
    resetPin = WIFI_RESET_PIN;
}

WifiServerCore::~WifiServerCore()
{

}

int32_t WifiServerCore::HwPowerUp()
{
    rt_pin_write(powerPin, PIN_HIGH);
    return 0;
}

int32_t WifiServerCore::HwPowerDown()
{
    rt_pin_write(powerPin, PIN_LOW);
    return 0;
}

int32_t WifiServerCore::HwReset()
{
    rt_pin_write(resetPin, PIN_LOW);
    rt_thread_mdelay(500);
    rt_pin_write(resetPin, PIN_HIGH);
    rt_thread_mdelay(100);
    return 0;
}

int32_t WifiServerCore::construct()
{
    rt_pin_mode(powerPin, PIN_MODE_OUTPUT);
    rt_pin_mode(resetPin, PIN_MODE_OUTPUT);
    HwPowerUp();

    mTransferDev = rt_device_find(WIFI_TRANSFER_UART_NAME);
    if (!mTransferDev) {
        rt_kprintf("find %s failed!\n", WIFI_TRANSFER_UART_NAME);
        return RT_ERROR;
    }

}

int32_t WifiServerCore::destruct()
{
    HwPowerDown();
    return 0;
}

