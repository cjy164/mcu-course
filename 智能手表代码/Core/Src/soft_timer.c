/*
 * soft_timer.c - 软件计时器 (基于 SysTick)
 */

#include "soft_timer.h"
#include <stdio.h>

/* 全局系统时间 */
volatile RTC_Time_t g_sys_time = {
    .year   = 2026,
    .month  = 7,
    .day    = 1,
    .hour   = 0,
    .minute = 0,
    .second = 0
};

/* 每个月的天数 */
static const uint8_t days_in_month[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* 判断闰年 */
static uint8_t is_leap_year(uint16_t year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void SoftTimer_Init(void)
{
    // 设置初始时间
    g_sys_time.year   = 2026;
    g_sys_time.month  = 7;
    g_sys_time.day    = 1;
    g_sys_time.hour   = 0;
    g_sys_time.minute = 0;
    g_sys_time.second = 0;
}

void SoftTimer_SetTime(uint16_t year, uint8_t month, uint8_t day,
                       uint8_t hour, uint8_t minute, uint8_t second)
{
    g_sys_time.year   = year;
    g_sys_time.month  = month;
    g_sys_time.day    = day;
    g_sys_time.hour   = hour;
    g_sys_time.minute = minute;
    g_sys_time.second = second;
}

void SoftTimer_Tick(void)
{
    /* 每秒递增，处理日期进位 */
    g_sys_time.second++;

    if (g_sys_time.second < 60) return;
    g_sys_time.second = 0;
    g_sys_time.minute++;

    if (g_sys_time.minute < 60) return;
    g_sys_time.minute = 0;
    g_sys_time.hour++;

    if (g_sys_time.hour < 24) return;
    g_sys_time.hour = 0;

    /* 日进位 */
    uint8_t month_days = days_in_month[g_sys_time.month - 1];
    if (g_sys_time.month == 2 && is_leap_year(g_sys_time.year))
        month_days = 29;

    g_sys_time.day++;
    if (g_sys_time.day <= month_days) return;
    g_sys_time.day = 1;
    g_sys_time.month++;

    if (g_sys_time.month <= 12) return;
    g_sys_time.month = 1;
    g_sys_time.year++;
}

void SoftTimer_GetTimeStr(char *buf)
{
    sprintf(buf, "%04u-%02u-%02u %02u:%02u:%02u",
            g_sys_time.year, g_sys_time.month, g_sys_time.day,
            g_sys_time.hour, g_sys_time.minute, g_sys_time.second);
}

void SoftTimer_GetTimeShort(char *buf)
{
    sprintf(buf, "%02u:%02u:%02u",
            g_sys_time.hour, g_sys_time.minute, g_sys_time.second);
}

void SoftTimer_GetDateStr(char *buf)
{
    sprintf(buf, "%04u-%02u-%02u",
            g_sys_time.year, g_sys_time.month, g_sys_time.day);
}
