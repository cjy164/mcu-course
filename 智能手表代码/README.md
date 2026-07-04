# 智能手表代码 — STM32F103C8T6 + FreeRTOS

## 项目概述

基于 **STM32F103C8T6** 的智能手表固件，使用 **FreeRTOS** 实时操作系统实现多任务管理，集成 **SSD1306 OLED** 显示和 **MPU6050** 六轴传感器。

### 功能（基本要求）

- ✅ OLED 时间页面显示（时:分:秒 + 日期）
- ✅ OLED 传感器页面显示（加速度 X/Y/Z + 温度）
- ✅ 按键切换页面
- ✅ MPU6050 加速度/温度数据采集（200ms 周期）
- ✅ 软件 RTC 计时（基于 TIM2 1秒中断）
- ✅ FreeRTOS 多任务调度

### 不包含（拓展内容）

- ❌ 旋转编码器菜单
- ❌ 蓝牙通信与上位机
- ❌ PCB 设计
- ❌ 运动计步算法

---

## 硬件连接

### 引脚分配

| STM32 引脚 | 连接目标 | 说明 |
|:----------:|:--------:|:----:|
| PB6 (I2C1_SCL) | OLED SCL + MPU6050 SCL | I2C 时钟线 |
| PB7 (I2C1_SDA) | OLED SDA + MPU6050 SDA | I2C 数据线 |
| PA0 | 按键 (对地) | 外部中断，下降沿触发 |
| PA2 | (可选) 马达驱动 | GPIO 推挽输出 |
| 3.3V | OLED VCC + MPU6050 VCC | 供电 |
| GND | OLED GND + MPU6050 GND | 共地 |

> **I2C 上拉电阻**: SCL 和 SDA 各接 4.7kΩ 到 3.3V（某些模块已内置）

### 杜邦线连接

```
STM32F103     OLED 1.3"       MPU6050
 ┌──────┐     ┌────────┐     ┌────────┐
 │ PB6  ├─────┤ SCL    ├─────┤ SCL    │
 │ PB7  ├─────┤ SDA    ├─────┤ SDA    │
 │ 3.3V ├─────┤ VCC    ├─────┤ VCC    │
 │ GND  ├─────┤ GND    ├─────┤ GND    │
 └──────┘     └────────┘     └────────┘
```

---

## 使用 STM32CubeMX 生成基础工程

### 步骤

1. 打开 **STM32CubeMX**
2. 选择芯片: **STM32F103C8T6**
3. 配置以下外设:

#### 1. RCC
- High Speed Clock (HSE): **Crystal/Ceramic Resonator**
- Low Speed Clock (LSE): **Disabled**

#### 2. SYS
- Timebase Source: **TIM1** (避免与 FreeRTOS 的 SysTick 冲突)
- Debug: **Serial Wire** (如果使用 SWD 调试器)

#### 3. I2C1
- Mode: **I2C**
- PB6: I2C1_SCL
- PB7: I2C1_SDA
- 参数: Speed Mode = Standard (100KHz)

#### 4. USART1 (可选，拓展蓝牙时用)
- Mode: Asynchronous
- PA9: TX, PA10: RX
- Baud Rate: 9600

#### 5. GPIO
- PA0: **GPIO_EXTI0**, Pull-up, Falling edge
- PA2: **GPIO_Output**, Push-Pull, Low

#### 6. TIM2
- Clock Source: **Internal**
- Prescaler: 7200-1
- Counter Period: 10000-1
- Enable interrupt: **TIM2 global interrupt**

#### 7. FreeRTOS
- Interface: **CMSIS_V2**
- 在 Tasks 中手动创建:

| Task Name | Stack Size | Priority | Entry Function |
|:---------:|:----------:|:--------:|:--------------:|
| oledTask  | 256        | Normal   | OLED_Task      |
| sensorTask| 256        | AboveNormal | Sensor_Task |
| buttonTask| 256        | High     | Button_Task    |

> 或者不在 CubeMX 中创建任务，直接使用本项目的 `xTaskCreate()` 代码（已在 main.c 中实现）

#### 8. 时钟配置
- HSE: 8MHz
- PLL: x9
- SYSCLK: **72MHz**
- APB1: 36MHz
- APB2: 72MHz

### 生成代码

1. Project Name: `SmartWatch`
2. Toolchain / IDE: **MDK-ARM** 或 **STM32CubeIDE**
3. 勾选: **Generate peripheral initialization as a pair of .c/.h files per peripheral**
4. 点击 GENERATE CODE

### 替换文件

将本项目的 `Core/Src/` 和 `Core/Inc/` 中的文件覆盖到 CubeMX 生成的工程中：

| 文件 | 操作 |
|:----|:----|
| `Core/Src/main.c` | 替换（保留 CubeMX 生成的初始化） |
| `Core/Src/oled.c` | 新增 |
| `Core/Src/mpu6050.c` | 新增 |
| `Core/Src/soft_timer.c` | 新增 |
| `Core/Src/i2c.c` | 替换（保留 `hi2c1` 定义） |
| `Core/Src/ui.c` | 新增 |
| `Core/Inc/oled.h` | 新增 |
| `Core/Inc/mpu6050.h` | 新增 |
| `Core/Inc/soft_timer.h` | 新增 |
| `Core/Inc/i2c.h` | 替换 |
| `Core/Inc/ui.h` | 新增 |
| `Core/Inc/FreeRTOSConfig.h` | 替换 |

> **注意**: 替换 `main.c` 时，保留 CubeMX 生成的 `MX_*_Init()` 函数和 `HAL_Init()`，本项目代码已兼容。

---

## 烧录步骤

### 方法 1: STM32 ST-LINK Utility / STM32CubeProgrammer

1. 编译生成 `.hex` 或 `.bin` 文件
2. 连接 ST-LINK 调试器（SWD: SWDIO, SWCLK, GND, 3.3V）
3. 打开 STM32CubeProgrammer
4. 选择 ST-LINK 连接 → 点击 Connect
5. 加载 `.hex` 文件 → 点击 Download

### 方法 2: Keil MDK

1. 打开工程 `.uvprojx`
2. 点击 **Build (F7)** 编译
3. 点击 **Load (F8)** 烧录

### 方法 3: 串口 ISP (FlyMcu)

1. 将 BOOT0 置 1, BOOT1 置 0
2. 上电/复位
3. 使用 FlyMcu 选择 `.hex` 文件
4. 波特率: 115200
5. 点击"开始编程"
6. 烧录完成后将 BOOT0 恢复为 0，复位运行

---

## 首次使用

1. 烧录完成后复位芯片
2. OLED 显示 `Starting...` 和 `MPU6050 OK` / `MPU6050 ERR`
3. 进入时间显示页面（时:分:秒 + 日期）
4. 按下 PA0 按键切换到传感器数据页面
5. 再次按下按键切回时间页面

### 时间设置

初始化时间为 `2026-07-01 00:00:00`，如需修改，在 `main.c` 中找到:
```c
SoftTimer_SetTime(2026, 7, 1, 0, 0, 0);
```
修改参数后重新编译烧录。

---

## 代码结构

```
智能手表代码/
├── Core/
│   ├── Inc/
│   │   ├── FreeRTOSConfig.h     # FreeRTOS 内核配置
│   │   ├── i2c.h               # I2C 总线封装
│   │   ├── oled.h              # SSD1306 OLED 驱动
│   │   ├── mpu6050.h           # MPU6050 传感器驱动
│   │   ├── soft_timer.h        # 软件计时器
│   │   └── ui.h                # UI 页面管理
│   └── Src/
│       ├── main.c              # 主程序 + 任务定义 + 硬件初始化
│       ├── i2c.c               # I2C 实现
│       ├── oled.c              # OLED 驱动实现 (6x8字库)
│       ├── mpu6050.c           # MPU6050 驱动实现
│       ├── soft_timer.c        # 软件 RTC 实现
│       └── ui.c                # UI 页面绘制
├── MDK-ARM/                    # Keil 工程文件夹
└── README.md                   # 本文件
```

---

## FreeRTOS 任务说明

| 任务 | 优先级 | 周期 | 功能 |
|:----|:-----:|:----:|:----|
| OLED_Task | 2 | 500ms | 刷新 OLED 显示 |
| Sensor_Task | 3 | 200ms | 读取 MPU6050 数据 |
| Button_Task | 4 | 20ms 轮询 | 检测按键、切换页面 |

调度方式: **抢占式**，高优先级任务可打断低优先级任务。
