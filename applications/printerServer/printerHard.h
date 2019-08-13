/**
 * @file printerHard.h
 * @brief printer hard ware interface
 * @author  lenovo <xxx@lenovo.com>
 * @version 1.0.0
 * @date 2019-08-13
 */

/* Copyright(C) 2009-2019, Lenovo Inc.
 * All right reserved
 *
 */

#ifndef _PRINTER_HARDWARE_H_
#define _PRINTER_HARDWARE_H_

#include <functional>

/* 设置最大接收小票数据 buffer 数量 */
#define PARINTER_RECEIVE_DATA_SIZE          (2048)

typedef struct PRINTER_BUF_TAG {
    uint32_t start_ticks;           /*<! 接收开始时间 */
    uint32_t lenght;                /*<! 数据长度 */
    uint32_t transer_end;           /*<! 传输结束 */
    uint8_t data_buf[PARINTER_RECEIVE_DATA_SIZE];         /*<! 数据缓冲 */
} PRINTER_BUF_T;

typedef enum {
    MCU_READ = 0, MCU_WRITE
} MCU_DIR_E;


class PrinterHardware
{
public:
    PrinterHardware(std::function<int32_t (PRINTER_BUF_T &date)> cb);
    virtual ~PrinterHardware();

public:
    int32_t construct();
    int32_t destruct();

private:
    int32_t EJD_PinInit(void);
    int32_t EJD_PinUnint(void);
    int32_t EJD_TimeInit(void);
    int32_t EJD_PrinterTimerStart(void);
    int32_t EJD_PrinterExternalSet(void);
    int32_t EJD_PrinterControlDir(MCU_DIR_E dirs);
    int32_t EJD_PrinterDateDir(MCU_DIR_E dirs);
    uint8_t EJD_Shift_Fun(uint8_t data);
    static void printerStorageData(void *args);
    static rt_err_t printerTimerCallBack(rt_device_t dev, rt_size_t size);

private:
    PrinterHardware() = delete;

private:
    std::function<int32_t (PRINTER_BUF_T &date)> mFunCb;
    rt_device_t   mhw_dev;
    PRINTER_BUF_T mBuffer;

};


#endif
