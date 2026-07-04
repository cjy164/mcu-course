/*
 * ui.c - OLED 用户界面
 * 包含时间和传感器两个页面，通过按键切换
 */

#include "ui.h"
#include "oled.h"
#include "soft_timer.h"
#include <stdio.h>
#include <string.h>

/* 当前页面 */
static UI_Page_t current_page = UI_PAGE_TIME;

/* 缓存的传感器数据 */
static volatile int16_t s_ax, s_ay, s_az;
static volatile float    s_temp;

void UI_Init(void)
{
    current_page = UI_PAGE_TIME;
    s_ax = s_ay = s_az = 0;
    s_temp = 25.0f;
}

void UI_SwitchPage(UI_Page_t page)
{
    if (page < UI_PAGE_COUNT) {
        current_page = page;
        OLED_ClearBuffer();
    }
}

UI_Page_t UI_GetCurrentPage(void)
{
    return current_page;
}

void UI_NextPage(void)
{
    current_page = (UI_Page_t)((current_page + 1) % UI_PAGE_COUNT);
    OLED_ClearBuffer();
}

void UI_UpdateSensorData(int16_t ax, int16_t ay, int16_t az, float temp)
{
    s_ax   = ax;
    s_ay   = ay;
    s_az   = az;
    s_temp = temp;
}

/* ========== 绘制时间页面 ========== */
static void draw_time_page(void)
{
    char buf[20];

    /* 第0行: 标题 */
    OLED_PutString(0, 0, "Smart Watch");

    /* 第2行: 时间 (大号字方案: 用8x16显示, 这里用两行6x8模拟大字效果) */
    // 小时:分钟
    snprintf(buf, sizeof(buf), "%02u:%02u",
             g_sys_time.hour, g_sys_time.minute);
    OLED_PutString(30, 2, buf);     // 居中偏上

    /* 第3行: 秒 */
    snprintf(buf, sizeof(buf), "%02u", g_sys_time.second);
    OLED_PutString(55, 3, buf);

    /* 第5行: 日期 */
    SoftTimer_GetDateStr(buf);
    OLED_PutString(20, 5, buf);

    /* 第6行: 星期 */
    // 简单显示年月标志
    OLED_PutString(20, 6, "Press KEY ->");
}

/* ========== 绘制传感器页面 ========== */
static void draw_sensor_page(void)
{
    char buf[20];

    /* 第0行: 标题 */
    OLED_PutString(0, 0, "Sensor Data");

    /* 第1行: 加速度 X */
    snprintf(buf, sizeof(buf), "AX:%6d", s_ax);
    OLED_PutString(0, 2, buf);

    /* 第2行: 加速度 Y */
    snprintf(buf, sizeof(buf), "AY:%6d", s_ay);
    OLED_PutString(0, 3, buf);

    /* 第3行: 加速度 Z */
    snprintf(buf, sizeof(buf), "AZ:%6d", s_az);
    OLED_PutString(0, 4, buf);

    /* 第5行: 温度 (×10转为整数显示一位小数，避免nano.specs不支持%f) */
    int16_t temp_int = (int16_t)(s_temp * 10.0f);
    snprintf(buf, sizeof(buf), "TEMP:%3d.%d C",
             temp_int / 10, (temp_int < 0 ? -temp_int : temp_int) % 10);
    OLED_PutString(0, 6, buf);

    /* 第7行: 提示 */
    OLED_PutString(0, 7, "PRESS KEY");
}

void UI_Refresh(void)
{
    switch (current_page) {
    case UI_PAGE_TIME:
        draw_time_page();
        break;
    case UI_PAGE_SENSOR:
        draw_sensor_page();
        break;
    default:
        break;
    }
    OLED_UpdateScreen();
}
