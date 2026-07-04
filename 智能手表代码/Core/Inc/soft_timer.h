/*
 * soft_timer.h - 软件计时器 (基于 SysTick)
 * 用于实现手表时间显示，无需外部 RTC 芯片
 */

#ifndef __SOFT_TIMER_H
#define __SOFT_TIMER_H

#include <stdint.h>

/* 时间结构体 */
typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
} RTC_Time_t;

/* 全局时间 - 由 SysTick 中断更新 */
extern volatile RTC_Time_t g_sys_time;

/*
 * 初始化软件计时器
 * 设置初始时间: 2026-07-01 00:00:00
 * 注册 SysTick 1秒回调
 */
void SoftTimer_Init(void);

/*
 * 设置时间
 */
void SoftTimer_SetTime(uint16_t year, uint8_t month, uint8_t day,
                       uint8_t hour, uint8_t minute, uint8_t second);

/*
 * SysTick 1秒中断回调 (由 HAL 或裸机调用)
 */
void SoftTimer_Tick(void);

/*
 * 获取当前时间字符串
 * buf: 至少 20 字节缓冲区
 * 格式: "2026-07-01 12:30:45"
 */
void SoftTimer_GetTimeStr(char *buf);

/*
 * 获取时间字符串 (简短)
 * buf: 至少 10 字节
 * 格式: "12:30:45"
 */
void SoftTimer_GetTimeShort(char *buf);

/*
 * 获取日期字符串
 * buf: 至少 12 字节
 * 格式: "2026-07-01"
 */
void SoftTimer_GetDateStr(char *buf);

#endif /* __SOFT_TIMER_H */
