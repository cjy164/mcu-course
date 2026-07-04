/*
 * i2c.h - I2C 总线驱动封装
 * 基于 STM32 HAL 库，用于 OLED + MPU6050 通信
 */

#ifndef __I2C_H
#define __I2C_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* I2C 句柄 - 由 CubeMX 生成 */
extern I2C_HandleTypeDef hi2c1;

/* I2C 超时时间 (ms) */
#define I2C_TIMEOUT                    100

/*
 * I2C 写寄存器
 * dev_addr: 设备地址 (7位左移1位)
 * reg    : 寄存器地址
 * data   : 要写的数据
 * 返回: HAL_OK=成功
 */
HAL_StatusTypeDef I2C_WriteRegister(uint8_t dev_addr, uint8_t reg, uint8_t data);

/*
 * I2C 读寄存器
 * dev_addr: 设备地址 (7位左移1位)
 * reg    : 寄存器地址
 * data   : 读出数据的指针
 * 返回: HAL_OK=成功
 */
HAL_StatusTypeDef I2C_ReadRegister(uint8_t dev_addr, uint8_t reg, uint8_t *data);

/*
 * I2C 连续读多字节
 * dev_addr: 设备地址
 * reg    : 起始寄存器
 * pData  : 数据缓冲区
 * len    : 读取长度
 */
HAL_StatusTypeDef I2C_ReadMultiBytes(uint8_t dev_addr, uint8_t reg,
                                     uint8_t *pData, uint16_t len);

/*
 * I2C 连续写多字节
 * dev_addr: 设备地址
 * reg    : 起始寄存器
 * pData  : 数据缓冲区
 * len    : 写入长度
 */
HAL_StatusTypeDef I2C_WriteMultiBytes(uint8_t dev_addr, uint8_t reg,
                                      uint8_t *pData, uint16_t len);

#endif /* __I2C_H */
