/*
 * oled.c - 0.96/1.3寸 OLED I2C 驱动
 * 同时支持 SSD1306 和 SH1106 驱动芯片
 * 分辨率: 128x64
 * I2C地址自动检测: 0x3C 或 0x3D
 */

#include "oled.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>

/* ========== 显存缓冲区 128x64 = 1024字节 ========== */
static uint8_t buffer[OLED_WIDTH * OLED_PAGES];

/* 检测到的 OLED 类型 */
static uint8_t oled_is_sh1106 = 0;
/* 检测到的 I2C 地址 (8位左移格式) */
static uint8_t oled_addr = OLED_I2C_ADDR;

/* ========== 6x8 ASCII 字库 ========== */
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

/* ========== 底层 I2C 写入 ========== */

/* 写命令 (单字节) */
static uint8_t OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = { 0x00, cmd };    // 0x00 = 命令模式
    return (HAL_I2C_Master_Transmit(&hi2c1, oled_addr, buf, 2, I2C_TIMEOUT) == HAL_OK);
}

/* 写数据 (单字节) */
static uint8_t OLED_WriteData(uint8_t data)
{
    uint8_t buf[2] = { 0x40, data };   // 0x40 = 数据模式
    return (HAL_I2C_Master_Transmit(&hi2c1, oled_addr, buf, 2, I2C_TIMEOUT) == HAL_OK);
}

/* ========== 设备检测 ========== */

/*
 * 尝试在指定 I2C 地址检测 OLED
 * 返回 1=检测到设备, 0=无设备
 */
static uint8_t OLED_DetectAt(uint8_t addr)
{
    if (HAL_I2C_IsDeviceReady(&hi2c1, addr, 3, I2C_TIMEOUT) == HAL_OK) {
        return 1;
    }
    return 0;
}

/* ========== 初始化序列 ========== */

/* SSD1306 初始化序列 */
static void OLED_Init_SSD1306(void)
{
    OLED_WriteCmd(0xAE);        // Display OFF
    OLED_WriteCmd(0xD5);        // Clock Divider
    OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xA8);        // MUX Ratio
    OLED_WriteCmd(0x3F);        // 64
    OLED_WriteCmd(0xD3);        // Display Offset
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x40);        // Start Line
    OLED_WriteCmd(0x8D);        // Charge Pump
    OLED_WriteCmd(0x14);        // Enable
    OLED_WriteCmd(0x20);        // Memory Mode
    OLED_WriteCmd(0x00);        // Horizontal
    OLED_WriteCmd(0xA1);        // Segment Remap
    OLED_WriteCmd(0xC8);        // COM Scan Direction
    OLED_WriteCmd(0xDA);        // COM Pins
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0x81);        // Contrast
    OLED_WriteCmd(0xCF);
    OLED_WriteCmd(0xD9);        // Pre-charge
    OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDB);        // VCOM Deselect
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0xA4);        // Display on resume
    OLED_WriteCmd(0xA6);        // Normal display
    OLED_WriteCmd(0x2E);        // Deactivate scroll
    OLED_WriteCmd(0xAF);        // Display ON
}

/* SH1106 初始化序列 (1.3寸 OLED 常用) */
static void OLED_Init_SH1106(void)
{
    OLED_WriteCmd(0xAE);        // Display OFF
    OLED_WriteCmd(0xD5);        // Clock Divider
    OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xA8);        // MUX Ratio
    OLED_WriteCmd(0x3F);        // 64
    OLED_WriteCmd(0xD3);        // Display Offset
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x40);        // Start Line
    OLED_WriteCmd(0x8D);        // Charge Pump
    OLED_WriteCmd(0x14);        // Enable
    OLED_WriteCmd(0xA0);        // Segment Remap (SH1106: 0xA0=normal, 0xA1=remap)
    OLED_WriteCmd(0xC0);        // COM Scan Direction (SH1106: 0xC0=normal, 0xC8=remap)
    OLED_WriteCmd(0xDA);        // COM Pins
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0x81);        // Contrast
    OLED_WriteCmd(0xCF);
    OLED_WriteCmd(0xD9);        // Pre-charge
    OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDB);        // VCOM Deselect
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0xA4);        // Display on resume
    OLED_WriteCmd(0xA6);        // Normal display
    OLED_WriteCmd(0xAF);        // Display ON
}

/* ========== 对外接口 ========== */

uint8_t OLED_Init(void)
{
    HAL_Delay(50);      // 等待上电稳定

    /* 先尝试检测 OLED 设备 */
    if (OLED_DetectAt(OLED_I2C_ADDR)) {
        oled_addr = OLED_I2C_ADDR;
    } else if (OLED_DetectAt(OLED_I2C_ADDR_ALT)) {
        oled_addr = OLED_I2C_ADDR_ALT;
    } else {
        // 没检测到 OLED
        return 0;
    }

    // 先尝试用 SH1106 初始化 (1.3寸)
    OLED_Init_SH1106();
    HAL_Delay(20);

    // 写一点数据测试是否正常工作
    OLED_WriteCmd(0xB0);    // page 0
    OLED_WriteCmd(0x10);    // col high
    OLED_WriteCmd(0x02);    // col low (SH1106 offset)
    OLED_WriteData(0xFF);   // 写全亮字节

    // 转回正常模式
    OLED_Init_SSD1306();

    OLED_ClearBuffer();
    OLED_UpdateScreen();

    return 1;   // 成功
}

void OLED_ClearBuffer(void)
{
    memset(buffer, 0x00, sizeof(buffer));
}

/*
 * 批量发送显存数据到 OLED
 * 使用页模式: 每页 128 字节，共 8 页
 * 对于 SH1106，列偏移 2 (从第2列开始映射)
 */
void OLED_UpdateScreen(void)
{
    for (uint8_t page = 0; page < 8; page++) {
        // 设置页地址
        OLED_WriteCmd(0xB0 + page);
        // 设置列地址 (SH1106 偏移2列)
        OLED_WriteCmd(0x10);                    // 列高4位
        OLED_WriteCmd(oled_is_sh1106 ? 0x02 : 0x00);  // 列低4位

        /* 一次性发送一页的数据 (128字节) */
        // 由于 HAL I2C 限制，分块发送
        uint8_t data_buf[130];  // 1 control byte + 128 data
        data_buf[0] = 0x40;     // 数据模式

        for (uint8_t col = 0; col < OLED_WIDTH; col++) {
            data_buf[1 + col] = buffer[col + page * OLED_WIDTH];
        }

        HAL_I2C_Master_Transmit(&hi2c1, oled_addr, data_buf, 129, I2C_TIMEOUT);
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

    uint8_t idx = (uint8_t)ch - 0x20;
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
    OLED_WriteCmd(0xB0 + y);
    OLED_WriteCmd(((x & 0xF0) >> 4) | 0x10);
    OLED_WriteCmd(x & 0x0F);
}
