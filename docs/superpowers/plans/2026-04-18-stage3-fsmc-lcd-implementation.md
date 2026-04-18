# Stage 3（FSMC + LCD 最小可显示）实现计划

> **面向 AI 代理的工作者：** 必需子技能：使用 superpowers:subagent-driven-development（推荐）或 superpowers:executing-plans 逐任务实现此计划。步骤使用复选框（`- [ ]`）语法来跟踪进度。

**目标：** 在 `APP_STAGE=3` 下基于 `D:\software\RT-ThreadStudio\workspace\06_demo_factory` 参考工程，完成 FSMC 并口 LCD（ST7789）最小可显示链路，实现开机纯色填充和周期换色。

**架构：** 新增 `Drivers/fsmc_lcd` 作为单一 LCD 底层模块，应用层 `app_stage.c` 仅调用 `fsmc_lcd_init()` 与 `fsmc_lcd_fill_color()`。构建层在 `project/main/CMakeLists.txt` 加入新驱动源，HAL 配置补齐 FSMC/SRAM 依赖，确保 stage1/2/3 可并行构建。

**技术栈：** STM32F407 HAL（FSMC/SRAM/GPIO/UART）、CMake + Ninja、ARM GCC、OpenOCD（板级验证）。

---

## 参考源（必须对齐）

- `D:\software\RT-ThreadStudio\workspace\06_demo_factory\libraries\Board_Drivers\lcd\drv_lcd.c`
- `D:\software\RT-ThreadStudio\workspace\06_demo_factory\libraries\Board_Drivers\lcd\drv_lcd.h`
- `D:\software\RT-ThreadStudio\workspace\06_demo_factory\board\CubeMX_Config\CubeMX_Config.ioc`

关键迁移点：

- FSMC 总线：`FSMC_NORSRAM_BANK3`，`PG10 = FSMC_NE3`
- A18 作为 `DCX`：`PD13 = FSMC_A18`
- 数据线：`D0~D7`（PD14/PD15/PD0/PD1/PE7/PE8/PE9/PE10）
- 控制线：`PD4 NOE`，`PD5 NWE`
- 复位脚：`PD3`（GPIO）
- 背光脚：`PF9`（GPIO 或 PWM；本计划采用 GPIO 先打通）
- LCD 基地址：`0x68000000 | 0x0003FFFE`

## 文件结构（先锁定职责）

- 创建：`project/main/Drivers/Inc/fsmc_lcd.h`  
  职责：暴露 Stage 3 最小接口和常量定义（颜色、状态）。

- 创建：`project/main/Drivers/Src/fsmc_lcd.c`  
  职责：FSMC GPIO + SRAM 初始化、LCD 指令序列、全屏填充。

- 修改：`project/main/CMakeLists.txt`  
  职责：纳入 `fsmc_lcd.c` 编译，并补齐 HAL 源文件依赖（`hal_sram.c`）。

- 修改：`project/main/Core/Inc/stm32f4xx_hal_conf.h`  
  职责：使能 `HAL_SRAM_MODULE_ENABLED`，包含 `stm32f4xx_hal_sram.h` 与 `stm32f4xx_ll_fsmc.h`。

- 修改：`project/main/App/Src/app_stage.c`  
  职责：Stage 3 初始化 LCD，按 tick 周期切换纯色并输出 UART 日志。

- 修改：`docs/stages/03_fsmc_lcd.md`  
  职责：同步“已完成最小可显示”状态、引脚映射、验证步骤。

---

### 任务 1：接入 Stage 3 LCD 模块骨架

**文件：**
- 创建：`project/main/Drivers/Inc/fsmc_lcd.h`
- 创建：`project/main/Drivers/Src/fsmc_lcd.c`
- 修改：`project/main/CMakeLists.txt`
- 测试：`cmake --preset stage3 && cmake --build --preset stage3`

- [ ] **步骤 1：在应用层引入尚未存在的头文件，制造可控失败**

```c
/* project/main/App/Src/app_stage.c */
#if APP_STAGE == 3
#include "fsmc_lcd.h"
#endif
```

- [ ] **步骤 2：运行构建验证失败（验证测试先失败）**

运行：`cmake --preset stage3 && cmake --build --preset stage3`  
预期：FAIL，报错类似 `fatal error: fsmc_lcd.h: No such file or directory`

- [ ] **步骤 3：补齐最小头文件与空实现（最少代码让编译恢复）**

```c
/* project/main/Drivers/Inc/fsmc_lcd.h */
#ifndef __FSMC_LCD_H
#define __FSMC_LCD_H

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef fsmc_lcd_init(void);
void fsmc_lcd_fill_color(uint16_t rgb565);

#endif
```

```c
/* project/main/Drivers/Src/fsmc_lcd.c */
#include "fsmc_lcd.h"

HAL_StatusTypeDef fsmc_lcd_init(void)
{
    return HAL_OK;
}

void fsmc_lcd_fill_color(uint16_t rgb565)
{
    (void)rgb565;
}
```

```cmake
# project/main/CMakeLists.txt (APP_SOURCES 追加)
${PROJECT_DIR}/Drivers/Src/fsmc_lcd.c
```

- [ ] **步骤 4：运行构建验证通过**

运行：`cmake --preset stage3 && cmake --build --preset stage3`  
预期：PASS，生成 `build/root-stage3/project/main/stm32_lvgl_learning.elf`

- [ ] **步骤 5：Commit**

```bash
git add project/main/Drivers/Inc/fsmc_lcd.h project/main/Drivers/Src/fsmc_lcd.c project/main/CMakeLists.txt project/main/App/Src/app_stage.c
git commit -m "feat(stage3): add fsmc lcd driver skeleton and build integration"
```

---

### 任务 2：迁移 FSMC 总线与 ST7789 初始化序列（参考 demo_factory）

**文件：**
- 修改：`project/main/Drivers/Src/fsmc_lcd.c`
- 修改：`project/main/Core/Inc/stm32f4xx_hal_conf.h`
- 修改：`project/main/CMakeLists.txt`
- 测试：`cmake --build --preset stage3 --clean-first`

- [ ] **步骤 1：先写失败检查（缺少 HAL_SRAM 依赖时应失败）**

在 `fsmc_lcd.c` 中加入以下声明并调用：

```c
static SRAM_HandleTypeDef s_hsram;
/* ... */
if (HAL_SRAM_Init(&s_hsram, &read_timing, &write_timing) != HAL_OK)
{
    return HAL_ERROR;
}
```

然后直接构建。  
运行：`cmake --build --preset stage3 --clean-first`  
预期：FAIL，报错包含 `HAL_SRAM_Init` 未声明或链接不到 `stm32f4xx_hal_sram.c`

- [ ] **步骤 2：补齐 HAL 依赖让失败转绿**

```c
/* project/main/Core/Inc/stm32f4xx_hal_conf.h */
#define HAL_SRAM_MODULE_ENABLED
#include "stm32f4xx_hal_sram.h"
#include "stm32f4xx_ll_fsmc.h"
```

```cmake
# project/main/CMakeLists.txt -> HAL_SOURCES 追加
${HAL_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sram.c
```

- [ ] **步骤 3：实现与参考工程一致的最小驱动主流程**

实现点（代码必须落在 `fsmc_lcd.c`）：

```c
/* 关键常量（对齐参考工程） */
#define LCD_RST_PORT GPIOD
#define LCD_RST_PIN  GPIO_PIN_3
#define LCD_BL_PORT  GPIOF
#define LCD_BL_PIN   GPIO_PIN_9
#define LCD_BASE_ADDR ((uint32_t)(0x68000000U | 0x0003FFFEU))

typedef struct
{
    __IO uint8_t REG8;
    __IO uint8_t RESERVED;
    __IO uint8_t RAM8;
    __IO uint16_t RAM16;
} lcd_bus_t;

#define LCD_BUS ((lcd_bus_t *)LCD_BASE_ADDR)
```

```c
/* 初始化顺序 */
1) GPIO: RST/BL 输出，FSMC 引脚复用（PD0/1/4/5/13/14/15 + PE7/8/9/10 + PG10）
2) FSMC: BANK3, 8bit, ExtendedMode ENABLE, read/write timing 分离
3) 硬复位：RST 拉低 100ms -> 拉高 100ms
4) ST7789 初始化：0x36/0x3A/0xB2/.../0x11 -> delay 120ms -> 0x29
5) 开背光：PF9 置高
```

- [ ] **步骤 4：构建验证通过**

运行：`cmake --build --preset stage3 --clean-first`  
预期：PASS，无 `FSMC_*` 未定义、无 `HAL_SRAM_*` 未定义

- [ ] **步骤 5：Commit**

```bash
git add project/main/Drivers/Src/fsmc_lcd.c project/main/Core/Inc/stm32f4xx_hal_conf.h project/main/CMakeLists.txt
git commit -m "feat(stage3): port fsmc ne3 lcd init sequence from demo_factory"
```

---

### 任务 3：接入 Stage 3 应用层最小显示（纯色切换）

**文件：**
- 修改：`project/main/App/Src/app_stage.c`
- 修改：`project/main/Drivers/Inc/fsmc_lcd.h`
- 测试：`cmake --build --preset stage3`，板级上电观察

- [ ] **步骤 1：先写失败检查（调用新接口但未声明）**

在 `app_stage.c` 的 `APP_STAGE==3` 分支先添加：

```c
static const uint16_t k_stage3_colors[] = {0xF800, 0x07E0, 0x001F, 0xFFFF};
/* 调用 fsmc_lcd_fill_color(...) */
```

若 `fsmc_lcd.h` 尚无公开颜色宏或接口，构建应失败。  
运行：`cmake --build --preset stage3`  
预期：FAIL，报错符号未声明。

- [ ] **步骤 2：补齐接口声明并实现阶段逻辑**

```c
/* app_stage.c, APP_STAGE == 3 */
if (fsmc_lcd_init() != HAL_OK)
{
    board_uart_write("[stage3] lcd init failed\r\n");
}
else
{
    board_uart_write("[stage3] lcd init ok\r\n");
    fsmc_lcd_fill_color(0xF800);
}
```

```c
/* app_stage_loop(), APP_STAGE == 3 */
uint32_t now = HAL_GetTick();
if ((now - s_last_tick) >= 500U)
{
    static uint8_t idx = 0U;
    fsmc_lcd_fill_color(k_stage3_colors[idx]);
    idx = (uint8_t)((idx + 1U) % 4U);
    s_last_tick = now;
}
```

- [ ] **步骤 3：构建验证通过**

运行：`cmake --build --preset stage3`  
预期：PASS

- [ ] **步骤 4：板级验证**

运行（示例）：

```bash
openocd -f tools/openocd-stlink.cfg -c "program build/root-stage3/project/main/stm32_lvgl_learning.elf verify reset exit"
```

预期：

- 串口输出 `lcd init ok`
- 屏幕周期切换红/绿/蓝/白

- [ ] **步骤 5：Commit**

```bash
git add project/main/App/Src/app_stage.c project/main/Drivers/Inc/fsmc_lcd.h
git commit -m "feat(stage3): add lcd color-cycle demo in app stage"
```

---

### 任务 4：回归构建与文档收口

**文件：**
- 修改：`docs/stages/03_fsmc_lcd.md`
- 测试：`stage1/stage2/stage3` 全量构建

- [ ] **步骤 1：运行回归构建**

运行：

```bash
cmake --preset stage1 && cmake --build --preset stage1
cmake --preset stage2 && cmake --build --preset stage2
cmake --preset stage3 && cmake --build --preset stage3
```

预期：全部 PASS

- [ ] **步骤 2：更新 Stage 3 文档为“已落地”**

文档必须包含：

- 已实现功能：FSMC 初始化、LCD 初始化、纯色切换
- 引脚映射（来自 `CubeMX_Config.ioc`）
- 最小验证步骤（构建、烧录、串口、屏幕观察）
- 已知限制（尚未接入 LVGL）

- [ ] **步骤 3：Commit**

```bash
git add docs/stages/03_fsmc_lcd.md
git commit -m "docs(stage3): document fsmc lcd bring-up and validation steps"
```

- [ ] **步骤 4：打里程碑标签**

```bash
git tag stage3-lcd-bringup
```

- [ ] **步骤 5：推送远端（可选）**

```bash
git push origin main
git push --tags
```

---

## 完成定义（DoD）

- `stage3` 可编译且板级可见颜色切换。
- `stage1/stage2` 回归构建通过。
- `docs/stages/03_fsmc_lcd.md` 已同步真实实现状态。
- 提交历史至少 4 个增量 commit，且含 `stage3-lcd-bringup` 标签。
