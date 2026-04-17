# STM32 LVGL 学习主线（单主工程）

这个仓库现在采用「一个主工程 + 5 个演进阶段」模式：

- 主工程路径：`project/main`
- 通过 `APP_STAGE`（1~5）切换学习阶段
- 通过 CMake Preset 一键切换构建配置

## 目录结构

```text
project/main/
  Core/      # 启动与 HAL 基础
  Drivers/   # 板级驱动（当前含 UART）
  App/       # 阶段行为切换
docs/stages/ # 每个阶段的学习目标与操作
resources/   # HAL / LVGL / protothreads 资源库
tools/       # 工具链与调试配置
```

## 快速开始

Stage 1（默认）：

```bash
cmake --preset default
cmake --build --preset default
```

切换到 Stage 2：

```bash
cmake --preset stage2
cmake --build --preset stage2
```

烧录（示例）：

```bash
openocd -f tools/openocd-stlink.cfg -c "program build/root-stage2/project/main/stm32_lvgl_learning.elf verify reset exit"
```

## 阶段说明

- [Stage 1: 最小系统与 HAL 启动](./stages/01_minimum_system.md)
- [Stage 2: UART 调试输出](./stages/02_uart_printf.md)
- [Stage 3: FSMC LCD 驱动](./stages/03_fsmc_lcd.md)
- [Stage 4: LVGL Hello World](./stages/04_lvgl_hello.md)
- [Stage 5: 开机动画 + Protothreads](./stages/05_boot_animation_pt.md)
