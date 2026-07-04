/*
 * mpu6050.h - MPU6050 六轴陀螺仪+加速度计驱动
 * I2C 接口
 */

#ifndef __MPU6050_H
#define __MPU6050_H

#include <stdint.h>

/* I2C 地址 (7位左移1位) */
#define MPU6050_ADDR                    (0x68 << 1)     // AD0=GND
#define MPU6050_ADDR_ALT                (0x69 << 1)     // AD0=VCC

/* ---- 寄存器地址 ---- */
#define MPU6050_WHO_AM_I                0x75
#define MPU6050_SMPLRT_DIV              0x19
#define MPU6050_CONFIG                  0x1A
#define MPU6050_GYRO_CONFIG             0x1B
#define MPU6050_ACCEL_CONFIG            0x1C
#define MPU6050_PWR_MGMT_1              0x6B
#define MPU6050_PWR_MGMT_2              0x6C

/* 数据输出寄存器 (高位在前) */
#define MPU6050_ACCEL_XOUT_H            0x3B
#define MPU6050_ACCEL_XOUT_L            0x3C
#define MPU6050_ACCEL_YOUT_H            0x3D
#define MPU6050_ACCEL_YOUT_L            0x3E
#define MPU6050_ACCEL_ZOUT_H            0x3F
#define MPU6050_ACCEL_ZOUT_L            0x40
#define MPU6050_TEMP_OUT_H              0x41
#define MPU6050_TEMP_OUT_L              0x42
#define MPU6050_GYRO_XOUT_H             0x43
#define MPU6050_GYRO_XOUT_L             0x44
#define MPU6050_GYRO_YOUT_H             0x45
#define MPU6050_GYRO_YOUT_L             0x46
#define MPU6050_GYRO_ZOUT_H             0x47
#define MPU6050_GYRO_ZOUT_L             0x48

/* 量程配置 */
#define MPU6050_ACCEL_RANGE_2G          0x00
#define MPU6050_ACCEL_RANGE_4G          0x08
#define MPU6050_ACCEL_RANGE_8G          0x10
#define MPU6050_ACCEL_RANGE_16G         0x18

#define MPU6050_GYRO_RANGE_250          0x00
#define MPU6050_GYRO_RANGE_500          0x08
#define MPU6050_GYRO_RANGE_1000         0x10
#define MPU6050_GYRO_RANGE_2000         0x18

/* MPU6050 数据结构 */
typedef struct {
    int16_t accel_x;        // 加速度 X 轴原始值
    int16_t accel_y;        // 加速度 Y 轴原始值
    int16_t accel_z;        // 加速度 Z 轴原始值
    int16_t gyro_x;         // 陀螺仪 X 轴原始值
    int16_t gyro_y;         // 陀螺仪 Y 轴原始值
    int16_t gyro_z;         // 陀螺仪 Z 轴原始值
    int16_t temperature;    // 温度原始值
    float   accel_x_g;      // 加速度 X (g)
    float   accel_y_g;      // 加速度 Y (g)
    float   accel_z_g;      // 加速度 Z (g)
    float   temp_c;         // 温度 (℃)
} MPU6050_Data_t;

/*
 * MPU6050 初始化
 * 唤醒设备，配置加速度 ±2g，陀螺仪 ±250°/s
 * 返回: 0=成功, -1=设备未找到
 */
int MPU6050_Init(void);

/*
 * 读取所有传感器数据
 * 返回: 0=成功
 */
int MPU6050_ReadAll(MPU6050_Data_t *data);

/*
 * 检查设备是否存在
 * 返回: 1=存在, 0=不存在
 */
uint8_t MPU6050_IsConnected(void);

#endif /* __MPU6050_H */
