/*
 * mpu6050.c - MPU6050 六轴传感器驱动
 */

#include "mpu6050.h"
#include "i2c.h"

/* 加速度灵敏度 (LSB/g) - ±2g 量程 */
#define ACCEL_SENSITIVITY               16384.0f

/* 温度公式: Temp(℃) = (TEMP_OUT / 340) + 36.53 */
#define TEMP_SENSITIVITY                340.0f
#define TEMP_OFFSET                     36.53f

int MPU6050_Init(void)
{
    uint8_t whoami;

    // 检查设备是否存在
    if (I2C_ReadRegister(MPU6050_ADDR, MPU6050_WHO_AM_I, &whoami) != HAL_OK)
        return -1;

    // WHO_AM_I 应返回 0x68 (对于 MPU6050)
    if (whoami != 0x68 && whoami != 0x98)
        return -1;

    // 复位设备
    I2C_WriteRegister(MPU6050_ADDR, MPU6050_PWR_MGMT_1, 0x80);
    HAL_Delay(100);

    // 唤醒设备 - 选择时钟源为 X 轴陀螺仪 PLL (稳定性更好)
    I2C_WriteRegister(MPU6050_ADDR, MPU6050_PWR_MGMT_1, 0x01);
    HAL_Delay(10);

    // 配置加速度 ±2g
    I2C_WriteRegister(MPU6050_ADDR, MPU6050_ACCEL_CONFIG, MPU6050_ACCEL_RANGE_2G);
    HAL_Delay(10);

    // 配置陀螺仪 ±250°/s
    I2C_WriteRegister(MPU6050_ADDR, MPU6050_GYRO_CONFIG, MPU6050_GYRO_RANGE_250);
    HAL_Delay(10);

    // 配置低通滤波 (带宽 44Hz)
    I2C_WriteRegister(MPU6050_ADDR, MPU6050_CONFIG, 0x03);
    HAL_Delay(10);

    // 采样率分频: 50Hz
    I2C_WriteRegister(MPU6050_ADDR, MPU6050_SMPLRT_DIV, 0x13);
    HAL_Delay(10);

    return 0;
}

uint8_t MPU6050_IsConnected(void)
{
    uint8_t whoami;
    if (I2C_ReadRegister(MPU6050_ADDR, MPU6050_WHO_AM_I, &whoami) != HAL_OK)
        return 0;
    return (whoami == 0x68 || whoami == 0x98) ? 1 : 0;
}

int MPU6050_ReadAll(MPU6050_Data_t *data)
{
    uint8_t raw[14];

    // 从 ACCEL_XOUT_H 连续读取 14 字节
    if (I2C_ReadMultiBytes(MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, raw, 14)
        != HAL_OK) {
        return -1;
    }

    // 解析加速度 (高位在前)
    data->accel_x = (int16_t)((raw[0]  << 8) | raw[1]);
    data->accel_y = (int16_t)((raw[2]  << 8) | raw[3]);
    data->accel_z = (int16_t)((raw[4]  << 8) | raw[5]);

    // 解析温度
    data->temperature = (int16_t)((raw[6] << 8) | raw[7]);

    // 解析陀螺仪
    data->gyro_x = (int16_t)((raw[8]  << 8) | raw[9]);
    data->gyro_y = (int16_t)((raw[10] << 8) | raw[11]);
    data->gyro_z = (int16_t)((raw[12] << 8) | raw[13]);

    // 转换为物理单位
    data->accel_x_g = (float)data->accel_x / ACCEL_SENSITIVITY;
    data->accel_y_g = (float)data->accel_y / ACCEL_SENSITIVITY;
    data->accel_z_g = (float)data->accel_z / ACCEL_SENSITIVITY;

    data->temp_c = (float)data->temperature / TEMP_SENSITIVITY + TEMP_OFFSET;

    return 0;
}
