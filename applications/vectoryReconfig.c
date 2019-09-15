/**
 * @file vectoryReconfig.c
 * @brief
 * @author  binsonli <binson666@163.com>
 * @version 1.0.0
 * @date 2019-09-08
 */
/* Copyright(C) 2009-2017, Persion Inc.
 * All right reserved
 *
 */


#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "fal.h"

/**
 * Function    ota_app_vtor_reconfig
 * Description Set Vector Table base location to the start addr of app(RT_APP_PART_ADDR).
 */
static int ota_app_vtor_reconfig(void)
{
#define NVIC_VTOR_MASK   0x3FFFFF80
    /* Set the Vector Table base location by user application firmware definition */
    SCB->VTOR = RT_APP_PART_ADDR & NVIC_VTOR_MASK;

    return 0;
}

INIT_BOARD_EXPORT(ota_app_vtor_reconfig);
