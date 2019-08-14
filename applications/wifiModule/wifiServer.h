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
#include "Thread.h"
#include <functional>

class WifiServerCore
{
public:
    WifiServerCore(std::function<void (uint8_t *date, rt_size_t &size)> cb);
    ~WifiServerCore();

public:
    int32_t HwPowerUp();
    int32_t HwPowerDown();
    int32_t HwReset();

public:
    int32_t transferData(uint8_t *data, rt_size_t size, std::function<int32_t (uint8_t *date, rt_size_t &size)> cb);

private:
    static rt_err_t serverReceiveIrq(rt_device_t dev, rt_size_t size);
    static void wifiServerEntry(void *parameter);
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
    rtthread::Thread    *mReceiveThread;
    std::function<void (uint8_t *date, rt_size_t &size)>  mFunCb;
    std::function<void (uint8_t *date, rt_size_t &size)>  mTrensCb;
    bool        mCall;
    struct rt_messagequeue mMessage;
};

#endif
