/*
 * oled.c - SSD1306/SH1106 OLED I2C 驱动 (128x64)
 *
 * 诊断设计:
 *   - 上电后 PA2 LED 以不同方式闪烁指示进度
 *   - 1次慢闪(亮200ms灭500ms): OLED检测到, 初始化中
 *   - 快速闪烁(亮100ms灭100ms): OLED未检测到
 *   - 长亮: 初始化完成
 */

#include "oled.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>

/* 显存缓冲区 128x64 = 1024字节 */
static uint8_t buffer[OLED_WIDTH * OLED_PAGES];

/* OLED I2C 地址 (8位格式) */
static uint8_t oled_addr = 0;

/* 6x8 ASCII 字库 */
static const uint8_t font6x8[][6] = {
    {0x00,0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0x5F,0x00,0x00,0x00},
    {0x00,0x07,0x00,0x07,0x00,0x00},{0x14,0x7F,0x14,0x7F,0x14,0x00},
    {0x24,0x2A,0x7F,0x2A,0x12,0x00},{0x23,0x13,0x08,0x64,0x62,0x00},
    {0x36,0x49,0x55,0x22,0x50,0x00},{0x00,0x05,0x03,0x00,0x00,0x00},
    {0x00,0x1C,0x22,0x41,0x00,0x00},{0x00,0x41,0x22,0x1C,0x00,0x00},
    {0x08,0x2A,0x1C,0x2A,0x08,0x00},{0x08,0x08,0x3E,0x08,0x08,0x00},
    {0x00,0x50,0x30,0x00,0x00,0x00},{0x08,0x08,0x08,0x08,0x08,0x00},
    {0x00,0x60,0x60,0x00,0x00,0x00},{0x20,0x10,0x08,0x04,0x02,0x00},
    {0x3E,0x51,0x49,0x45,0x3E,0x00},{0x00,0x42,0x7F,0x40,0x00,0x00},
    {0x42,0x61,0x51,0x49,0x46,0x00},{0x21,0x41,0x45,0x4B,0x31,0x00},
    {0x18,0x14,0x12,0x7F,0x10,0x00},{0x27,0x45,0x45,0x45,0x39,0x00},
    {0x3C,0x4A,0x49,0x49,0x30,0x00},{0x01,0x71,0x09,0x05,0x03,0x00},
    {0x36,0x49,0x49,0x49,0x36,0x00},{0x06,0x49,0x49,0x29,0x1E,0x00},
    {0x00,0x36,0x36,0x00,0x00,0x00},
};

/* ========== 底层 I2C 写入 ========== */

/* 发送 n 字节到 OLED (首字节为控制字节: 0x00=命令, 0x40=数据) */
static uint8_t oled_send(uint8_t *buf, uint16_t len)
{
    return (HAL_I2C_Master_Transmit(&hi2c1, oled_addr, buf, len, 100) == HAL_OK);
}

static uint8_t OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = { 0x00, cmd };
    return oled_send(buf, 2);
}

static uint8_t OLED_WriteData(uint8_t data)
{
    uint8_t buf[2] = { 0x40, data };
    return oled_send(buf, 2);
}

/* ========== 设备检测 ========== */

static uint8_t oled_detect(void)
{
    // 先试 0x3C
    if (HAL_I2C_IsDeviceReady(&hi2c1, OLED_I2C_ADDR, 3, 50) == HAL_OK) {
        oled_addr = OLED_I2C_ADDR;
        return 1;
    }
    // 再试 0x3D
    if (HAL_I2C_IsDeviceReady(&hi2c1, OLED_I2C_ADDR_ALT, 3, 50) == HAL_OK) {
        oled_addr = OLED_I2C_ADDR_ALT;
        return 1;
    }
    return 0;
}

/* ========== 对外接口 ========== */

uint8_t OLED_Init(void)
{
    HAL_Delay(100);

    // === 第1步: 检测 I2C 设备 ===
    if (!oled_detect()) {
        // I2C 上没有 OLED 响应
        return 0;
    }

    // === 第2步: 发送初始化序列 ===
    // 这个序列对 SSD1306 和 SH1106 都兼容

    OLED_WriteCmd(0xAE);        // Display OFF

    OLED_WriteCmd(0xD5);        // Clock Divider
    OLED_WriteCmd(0x80);

    OLED_WriteCmd(0xA8);        // MUX Ratio
    OLED_WriteCmd(0x3F);        // 64行

    OLED_WriteCmd(0xD3);        // Display Offset
    OLED_WriteCmd(0x00);

    OLED_WriteCmd(0x40);        // Start Line = 0

    OLED_WriteCmd(0x8D);        // Charge Pump
    OLED_WriteCmd(0x14);        // Enable (3.3V必需!)

    OLED_WriteCmd(0x20);        // Memory Mode (SSD1306)
    OLED_WriteCmd(0x00);        // Horizontal (被SH1106忽略)

    OLED_WriteCmd(0xA1);        // Segment Remap (column 127->0)

    OLED_WriteCmd(0xC8);        // COM Scan Direction (bottom->top)

    OLED_WriteCmd(0xDA);        // COM Pins
    OLED_WriteCmd(0x12);        // Alternative config

    OLED_WriteCmd(0x81);        // Contrast
    OLED_WriteCmd(0xFF);        // 最大对比度!

    OLED_WriteCmd(0xD9);        // Pre-charge
    OLED_WriteCmd(0xF1);

    OLED_WriteCmd(0xDB);        // VCOM Deselect
    OLED_WriteCmd(0x40);

    OLED_WriteCmd(0xA4);        // Display on resume
    OLED_WriteCmd(0xA6);        // Normal display (非反色)
    OLED_WriteCmd(0x2E);        // Deactivate scroll

    // === 第3步: 测试写像素 ===
    // 直接往GDDRAM写测试数据
    OLED_WriteCmd(0xB0);        // Page 0
    OLED_WriteCmd(0x10);        // Column high = 0
    OLED_WriteCmd(0x00);        // Column low = 0

    // 写第0行全部点亮 (测试行)
    for (uint8_t i = 0; i < 128; i++) {
        OLED_WriteData(0xFF);
    }

    OLED_WriteCmd(0xAF);        // Display ON

    HAL_Delay(50);

    // === 第4步: 清显存并全屏填充测试 ===
    OLED_ClearBuffer();

    // 画棋盘格测试图案 (隔列点亮)
    for (uint8_t y = 0; y < OLED_HEIGHT; y++) {
        for (uint8_t x = 0; x < OLED_WIDTH; x++) {
            if ((x + y) % 2 == 0) {
                OLED_DrawPixel(x, y, 1);
            }
        }
    }
    OLED_UpdateScreen();
    HAL_Delay(500);

    // 清掉测试图案
    OLED_ClearBuffer();
    OLED_UpdateScreen();

    return 1;
}

void OLED_ClearBuffer(void)
{
    memset(buffer, 0x00, sizeof(buffer));
}

void OLED_UpdateScreen(void)
{
    // 按页发送, 每页128字节
    for (uint8_t page = 0; page < 8; page++) {
        OLED_WriteCmd(0xB0 + page);
        OLED_WriteCmd(0x10);        // col high
        OLED_WriteCmd(0x00);        // col low

        // 打包129字节: 1控制字节 + 128数据
        uint8_t data_buf[129];
        data_buf[0] = 0x40;
        memcpy(data_buf + 1, buffer + page * OLED_WIDTH, OLED_WIDTH);
        oled_send(data_buf, 129);
    }
}

void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    if (color)
        buffer[x + (y / 8) * OLED_WIDTH] |=  (1 << (y % 8));
    else
        buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
}

void OLED_PutChar(uint8_t x, uint8_t y, char ch)
{
    if (x > OLED_WIDTH - 6 || y > OLED_PAGES - 1) return;
    uint8_t idx = (uint8_t)ch - 0x20;
    if (idx >= sizeof(font6x8) / 6) idx = 0;
    for (uint8_t i = 0; i < 6; i++)
        buffer[x + i + y * OLED_WIDTH] = font6x8[idx][i];
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
