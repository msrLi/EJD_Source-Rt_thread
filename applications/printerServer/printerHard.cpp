/**
 * @file printerHard.c
 * @brief
 * @author  lenovo <xxx@lenovo.com>
 * @version 1.0.0
 * @date 2019-08-13
 */

/* Copyright(C) 2009-2019, Lenovo Inc.
 * All right reserved
 *
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "printerHard.h"

/* 定义硬件相关宏文件 */
/* detail see: https://wenku.baidu.com/view/56897376f242336c1eb95ea3.html */
/* 数据输入口定义 */
#define PRINTER_DATA0           GET_PIN(D, 15)      // GPIO_PIN_15         /*<! 并行数据接口bit0 */
#define PRINTER_DATA1           GET_PIN(D, 14)      // GPIO_PIN_14         /*<! 并行数据接口bit1 */
#define PRINTER_DATA2           GET_PIN(D, 13)      // GPIO_PIN_13         /*<! 并行数据接口bit2 */
#define PRINTER_DATA3           GET_PIN(D, 12)      // GPIO_PIN_12         /*<! 并行数据接口bit3 */
#define PRINTER_DATA4           GET_PIN(D, 11)      // GPIO_PIN_11         /*<! 并行数据接口bit4 */
#define PRINTER_DATA5           GET_PIN(D, 10)      // GPIO_PIN_10         /*<! 并行数据接口bit5 */
#define PRINTER_DATA6           GET_PIN(D, 9)       // GPIO_PIN_9          /*<! 并行数据接口bit6 */
#define PRINTER_DATA7           GET_PIN(D, 8)       // GPIO_PIN_8          /*<! 并行数据接口bit7 */
/* 控制接口定义 */
#define PRINTER_STROBE          GET_PIN(E, 8)       // GPIO_PIN_8          /*<! 主机生成信号: 低脉冲时段上数据有效 */
#define PRINTER_SLCTIN          GET_PIN(E, 7)       // GPIO_PIN_7          /*<! 主机生成信号: 信号为低时使能打印机 */
#define PRINTER_AUTOLF          GET_PIN(B, 2)       // GPIO_PIN_2          /*<! 主机生成信号: 自动换行信号；为低时打印机在回车后换行，为高时只执行回车 */
#define PRINTER_INT             GET_PIN(B, 1)       // GPIO_PIN_1          /*<! 主机生成信号: 初始化打印机; 保持50us以上低电平打印机复位，并清除缓冲区中的数据*/

/* 状态信号接口定义 */
#define PRINTER_ERROR           GET_PIN(E, 10)      // GPIO_PIN_10         /*<! 打印机生成信号: 再打印机出现错误时拉低,表示打印机出错不可用 */
#define PRINTER_SLCT            GET_PIN(E, 11)      //  GPIO_PIN_11         /*<! 打印机输出信号： 打印机正常且准备好数据接收时设置为高 */
#define PRINTER_PE              GET_PIN(E, 12)      //  GPIO_PIN_12         /*<! 打印机输出信号： 打印机缺纸时输出为高 */
#define PRINTER_BUSY            GET_PIN(E, 13)      //  GPIO_PIN_13         /*<! 打印机输出信号： 高电平表示打印机正忙 */
#define PRINTER_ACK             GET_PIN(E, 14)      //  GPIO_PIN_14         /*<! 打印机输出信号：打印机应答信号,当打印机准备好接收 下一个数据时输出5us的负脉冲 */

/* 数据输入输出方向控制接口 */
#define PC_TO_PRINTER_OE        GET_PIN(D, 0)       // GPIO_PIN_0          /*<! PC到打印机数据使能,低电平有效 */
#define PC_TO_PRINTER_DIR       GET_PIN(D, 1)       // GPIO_PIN_1          /*<! PC到打印机数据方向选择信号,低电平时B到A(MCU读取);高电平时数据由A到B(MCU输出） */
// #define PC_TO_PRINTER_D_PORT    GPIOD               /*<! C组端口为GPIOC */

// #define PRINTER_TO_PC_B_OE      GPIO_PIN_11         /*<! ..... */
// #define PRINTER_TO_PC_E_DIR     GPIO_PIN_15         /*<! ..... */

#ifdef BSP_USING_TIM11
#define HWTIMER_DEV_NAME   "timer11"     /* 定时器名称 */
#endif

/**
 * @brief EJD_Shift_Fun
 *          1Byte数据高低bit位互换
 * @param[in] data  输入原始数据
 * @return
 *          转换后的数据
 */
uint8_t PrinterHardware::EJD_Shift_Fun(uint8_t data)
{

    data = (data << 4) | (data >> 4);
    data = ((data << 2) & 0xcc) | ((data >> 2) & 0x33);
    data = ((data << 1) & 0xaa) | ((data >> 1) & 0x55);

    return data;
}

rt_err_t PrinterHardware::printerTimerCallBack(rt_device_t dev, rt_size_t size)
{
    PrinterHardware *thisP = (PrinterHardware*) dev->private_data;

    thisP->mFunCb(thisP->mBuffer);
    return 0;
}

void PrinterHardware::printerStorageData(void *args)
{
    /* 清除定时器值 */
    // priner_time_clean();
    PrinterHardware *thisP = (PrinterHardware*) args;
    uint8_t tmp = 0;

    /* 刚开始接收 记录 ticks值 */
    if (!thisP->mBuffer.start_ticks) {
        tmp = rt_tick_get();
        /* 确保获取到的时间不为 0 */
        thisP->mBuffer.start_ticks = (tmp == 0) ? 1 : tmp;
        /* 开始定时器 */
#ifdef BSP_USING_TIM11
        thisP->EJD_PrinterTimerStart();
#endif
    }

    /* 读取数据 */
    tmp = ((GPIOD->IDR & GPIO_PIN_All) >> 8);
    /* 存储数据 */
    if (thisP->mBuffer.lenght < PARINTER_RECEIVE_DATA_SIZE) {
        thisP->mBuffer.data_buf[thisP->mBuffer.lenght++] =
            thisP->EJD_Shift_Fun(tmp);
    } else {
        /* 传输超字节 */
        // trace_printf("Error: receive printer data longer than %d \n",
        // PARINTER_RECEIVE_DATA_SIZE);
    }
    /* --FIXME-- 测试时使用  通过后立即删除 */
    // trace_printf("data=0x%x, tmp=0x%x\n", EJD_Shift_Fun(tmp), tmp);
    rt_kprintf("data=0x%x, tmp=0x%x\n!\n", thisP->EJD_Shift_Fun(tmp), tmp);
}

int32_t PrinterHardware::EJD_PrinterDateDir(MCU_DIR_E dirs)
{
    rt_base_t mode;

    /* 方向选择芯片使能 */
    rt_pin_write(PC_TO_PRINTER_OE, PIN_LOW);
    /* 设置方向变量 */
    mode = (dirs == MCU_READ) ? PIN_MODE_INPUT_PULLUP : PIN_MODE_OUTPUT;
    /* 配置IO引脚 */
    rt_pin_mode(PRINTER_DATA0, mode);
    rt_pin_mode(PRINTER_DATA1, mode);
    rt_pin_mode(PRINTER_DATA2, mode);
    rt_pin_mode(PRINTER_DATA3, mode);
    rt_pin_mode(PRINTER_DATA4, mode);
    rt_pin_mode(PRINTER_DATA5, mode);
    rt_pin_mode(PRINTER_DATA6, mode);
    rt_pin_mode(PRINTER_DATA7, mode);

    return 0;
}

int32_t PrinterHardware::EJD_PrinterControlDir(MCU_DIR_E dirs)
{
    rt_base_t mode;

    /* 设置方向变量 */
    mode = (dirs == MCU_READ) ? PIN_MODE_INPUT_PULLUP : PIN_MODE_OUTPUT;

    rt_pin_mode(PRINTER_ERROR, mode);
    rt_pin_mode(PRINTER_SLCT, mode);
    rt_pin_mode(PRINTER_PE, mode);
    rt_pin_mode(PRINTER_BUSY, mode);
    rt_pin_mode(PRINTER_ACK, mode);

    return 0;
}

int32_t PrinterHardware::EJD_PrinterExternalSet(void)
{
    rt_pin_mode(PRINTER_SLCTIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(PRINTER_AUTOLF, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(PRINTER_INT, PIN_MODE_INPUT_PULLUP);

    rt_pin_mode(PRINTER_STROBE, PIN_MODE_INPUT_PULLUP);
    /* 绑定中断，下降沿模式，回调函数名为beep_on */
    rt_pin_attach_irq(PRINTER_STROBE, PIN_IRQ_MODE_FALLING, printerStorageData, &mBuffer);
    /* 使能中断 */
    rt_pin_irq_enable(PRINTER_STROBE, PIN_IRQ_ENABLE);

    return 0;
}

int32_t PrinterHardware::EJD_PrinterTimerStart(void)
{
    rt_hwtimerval_t timeout_s;      /* 定时器超时值 */

    /* 设置定时器超时值为5s并启动定时器 */
    timeout_s.sec = 0;      /* 秒 */
    timeout_s.usec = 500 * 1000;     /* 微秒 */

    if (rt_device_write(mhw_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s)) {
        rt_kprintf("set timeout value failed\n");
        return RT_ERROR;
    }
    return RT_EOK;
}

int32_t PrinterHardware::EJD_TimeInit(void)
{
#ifdef BSP_USING_TIM11
    rt_err_t ret = RT_EOK;
    rt_hwtimer_mode_t mode;         /* 定时器模式 */

    /* 查找定时器设备 */
    mhw_dev = rt_device_find(HWTIMER_DEV_NAME);
    if (mhw_dev == RT_NULL) {
        rt_kprintf("hwtimer sample run failed! can't find %s device!\n", HWTIMER_DEV_NAME);
        return RT_ERROR;
    }
    mhw_dev->private_data = this;
    /* 以读写方式打开设备 */
    ret = rt_device_open(mhw_dev, RT_DEVICE_OFLAG_RDWR);
    if (ret != RT_EOK) {
        rt_kprintf("open %s device failed!\n", HWTIMER_DEV_NAME);
        return ret;
    }

    /* 设置超时回调函数 */
    rt_device_set_rx_indicate(mhw_dev, printerTimerCallBack);


    /* 设置模式为周期性定时器 */
    mode = HWTIMER_MODE_ONESHOT;
    ret = rt_device_control(mhw_dev, HWTIMER_CTRL_MODE_SET, &mode);
    if (ret != RT_EOK) {
        rt_kprintf("set mode failed! ret is :%d\n", ret);
        return ret;
    }
#endif
}

int32_t PrinterHardware::EJD_PinInit(void)
{
    rt_pin_mode(PC_TO_PRINTER_OE, PIN_MODE_OUTPUT);
    rt_pin_mode(PC_TO_PRINTER_DIR, PIN_MODE_OUTPUT);
    rt_pin_write(PC_TO_PRINTER_OE, PIN_LOW);
    rt_pin_write(PC_TO_PRINTER_DIR, PIN_LOW);

    EJD_PrinterDateDir(MCU_READ);
    EJD_PrinterControlDir(MCU_READ);

    EJD_TimeInit();

    EJD_PrinterExternalSet();

    rt_pin_mode(PRINTER_SLCT, PIN_MODE_OUTPUT);
    rt_pin_mode(PRINTER_PE, PIN_MODE_OUTPUT);
    rt_pin_mode(PRINTER_BUSY, PIN_MODE_OUTPUT);
    rt_pin_mode(PRINTER_ACK, PIN_MODE_OUTPUT);
    rt_pin_write(PRINTER_SLCT, PIN_HIGH);
    rt_pin_write(PRINTER_PE, PIN_HIGH);
    rt_pin_write(PRINTER_BUSY, PIN_HIGH);
    rt_pin_write(PRINTER_ACK, PIN_HIGH);

    return 0;
}

int32_t PrinterHardware::EJD_PinUnint(void)
{
    if (mhw_dev) {
        rt_device_close(mhw_dev);
        mhw_dev = RT_NULL;
    }
    return 0;
}

PrinterHardware::PrinterHardware(std::function<int32_t (PRINTER_BUF_T &date)> cb):
    mFunCb(cb),
    mhw_dev(RT_NULL)
{

}

PrinterHardware::~PrinterHardware()
{

}

int32_t PrinterHardware::construct()
{
    rt_memset(&mBuffer, 0, sizeof(mBuffer));
    return EJD_PinInit();
}

int32_t PrinterHardware::destruct()
{
    return EJD_PinUnint();
    return 0;
}

