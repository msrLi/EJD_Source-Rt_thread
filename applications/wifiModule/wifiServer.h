/**
 * @file wifiServer.h
 * @brief
 * @author  lenovo <xxx@lenovo.com>
 * @version 1.0.0
 * @date 2019-08-14
 */

/* Copyright(C) 2009-2019, Lenovo Inc.
 * All right reserved
 *
 */

#ifndef __WIFI_SERVICE_CORE__
#define __WIFI_SERVICE_CORE__

#include <rtthread.h>
#include "drv_gpio.h"

class WifiServerCore
{
public:
    WifiServerCore();
    ~WifiServerCore();

public:
    int32_t HwPowerUp();
    int32_t HwPowerDown();
    int32_t HwReset();

public:
    int32_t construct();
    int32_t destruct();

private:
    WifiServerCore(const WifiServerCore &rhs) = delete;
    WifiServerCore &operator=(const WifiServerCore &rhs) = delete;

private:
    rt_base_t   powerPin;
    rt_base_t   resetPin;
    rt_device_t mTransferDev;
};

#endif
