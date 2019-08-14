/**
 * @file printerServer.h
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
#include "list.h"
#include "Thread.h"

class PrinterHardware;
class PrinterProcessCore
{
public:
    PrinterProcessCore();
    ~PrinterProcessCore();

public:
    int32_t construct();
    int32_t destruct();

private:
    static void PrinterPorductor(void *parameter);
    static void PrinterConsumerEntry(void *parameter);

private:
    PrinterProcessCore(const PrinterProcessCore &rhs) = delete;
    PrinterProcessCore &operator=(const PrinterProcessCore &rhs) = delete;

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
    struct rt_event     mEvent;
    struct rt_mempool   mMemp;
    PRINTER_BUF_T       mReceiveDate;

};

#endif

