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
#include <transferCore.h>
#include "crc32.h"

#define THREAD_PRIORITY       5
#define THREAD_STACK_SIZE     512
#define THREAD_TIMESLICE      10
#define MEMPOOL_SIZE         (70)
#define MEMPOOL_BLOCK_SIZE   (256)
#define EVENT_FLAGSYNC       (1 << 7)

static PRINTER_DATA_T gMemPool[MEMPOOL_SIZE] = {0};
#if 0
/* 小票信息存储结构体 */
#pragma pack(1)
typedef struct PRINTER_DATA_TAG {
    uint32_t length; /*<! 小票数据长度 */
    uint8_t *addr; /*<! 小票数据缓冲指针 */
    struct list_head list; /*<! linux 列表 */
} PRINTER_DATA_T;
#pragma pack()
#endif
TransferCore::TransferCore():
    mConsumerThread(NULL),
    mProductorThread(NULL),
    mMutex(NULL),
    mListSem(NULL),
    mPrinterHard(NULL),
    mWifiService(NULL)
{

}

TransferCore::~TransferCore()
{

}

void TransferCore::PrinterPorductor(void *parameter)
{
    rt_uint32_t e;
    TransferCore *thisP = (TransferCore*) parameter;
    while (true) {
        if (rt_event_recv(&thisP->mEvent, EVENT_FLAGSYNC,
                          RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                          RT_WAITING_FOREVER, &e) == RT_EOK) {
            rt_kprintf("thread1: OR recv event 0x%x\n", e);
            rt_kprintf("Receive data length=%d\n", thisP->mReceiveDate.lenght);
            // uint32_t bufSize = thisP->mReceiveDate.lenght + sizeof(PRINTER_DATA_T);
            uint8_t *bufTemp = (uint8_t *) rt_mp_alloc(&thisP->mMemp, RT_WAITING_FOREVER);
            if (bufTemp == RT_NULL) {
                rt_kprintf("There is no memory to malloc\n");
            } else {
                PRINTER_DATA_T *listTemp = (PRINTER_DATA_T *) bufTemp;
                listTemp->length = thisP->mReceiveDate.lenght;
                rt_memcpy(listTemp->addr,thisP->mReceiveDate.data_buf, listTemp->length);
                // listTemp->addr   = (uint8_t *)(bufTemp + sizeof(PRINTER_DATA_T));
                /* 加锁 */
                rt_mutex_take(thisP->mMutex, RT_WAITING_FOREVER);
                list_add_tail(&listTemp->list, &thisP->mWaitToSendL);
                rt_mutex_release(thisP->mMutex);
                /* 增加sem */
                rt_sem_release(thisP->mListSem);
            }

        } else {

        }
    }


}

int32_t  TransferCore::TransferDataToService(const PRINTER_DATA_T &pPrinData)
{
    uint8_t packageHeader[38];
    int32_t rc = 0;

    rt_memset(packageHeader, 0, sizeof(packageHeader));

    packageHeader[0] = '$';
    packageHeader[1] = 'B';
    packageHeader[2] = 'Y';
    packageHeader[3] = 'P';
    packageHeader[4] = '$';
    packageHeader[5] = 0x01;
    packageHeader[6] = 0x0C;
    packageHeader[7] = 0xE9;
    packageHeader[8] = 0;
    packageHeader[9] = 0;
    /* 存储数据有效包长度 */
    uint32_t packageLength = pPrinData.length + 24;
    *((uint32_t*)(&packageHeader[10])) = __ntohl(packageLength);

    uint32_t index = 0;
    *((uint32_t*)(&packageHeader[14])) = __ntohl(index);
    uint32_t mcuId[3];
    *((uint32_t*)(&packageHeader[18])) = __ntohl(mcuId[0]);
    *((uint32_t*)(&packageHeader[22])) = __ntohl(mcuId[1]);
    *((uint32_t*)(&packageHeader[26])) = __ntohl(mcuId[2]);

    uint32_t crc32Value = crc32(0, pPrinData.addr, pPrinData.length);
    *((uint32_t*)(&packageHeader[30])) = __ntohl(crc32Value);

    *((uint32_t*)(&packageHeader[34])) = __ntohl(pPrinData.length);

    /* wifi send header */
    rc = mWifiService->transferData(packageHeader, 38);
    /* wifi send boady data */
    rc |= mWifiService->transferData(pPrinData.addr, pPrinData.length);

    /* wait for serverice return */

    return rc;
}

void TransferCore::PrinterConsumerEntry(void *parameter)
{
    rt_err_t result;
    TransferCore *thisP = (TransferCore*) parameter;
    while (true) {
        result = rt_sem_take(thisP->mListSem, RT_WAITING_FOREVER);
        if (result != RT_EOK) {
            rt_kprintf("take sem failed\n");
            rt_thread_mdelay(500);
            continue;
        }

        /* --FIXME-- 检查 WIFI状态 */

        rt_mutex_take(thisP->mMutex, RT_WAITING_FOREVER);
        if (list_empty(&thisP->mWaitToSendL)) {
            rt_mutex_release(thisP->mMutex);
            continue;
        }
        struct list_head  *list = thisP->mWaitToSendL.next;
        list_del(list);
        rt_mutex_release(thisP->mMutex);
        PRINTER_DATA_T *dataPtr = list_entry(list, PRINTER_DATA_T, list);

        if (dataPtr == RT_NULL) {
            rt_kprintf("list_entry Get point failed\n");
            rt_mutex_take(thisP->mMutex, RT_WAITING_FOREVER);
            list_add(list, &thisP->mWaitToSendL);
            rt_mutex_release(thisP->mMutex);
            continue;
        }
        /* send data to serveice */
        thisP->TransferDataToService(*dataPtr);

    }
}



int32_t TransferCore::construct()
{
    int32_t rc = 0;
#if 1
    mEmptyL = LIST_HEAD_INIT(mEmptyL);
    mWaitToSendL = LIST_HEAD_INIT(mWaitToSendL);
#endif
    /* --FIXME-- wifi init */

    /* --FIXME-- init data read */

    rc  = rt_event_init(&mEvent, "sync event", RT_IPC_FLAG_FIFO);
    if (rc != RT_EOK) {
        rt_kprintf("init event failed.\n");
        return -1;
    }

    /* 创建一个动态互斥量 */
    mMutex = rt_mutex_create("List Mutex", RT_IPC_FLAG_FIFO);
    if (mMutex == RT_NULL) {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }

    mListSem = rt_sem_create("list sem", 0, RT_IPC_FLAG_FIFO);
    if (mListSem == RT_NULL) {
        rt_kprintf("Create sem failed. \n");
        return -1;
    }

    /* 初始化内存池对象 */
    rc = rt_mp_init(&mMemp, "Mempool", &gMemPool[0], sizeof(gMemPool), sizeof(gMemPool[0]));
    if (rc != RT_EOK) {
        rt_kprintf("memory pool failed.\n");
        return -1;
    }

    mConsumerThread = new rtthread::Thread(
        PrinterConsumerEntry, this,
        THREAD_STACK_SIZE,
        THREAD_PRIORITY, THREAD_TIMESLICE,
        "EJD_consumer");
    if (mConsumerThread != RT_NULL) {
        mConsumerThread->start();

    } else {
        rt_kprintf("Create EJD_consumer failed\n");
        rc = -2;
    }

    mProductorThread = new rtthread::Thread(
        PrinterPorductor, this,
        THREAD_STACK_SIZE,
        THREAD_PRIORITY + 1, THREAD_TIMESLICE,
        "EJD_productor");
    if (mProductorThread != RT_NULL) {
        mProductorThread->start();
    } else {
        rt_kprintf("Create EJD_consumer failed\n");
        rc = -2;
    }

#if 0
    /* initid consumer thread */
    mConsumerTid = rt_thread_create("EJD_consumer",
                                    PrinterConsumerEntry, this,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY, THREAD_TIMESLICE);

    if (mConsumerTid != RT_NULL) {
        rt_thread_startup(mConsumerTid);

    } else {
        rc = -2;
    }

    mProductorTid = rt_thread_create("EJD_consumer",
                                     PrinterPorductor, this,
                                     THREAD_STACK_SIZE,
                                     THREAD_PRIORITY + 1, THREAD_TIMESLICE);

    if (mProductorTid != RT_NULL) {
        rt_thread_startup(mProductorTid);

    } else {
        rc = -2;
    }
#endif
    /* --TODO-- init buffer */
    mPrinterHard = new PrinterHardware([this](PRINTER_BUF_T & date)->int32_t {
        rt_memcpy(&mReceiveDate, &date, sizeof(PRINTER_BUF_T));
        rt_event_send(&mEvent, EVENT_FLAGSYNC);
    });

    if (mPrinterHard != NULL) {
        mPrinterHard->construct();
    } else {
        rc = -1;
    }

    mWifiService = new WifiServerCore([](uint8_t *date, rt_size_t &size)->void {
        int32_t rc = 0;
        return;
    });

    if (mWifiService != NULL) {
        mWifiService->construct();
    }

    return rc;
}

int32_t TransferCore::destruct()
{
#if 0
    rt_thread_delete(mConsumerTid);
    rt_thread_delete(mProductorTid);
#endif
    rt_mutex_delete(mMutex);
    rt_sem_delete(mListSem);

    rt_event_detach(&mEvent);
    mPrinterHard->destruct();
    delete mPrinterHard;

    /* 执行内存池脱离 */
    rt_mp_detach(&mMemp);

    /* list delete */
    return 0;
}


