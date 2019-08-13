/**
 * @file printerServer.cpp
 * @brief  about some server
 * @author  lenovo <xxx@lenovo.com>
 * @version 1.0.0
 * @date 2019-08-12
 */

/* Copyright(C) 2009-2019, Lenovo Inc.
 * All right reserved
 *
 */
#include <rtthread.h>
#include <printerServer.h>

#define THREAD_PRIORITY       6
#define THREAD_STACK_SIZE     512
#define THREAD_TIMESLICE      5

/* 小票信息存储结构体 */
typedef struct PRINTER_DATA_TAG {
    uint8_t *addr; /*<! 小票数据缓冲指针 */
    uint32_t index; /*<! 小票数据缓冲指针索引保存 */
    uint32_t length; /*<! 小票数据长度 */
    struct list_head list; /*<! linux 列表 */
} PRINTER_DATA_T;

PrinterProcessCore::PrinterProcessCore():
    mEmptyL(NULL),
    mWaitToSendL(NULL),
    mConsumerTid(NULL),
    mPrinterHard(NULL)
{

}

PrinterProcessCore::~PrinterProcessCore()
{

}

int32_t PrinterProcessCore::PrinterPorductor(PRINTER_BUF_T &date)
{
    return 0;
}

void PrinterProcessCore::PrinterConsumerEntry(void *parameter)
{

}


int32_t PrinterProcessCore::construct()
{
    int32_t rc = 0;
    mEmptyL = LIST_HEAD_INIT(mEmptyL);
    mWaitToSendL = LIST_HEAD_INIT(mWaitToSendL);
    /* --FIXME-- wifi init */

    /* --FIXME-- init data read */
    
    /* initid consumer thread */
    mConsumerTid = rt_thread_create("EJD_consumer",
            consumer_thread_entry, RT_NULL,
            THREAD_STACK_SIZE,
            THREAD_PRIORITY + 1, THREAD_TIMESLICE);

    if (mConsumerTid != RT_NULL) {
        rt_thread_startup(consumer_tid);
    
    }else {
        rc = -2;
    }

    /* --TODO-- init buffer */
    mPrinterHard = new PrinterHardware([this](PRINTER_BUF_T &date)->int32_t {
                return PrinterPorductor(date);
            });

    if (mPrinterHard != NULL) {
        mPrinterHard->construct(); 
    }else{
        rc = -1;
    }

    return rc;
}

int32_t PrinterProcessCore::destruct()
{

    return 0;
}


