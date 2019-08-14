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

#define WIFI_TRANSFER_UART_NAME       "uart2"

#define THREAD_PRIORITY       10
#define THREAD_STACK_SIZE     512
#define THREAD_TIMESLICE      10

/* 串口接收消息结构*/
struct rx_msg {
    rt_device_t dev;
    rt_size_t size;
} MSG_POOL[2];


WifiServerCore::WifiServerCore(std::function<void (uint8_t *date, rt_size_t &size)> cb):
    mTransferDev(NULL),
    mReceiveThread(NULL),
    mFunCb(cb),
    mCall(false)
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

rt_err_t WifiServerCore::serverReceiveIrq(rt_device_t dev, rt_size_t size)
{
    WifiServerCore *thisP = (WifiServerCore*)  dev->user_data;
    struct rx_msg msg;
    rt_err_t result;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(&thisP->mMessage, &msg, sizeof(msg));
    if (result == -RT_EFULL) {
        /* 消息队列满 */
        rt_kprintf("message queue full！\n");
    }
    return result;
}

void WifiServerCore::wifiServerEntry(void *parameter)
{
    WifiServerCore *thisP = (WifiServerCore*) parameter;
    uint8_t rx_buffer[RT_SERIAL_RB_BUFSZ + 1];
    rt_size_t rx_length;
    struct rx_msg msg;
    rt_err_t result;

    while (true) {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息*/
        result = rt_mq_recv(&thisP->mMessage, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result != RT_EOK) {
            rt_kprintf("rt_mq_reve failed\n");
            continue;
        }

        /* 从串口读取数据*/
        rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
        rx_buffer[rx_length] = '\0';
        /* 通过串口设备 serial 输出读取到的消息 */
        // rt_device_write(serial, 0, rx_buffer, rx_length);
        /* 打印数据 */
        // rt_kprintf("%s\n",rx_buffer);

        if (!thisP->mCall &&  thisP->mFunCb) {
            thisP->mFunCb(rx_buffer, rx_length);
        } else if (thisP->mCall && thisP->mTrensCb) {
            thisP->mTrensCb(rx_buffer, rx_length);
            thisP->mTrensCb = NULL;
        }
        thisP->mCall = false;
    }
}

int32_t WifiServerCore::transferData(uint8_t *data, rt_size_t size, std::function<int32_t (uint8_t *date, rt_size_t &size)> cb)
{
    mTrensCb = cb;
    mCall = false;
    uint32_t index = size / 1024;
    uint32_t reserved = size % 1024;
    uint8_t *dataPtr = data;

    for (uint32_t i = 0; i < index; i++) {
        rt_device_write(mTransferDev, 0, dataPtr, 1024);
        dataPtr += 1024;
    }

    if (reserved) {
        rt_device_write(mTransferDev, 0, dataPtr, reserved);
    }

    return 0;
}

int32_t WifiServerCore::construct()
{
    rt_pin_mode(powerPin, PIN_MODE_OUTPUT);
    rt_pin_mode(resetPin, PIN_MODE_OUTPUT);
    HwPowerUp();
    /* 初始化消息队列 */
    rt_mq_init(&mMessage, "rx_mq",
               MSG_POOL,                 /* 存放消息的缓冲区 */
               sizeof(struct rx_msg),    /* 一条消息的最大长度 */
               sizeof(MSG_POOL),         /* 存放消息的缓冲区大小 */
               RT_IPC_FLAG_FIFO);        /* 如果有多个线程等待，按照先来先得到的方法分配消息 */

    mTransferDev = rt_device_find(WIFI_TRANSFER_UART_NAME);
    if (!mTransferDev) {
        rt_kprintf("find %s failed!\n", WIFI_TRANSFER_UART_NAME);
        return RT_ERROR;
    }
    mTransferDev->user_data = this;
    /* 以 DMA 接收及轮询发送方式打开串口设备 */
    rt_device_open(mTransferDev, RT_DEVICE_FLAG_DMA_RX);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(mTransferDev, serverReceiveIrq);

    mReceiveThread = new rtthread::Thread(
        wifiServerEntry, this,
        THREAD_STACK_SIZE,
        THREAD_PRIORITY, THREAD_TIMESLICE,
        "EJD_wifiTransfer");
    if (mReceiveThread != RT_NULL) {
        mReceiveThread->start();
    } else {
        rt_kprintf("Create EJD_consumer failed\n");
        return -1;
    }

    return 0;

}

int32_t WifiServerCore::destruct()
{
    delete mReceiveThread;
    mReceiveThread = NULL;

    rt_device_close(mTransferDev);

    HwPowerDown();
    return 0;
}

