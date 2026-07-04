/*
 * oled.h - 0.96/1.3寸 SSD1306 OLED I2C 驱动
 * 分辨率: 128x64
 * I2C地址: 0x3C (SA0=GND) 或 0x3D (SA0=VCC)
 */

#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>

/* OLED 尺寸 */
#define OLED_WIDTH                      128
#define OLED_HEIGHT                     64
#define OLED_PAGES                      (OLED_HEIGHT / 8)

/* I2C 地址 (7位地址左移1位) */
#define OLED_I2C_ADDR                   (0x3C << 1)
#define OLED_I2C_ADDR_ALT               (0x3D << 1)

/* ---- 基本命令 ---- */
#define OLED_CMD_SET_CONTRAST           0x81
#define OLED_CMD_DISPLAY_ON             0xAF
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_SET_NORM_DISP          0xA6
#define OLED_CMD_SET_INV_DISP           0xA7
#define OLED_CMD_SET_MEM_MODE           0x20
#define OLED_CMD_SET_COL_ADDR           0x21
#define OLED_CMD_SET_PAGE_ADDR          0x22
#define OLED_CMD_DISPLAY_START_LINE     0x40
#define OLED_CMD_SEG_REMAP              0xA1
#define OLED_CMD_COM_SCAN_DEC           0xC8
#define OLED_CMD_SET_MUX_RATIO          0xA8
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3
#define OLED_CMD_SET_COM_PINS           0xDA
#define OLED_CMD_SET_CLK_DIV            0xD5
#define OLED_CMD_SET_PRECHARGE          0xD9
#define OLED_CMD_SET_VCOM_DESELECT      0xDB
#define OLED_CMD_CHARGE_PUMP            0x8D
#define OLED_CMD_ENTIRE_DISP_ON         0xA4

/* ---- 外部函数 ---- */

/*
 * OLED 初始化
 * 自动检测 I2C 地址 (0x3C/0x3D)
 * 支持 SSD1306 和 SH1106 驱动芯片
 * 返回: 1=成功, 0=失败
 */
uint8_t OLED_Init(void);

/*
 * 清空显存 (不刷新屏幕)
 */
void OLED_ClearBuffer(void);

/*
 * 将显存内容刷新到 OLED
 */
void OLED_UpdateScreen(void);

/*
 * 画一个像素点
 * x: 0-127, y: 0-63
 * color: 1=亮, 0=灭
 */
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);

/*
 * 显示一个 ASCII 字符 (6x8 字体)
 * x: 列, y: 页 (0-7)
 */
void OLED_PutChar(uint8_t x, uint8_t y, char ch);

/*
 * 显示字符串
 * x: 列, y: 页 (0-7)
 */
void OLED_PutString(uint8_t x, uint8_t y, const char *str);

/*
 * 显示数字 (十进制)
 */
void OLED_PutNumber(uint8_t x, uint8_t y, uint32_t num, uint8_t len);

/*
 * 画水平线
 */
void OLED_DrawHLine(uint8_t x0, uint8_t x1, uint8_t y);

/*
 * 画垂直线
 */
void OLED_DrawVLine(uint8_t x, uint8_t y0, uint8_t y1);

/*
 * 画矩形框
 */
void OLED_DrawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

/*
 * 全屏填充
 */
void OLED_Fill(uint8_t color);

/*
 * 设置写入位置 (直接写模式)
 */
void OLED_SetCursor(uint8_t x, uint8_t y);

#endif /* __OLED_H */
