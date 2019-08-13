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
        int32_t PrinterPorductor(PRINTER_BUF_T &date);
        void PrinterConsumerEntry(void *parameter);

    private:
        PrinterProcessCore(const PrinterProcessCore &rhs) = delete;
        PrinterProcessCore &operator=(const PrinterProcessCore &rhs) = delete;

    private:
        struct list_head mEmptyL;
        struct list_head mWaitToSendL;
        rt_thread_t mConsumerTid;
        PrinterHardware  *mPrinterHard;
}

#endif 

