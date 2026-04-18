# Stage 3：FSMC LCD 驱动

## 目标

- 接入 ST7789（FSMC 模式）
- 验证地址线、片选线与命令/数据写入流程
- 建立 LCD 驱动与应用层之间的抽象边界

## 已实现内容

- 新增 `project/main/Drivers/Inc/fsmc_lcd.h`
- 新增 `project/main/Drivers/Src/fsmc_lcd.c`
- `APP_STAGE=3` 下已接入：
  - FSMC 总线初始化（8-bit 并口）
  - ST7789 初始化序列
  - 全屏纯色填充
  - 周期换色演示（红/绿/蓝/白）

## 参考来源

- `D:\software\RT-ThreadStudio\workspace\06_demo_factory\libraries\Board_Drivers\lcd\drv_lcd.c`
- `D:\software\RT-ThreadStudio\workspace\06_demo_factory\board\CubeMX_Config\CubeMX_Config.ioc`

## 引脚映射（与参考工程一致）

- 控制线：
  - `PG10` -> `FSMC_NE3`（片选）
  - `PD13` -> `FSMC_A18`（DCX）
  - `PD4` -> `FSMC_NOE`
  - `PD5` -> `FSMC_NWE`
- 数据线：
  - `PD14/PD15/PD0/PD1` -> `D0~D3`
  - `PE7/PE8/PE9/PE10` -> `D4~D7`
- 其他：
  - `PD3` -> LCD 复位
  - `PF9` -> LCD 背光

## 构建

```bash
cmake --preset stage3
cmake --build --preset stage3
```

## 烧录与验证

```bash
openocd -f tools/openocd-stlink.cfg -c "program build/root-stage3/project/main/stm32_lvgl_learning.elf verify reset exit"
```

预期现象：

- 串口输出：`[stage3] lcd init ok`
- 屏幕每隔约 500 ms 切换纯色

## 已知限制

- 目前仅实现最小显示链路，还未接入 LVGL `flush_cb`
- 颜色填充为阻塞式写入，后续可在 Stage 4 优化
