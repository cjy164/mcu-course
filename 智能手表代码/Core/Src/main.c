/*
 * main.c - 基于 STM32F103C8T6 的智能手表主程序
 *
 * 功能：
 *   1. FreeRTOS 多任务管理
 *   2. OLED 显示时间页面 + 传感器数据页面
 *   3. 按键切换页面
 *   4. MPU6050 加速度/温度读取
 *   5. 软件 RTC 计时
 *
 * CubeMX 配置说明:
 *   - SYS: Timebase Source = TIM1 (避免与 FreeRTOS 冲突)
 *   - I2C1: PB6=SCL, PB7=SDA
 *   - USART1: PA9=TX, PA10=RX (用于 HC-05 蓝牙)
 *   - GPIO: PA0 外部中断输入 (按键)
 *   - FreeRTOS: CMSIS_V2 或 默认
 *   - RCC: HSE 8MHz Crystal
 *
 * 引脚分配:
 *   PB6 - I2C1_SCL → OLED SCL + MPU6050 SCL
 *   PB7 - I2C1_SDA → OLED SDA + MPU6050 SDA
 *   PA0 - 按键输入 (外部中断, 下降沿)
 *   PA2 - 马达驱动 GPIO (可选)
 */

/* Includes */
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "i2c.h"
#include "oled.h"
#include "mpu6050.h"
#include "soft_timer.h"
#include "ui.h"
#include <stdio.h>

/* ========== 任务句柄 ========== */
TaskHandle_t oled_task_handle   = NULL;
TaskHandle_t sensor_task_handle = NULL;

/* ========== 传感器数据 ========== */
static MPU6050_Data_t mpu_data;

/* ========== 按键防抖 ========== */
static volatile uint8_t button_pressed = 0;
#define DEBOUNCE_MS                     50

/* ========== 函数声明 ========== */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);

/* ========== FreeRTOS 任务 ========== */

/*
 * OLED 显示任务
 * 每 500ms 刷新一次屏幕
 */
void OLED_Task(void *argument)
{
    (void)argument;

    for (;;) {
        UI_Refresh();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/*
 * 传感器采集任务
 * 每 200ms 读取一次 MPU6050
 */
void Sensor_Task(void *argument)
{
    (void)argument;

    for (;;) {
        if (MPU6050_ReadAll(&mpu_data) == 0) {
            UI_UpdateSensorData(
                mpu_data.accel_x,
                mpu_data.accel_y,
                mpu_data.accel_z,
                mpu_data.temp_c
            );
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/*
 * 按键处理任务
 * 等待按键信号量，切换 UI 页面
 */
void Button_Task(void *argument)
{
    (void)argument;

    for (;;) {
        // 等待按键按下标志 (由中断设置)
        if (button_pressed) {
            button_pressed = 0;

            // 防抖延时
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));

            // 再次检查按键是否确实按下
            if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
                // 等待按键释放
                while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }

                // 切换到下一页
                UI_NextPage();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* ========== 主函数 ========== */
int main(void)
{
    /* HAL 库初始化 */
    HAL_Init();

    /* 系统时钟配置: 72MHz (HSE 8MHz x9 PLL) */
    SystemClock_Config();

    /* 外设初始化 */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM2_Init();     // 启动 TIM2 1秒定时器 (软件RTC)

    /* 芯片自检: 板载 LED 闪烁两次 (PC13 低电平点亮) */
    for (int i = 0; i < 2; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  // 亮
        HAL_Delay(200);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    // 灭
        if (i < 1) HAL_Delay(200);
    }

    /* OLED 初始化 */
    if (OLED_Init()) {
        // OLED 成功: 板载 LED 闪一下
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        HAL_Delay(200);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

        OLED_ClearBuffer();
        OLED_PutString(16, 1, "OLED OK");
        OLED_UpdateScreen();
    } else {
        // 失败: 板载 LED 快速闪烁 20 次
        for (int i = 0; i < 20; i++) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            HAL_Delay(100);
        }
        // 闪完后常亮 (I2C 错误指示)
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    }

    /* 软件计时器初始化 */
    SoftTimer_Init();

    /* MPU6050 初始化 */
    if (MPU6050_Init() == 0) {
        OLED_PutString(20, 4, "MPU6050 OK ");
    } else {
        OLED_PutString(20, 4, "MPU6050 ERR");
    }
    OLED_UpdateScreen();
    HAL_Delay(500);

    /* UI 初始化 */
    UI_Init();

    /* 创建 FreeRTOS 任务 */
    xTaskCreate(OLED_Task,   "OLED",   configTASK_STACK_DEPTH_OLED,
                NULL, OLED_TASK_PRIORITY, &oled_task_handle);
    xTaskCreate(Sensor_Task, "Sensor", configTASK_STACK_DEPTH_SENSOR,
                NULL, SENSOR_TASK_PRIORITY, &sensor_task_handle);
    xTaskCreate(Button_Task, "Button", configTASK_STACK_DEPTH_BUTTON,
                NULL, BUTTON_TASK_PRIORITY, NULL);

    /* 启动调度器 */
    vTaskStartScheduler();

    /* 调度器启动后不会到达这里 */
    for (;;) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);  // 错误指示灯
        HAL_Delay(500);
    }
}

/* ========== SysTick 1秒回调 ========== */

/*
 * 注意: 当 FreeRTOS 启用时，SysTick_Handler 由 FreeRTOS 管理。
 * 需要在 FreeRTOSConfig.h 中配置:
 *   #define xPortSysTickHandler  SysTick_Handler
 *
 * 软件计时器的秒中断通过 FreeRTOS 的定时器或单独的 TIM 实现。
 *
 * 这里在 HAL_SYSTICK_Callback 中处理秒钟增加。
 * 或者可以使用单独的 TIM2 定时器产生 1 秒中断。
 */

/* 使用 TIM2 作为 1 秒定时器，避免与 FreeRTOS 的 SysTick 冲突 */
TIM_HandleTypeDef htim2;

static void MX_TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance               = TIM2;
    htim2.Init.Prescaler         = 7200 - 1;     // 72MHz / 7200 = 10kHz
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = 10000 - 1;     // 10kHz / 10000 = 1Hz
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    __HAL_RCC_TIM2_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    HAL_TIM_Base_Init(&htim2);
    HAL_TIM_Base_Start_IT(&htim2);
}

/* TIM2 中断回调 - 每秒调用 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        SoftTimer_Tick();       // 软件计时 +1s
    }
}

/* ========== 按键外部中断 ========== */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0) {
        button_pressed = 1;     // 设置按键标志
    }
}

/* ========== 硬件初始化 ========== */

I2C_HandleTypeDef hi2c1;   // I2C 句柄 (CubeMX 生成于此)

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* HSE 8MHz, PLL 9倍频 -> 72MHz */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;   // APB1: 36MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;   // APB2: 72MHz
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* PA0: 按键输入 (外部中断) */
    GPIO_InitStruct.Pin   = GPIO_PIN_0;
    GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA2: 马达输出 (推挽输出) */
    GPIO_InitStruct.Pin   = GPIO_PIN_2;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PC13: 板载 LED (低电平点亮) */
    GPIO_InitStruct.Pin   = GPIO_PIN_13;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);  // 初始熄灭

    /* 中断优先级配置 */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void MX_I2C1_Init(void)
{
    hi2c1.Instance             = I2C1;
    hi2c1.Init.ClockSpeed      = 100000;           // 100kHz 标准模式
    hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1     = 0;
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

/* ========== FreeRTOS 钩子 ========== */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    for (;;);
}

void vApplicationMallocFailedHook(void)
{
    for (;;);
}

/* ========== HAL 断言 ========== */
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif
