/*
 * ui.h - OLED 用户界面
 * 页面管理：时间页面 + 传感器数据页面
 */

#ifndef __UI_H
#define __UI_H

#include <stdint.h>

/* 页面枚举 */
typedef enum {
    UI_PAGE_TIME = 0,       // 时间显示页面
    UI_PAGE_SENSOR,         // 传感器数据页面
    UI_PAGE_COUNT
} UI_Page_t;

/*
 * UI 初始化
 */
void UI_Init(void);

/*
 * 切换到指定页面
 */
void UI_SwitchPage(UI_Page_t page);

/*
 * 获取当前页面
 */
UI_Page_t UI_GetCurrentPage(void);

/*
 * 下一页
 */
void UI_NextPage(void);

/*
 * 刷新当前页面内容到 OLED
 * 由 OLED 显示任务调用
 */
void UI_Refresh(void);

/*
 * 更新传感器数据引用
 */
void UI_UpdateSensorData(int16_t ax, int16_t ay, int16_t az, float temp);

#endif /* __UI_H */
