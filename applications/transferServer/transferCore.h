/**
 * @file TransferCore.h
 * @brief
 * @author  lenovo <xxx@lenovo.com>
 * @version 1.0.0
 * @date 2019-08-13
 */

/* Copyright(C) 2009-2019, Lenovo Inc.
 * All right reserved
 *
 */
#ifndef __PRINTER_PROCESS_CORE_H__
#define __PRINTER_PROCESS_CORE_H__

#include "printerHard.h"
#include "wifiServer.h"
#include "list.h"
#include "Thread.h"


/* 小票信息存储结构体 */
#pragma pack(1)
typedef struct PRINTER_DATA_TAG {
    uint32_t length; /*<! 小票数据长度 */
    uint8_t *addr; /*<! 小票数据缓冲指针 */
    struct list_head list; /*<! linux 列表 */
} PRINTER_DATA_T;
#pragma pack()

class PrinterHardware;
class WifiServerCore;
class TransferCore
{
public:
    TransferCore();
    ~TransferCore();

public:
    int32_t construct();
    int32_t destruct();

private:
    int32_t TransferDataToService(const PRINTER_DATA_T *pPrinData);

private:
    static void PrinterPorductor(void *parameter);
    static void PrinterConsumerEntry(void *parameter);

private:
    TransferCore(const TransferCore &rhs) = delete;
    TransferCore &operator=(const TransferCore &rhs) = delete;

private:
#if 1
    struct list_head mEmptyL;
    struct list_head mWaitToSendL;
#endif
    rtthread::Thread          *mConsumerThread;
    rtthread::Thread          *mProductorThread;
    rt_mutex_t          mMutex;
    rt_sem_t            mListSem;
    PrinterHardware     *mPrinterHard;
    WifiServerCore      *mWifiService;
    struct rt_event     mEvent;
    struct rt_mempool   mMemp;
    PRINTER_BUF_T       mReceiveDate;

};

#endif

