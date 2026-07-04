/*
 * oled.c - 0.96/1.3寸 SSD1306 OLED I2C 驱动 (128x64)
 *
 * 6x8 字体表来源于标准 ASCII 字库
 * 支持中英文混排 (中文用替代方案)
 */

#include "oled.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>

/* ========== 显存缓冲区 ==========
 * 128 列 x 8 页 (每页8行) = 1024 字节
 */
static uint8_t buffer[OLED_WIDTH * OLED_PAGES];

/* ========== 6x8 ASCII 字库 (部分) ========== */
static const uint8_t font6x8[][6] = {
    { 0x00,0x00,0x00,0x00,0x00,0x00 }, /* space */
    { 0x00,0x00,0x5F,0x00,0x00,0x00 }, /* ! */
    { 0x00,0x07,0x00,0x07,0x00,0x00 }, /* " */
    { 0x14,0x7F,0x14,0x7F,0x14,0x00 }, /* # */
    { 0x24,0x2A,0x7F,0x2A,0x12,0x00 }, /* $ */
    { 0x23,0x13,0x08,0x64,0x62,0x00 }, /* % */
    { 0x36,0x49,0x55,0x22,0x50,0x00 }, /* & */
    { 0x00,0x05,0x03,0x00,0x00,0x00 }, /* ' */
    { 0x00,0x1C,0x22,0x41,0x00,0x00 }, /* ( */
    { 0x00,0x41,0x22,0x1C,0x00,0x00 }, /* ) */
    { 0x08,0x2A,0x1C,0x2A,0x08,0x00 }, /* * */
    { 0x08,0x08,0x3E,0x08,0x08,0x00 }, /* + */
    { 0x00,0x50,0x30,0x00,0x00,0x00 }, /* , */
    { 0x08,0x08,0x08,0x08,0x08,0x00 }, /* - */
    { 0x00,0x60,0x60,0x00,0x00,0x00 }, /* . */
    { 0x20,0x10,0x08,0x04,0x02,0x00 }, /* / */
    { 0x3E,0x51,0x49,0x45,0x3E,0x00 }, /* 0 */
    { 0x00,0x42,0x7F,0x40,0x00,0x00 }, /* 1 */
    { 0x42,0x61,0x51,0x49,0x46,0x00 }, /* 2 */
    { 0x21,0x41,0x45,0x4B,0x31,0x00 }, /* 3 */
    { 0x18,0x14,0x12,0x7F,0x10,0x00 }, /* 4 */
    { 0x27,0x45,0x45,0x45,0x39,0x00 }, /* 5 */
    { 0x3C,0x4A,0x49,0x49,0x30,0x00 }, /* 6 */
    { 0x01,0x71,0x09,0x05,0x03,0x00 }, /* 7 */
    { 0x36,0x49,0x49,0x49,0x36,0x00 }, /* 8 */
    { 0x06,0x49,0x49,0x29,0x1E,0x00 }, /* 9 */
    { 0x00,0x36,0x36,0x00,0x00,0x00 }, /* : */
};

/* ========== 底层写入 ========== */

/* 写命令 */
static void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = { 0x00, cmd };    // 0x00 = 命令模式
    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR, buf, 2, I2C_TIMEOUT);
}

/* 写数据 */
static void OLED_WriteData(uint8_t data)
{
    uint8_t buf[2] = { 0x40, data };   // 0x40 = 数据模式
    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR, buf, 2, I2C_TIMEOUT);
}

/* ========== 初始化 ========== */

void OLED_Init(void)
{
    HAL_Delay(100);     // 等待上电稳定

    OLED_WriteCmd(OLED_CMD_DISPLAY_OFF);        // 关闭显示
    OLED_WriteCmd(OLED_CMD_SET_MUX_RATIO);      // 多路复用比
    OLED_WriteCmd(0x3F);                        // 64行
    OLED_WriteCmd(OLED_CMD_SET_DISPLAY_OFFSET); // 偏移
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(OLED_CMD_DISPLAY_START_LINE); // 起始行
    OLED_WriteCmd(OLED_CMD_SEG_REMAP);          // 段重映射 (列0对应SEG127)
    OLED_WriteCmd(OLED_CMD_COM_SCAN_DEC);       // COM反向扫描
    OLED_WriteCmd(OLED_CMD_SET_COM_PINS);       // COM引脚配置
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(OLED_CMD_SET_CONTRAST);       // 对比度
    OLED_WriteCmd(0x7F);
    OLED_WriteCmd(OLED_CMD_ENTIRE_DISP_ON);     // 全局显示开启
    OLED_WriteCmd(OLED_CMD_SET_NORM_DISP);      // 正常显示 (非反色)
    OLED_WriteCmd(OLED_CMD_SET_CLK_DIV);        // 时钟分频/振荡频率
    OLED_WriteCmd(0x80);
    OLED_WriteCmd(OLED_CMD_CHARGE_PUMP);        // 电荷泵
    OLED_WriteCmd(0x14);                        // 开启
    OLED_WriteCmd(OLED_CMD_SET_VCOM_DESELECT);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(OLED_CMD_SET_MEM_MODE);       // 内存地址模式
    OLED_WriteCmd(0x00);                        // 水平模式
    OLED_WriteCmd(OLED_CMD_SET_PRECHARGE);
    OLED_WriteCmd(0xF1);

    OLED_ClearBuffer();
    OLED_UpdateScreen();
    OLED_WriteCmd(OLED_CMD_DISPLAY_ON);         // 开启显示
}

/* ========== 缓冲区操作 ========== */

void OLED_ClearBuffer(void)
{
    memset(buffer, 0x00, sizeof(buffer));
}

void OLED_UpdateScreen(void)
{
    OLED_SetCursor(0, 0);

    for (uint16_t i = 0; i < sizeof(buffer); i++) {
        OLED_WriteData(buffer[i]);
    }
}

void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;

    if (color) {
        buffer[x + (y / 8) * OLED_WIDTH] |=  (1 << (y % 8));
    } else {
        buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
    }
}

/* ========== 字符显示 ========== */

void OLED_PutChar(uint8_t x, uint8_t y, char ch)
{
    if (x > OLED_WIDTH - 6 || y > OLED_PAGES - 1) return;

    uint8_t idx = (uint8_t)ch - 0x20;   // 字库从空格开始
    if (idx >= sizeof(font6x8) / 6) idx = 0;

    for (uint8_t i = 0; i < 6; i++) {
        buffer[x + i + y * OLED_WIDTH] = font6x8[idx][i];
    }
}

void OLED_PutString(uint8_t x, uint8_t y, const char *str)
{
    while (*str) {
        OLED_PutChar(x, y, *str);
        x += 6;
        if (x > OLED_WIDTH - 6) { x = 0; y++; }
        str++;
    }
}

void OLED_PutNumber(uint8_t x, uint8_t y, uint32_t num, uint8_t len)
{
    char buf[12];
    snprintf(buf, sizeof(buf), "%*lu", len, (unsigned long)num);
    OLED_PutString(x, y, buf);
}

/* ========== 图形绘制 ========== */

void OLED_DrawHLine(uint8_t x0, uint8_t x1, uint8_t y)
{
    for (uint8_t x = x0; x <= x1; x++)
        OLED_DrawPixel(x, y, 1);
}

void OLED_DrawVLine(uint8_t x, uint8_t y0, uint8_t y1)
{
    for (uint8_t y = y0; y <= y1; y++)
        OLED_DrawPixel(x, y, 1);
}

void OLED_DrawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    OLED_DrawHLine(x0, x1, y0);
    OLED_DrawHLine(x0, x1, y1);
    OLED_DrawVLine(x0, y0, y1);
    OLED_DrawVLine(x1, y0, y1);
}

void OLED_Fill(uint8_t color)
{
    memset(buffer, color ? 0xFF : 0x00, sizeof(buffer));
}

void OLED_SetCursor(uint8_t x, uint8_t y)
{
    OLED_WriteCmd(0xB0 + y);                    // 设置页地址
    OLED_WriteCmd(((x & 0xF0) >> 4) | 0x10);    // 设置列地址高4位
    OLED_WriteCmd(x & 0x0F);                    // 设置列地址低4位
}
