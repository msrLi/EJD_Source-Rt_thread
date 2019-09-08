/**
 * @file crc32.h
 * @brief CRC32 计算头文件
 * @author  <binsonli666@icloud.com>
 * @version 1.0.0
 * @date 2018-05-29
 */

#ifndef __CRC32_H__
#define __CRC32_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>

/* --------------------------------------------------------------------------*/
/**
 * @brief crc32
 *          CRC计算
 * @param[in] crc       crc值
 * @param[in] buf       数据缓冲地址
 * @param[in] length    数据长度
 *
 * @return
 *      crc32计算结果
 */
/* --------------------------------------------------------------------------*/
uint32_t crc32(uint32_t crc,const uint8_t *buf, uint32_t length);

/**
 * @brief Shift_Fun
 *          1Byte数据高低bit位互换
 * @param[in] data  输入原始数据
 * @return
 *          转换后的数据
 */
uint8_t shiftFun(uint8_t data);
#ifdef __cplusplus
}
#endif

#endif // __CRC32_H__
