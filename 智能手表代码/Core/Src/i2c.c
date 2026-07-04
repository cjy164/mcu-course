/*
 * i2c.c - I2C 总线驱动封装
 */

#include "i2c.h"

/* I2C 句柄 - 由 CubeMX 在 main.c 中生成 */
extern I2C_HandleTypeDef hi2c1;

HAL_StatusTypeDef I2C_WriteRegister(uint8_t dev_addr, uint8_t reg, uint8_t data)
{
    return HAL_I2C_Mem_Write(&hi2c1, dev_addr, reg,
                             I2C_MEMADD_SIZE_8BIT, &data, 1, I2C_TIMEOUT);
}

HAL_StatusTypeDef I2C_ReadRegister(uint8_t dev_addr, uint8_t reg, uint8_t *data)
{
    return HAL_I2C_Mem_Read(&hi2c1, dev_addr, reg,
                            I2C_MEMADD_SIZE_8BIT, data, 1, I2C_TIMEOUT);
}

HAL_StatusTypeDef I2C_ReadMultiBytes(uint8_t dev_addr, uint8_t reg,
                                     uint8_t *pData, uint16_t len)
{
    return HAL_I2C_Mem_Read(&hi2c1, dev_addr, reg,
                            I2C_MEMADD_SIZE_8BIT, pData, len, I2C_TIMEOUT);
}

HAL_StatusTypeDef I2C_WriteMultiBytes(uint8_t dev_addr, uint8_t reg,
                                      uint8_t *pData, uint16_t len)
{
    return HAL_I2C_Mem_Write(&hi2c1, dev_addr, reg,
                             I2C_MEMADD_SIZE_8BIT, pData, len, I2C_TIMEOUT);
}
