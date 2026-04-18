# Stage 3 设计规格：FSMC + LCD 最小可显示链路

## 1. 背景与目标

当前仓库采用「一个主工程 + 阶段递进」模式，Stage 1 和 Stage 2 已具备可编译与可观测基础。  
本设计聚焦 Stage 3：在 `APP_STAGE=3` 下打通 FSMC 驱动 ST7789（并口模式）的最小显示路径，实现开机可见画面验证。

目标：

- 完成 FSMC 与 LCD 的基础初始化。
- 提供最小绘制能力：纯色填充（全屏）。
- 在应用层实现周期换色演示，用于确认显示链路可用。
- 保持 Stage 1 / Stage 2 行为和构建链路不回归。

非目标：

- 不在本阶段接入 LVGL。
- 不在本阶段实现复杂绘图 API 或动画系统。
- 不引入触摸或输入设备。

## 2. 方案对比与结论

候选方案：

1. 最小可编译占位：只补框架，保留空实现。  
   优点：风险低。缺点：无法验证硬件链路，学习价值不足。
2. 最小可显示（选定）：真实初始化 + 纯色填充 + 周期换色。  
   优点：立即验证硬件与时序，复杂度可控，最符合当前学习目标。
3. 直接对齐 LVGL `flush_cb`：先做矩形 blit 等抽象。  
   优点：后续复用更顺。缺点：当前阶段会引入额外接口设计负担。

结论：采用方案 2，在不扩大范围的前提下拿到可见结果。

## 3. 架构与模块边界

### 3.1 模块划分

- `Core`：系统时钟、启动、中断。
- `Drivers/board_uart`：串口日志输出（延续 Stage 2 可观测性）。
- `Drivers/fsmc_lcd`（新增）：FSMC + LCD 最小驱动。
- `App/app_stage`：阶段行为分发，Stage 3 调用 LCD 驱动并执行演示逻辑。

### 3.2 新增文件

- `project/main/Drivers/Inc/fsmc_lcd.h`
- `project/main/Drivers/Src/fsmc_lcd.c`

### 3.3 现有文件修改

- `project/main/CMakeLists.txt`
- `project/main/Core/Inc/stm32f4xx_hal_conf.h`
- `project/main/App/Src/app_stage.c`

## 4. 关键数据流与调用流

1. `main()` 完成 `HAL_Init`、时钟与 GPIO 基础初始化后进入 `app_stage_init()`。
2. Stage 3 分支先初始化 UART（用于日志），再调用 `fsmc_lcd_init()`。
3. `fsmc_lcd_init()` 完成：
   - FSMC 时钟与 GPIO 复用配置。
   - SRAM/FSMC 时序初始化。
   - LCD 复位与寄存器初始化序列。
4. 初始化成功后执行 `fsmc_lcd_fill_color(color)` 显示首帧。
5. `app_stage_loop()` 基于 `HAL_GetTick()` 定时切换颜色并再次填充全屏。

## 5. 接口设计（Stage 3 最小集）

`fsmc_lcd.h` 暴露接口：

- `HAL_StatusTypeDef fsmc_lcd_init(void);`
- `void fsmc_lcd_fill_color(uint16_t rgb565);`

说明：

- `init` 负责底层总线与屏初始化，返回可判定状态。
- `fill_color` 负责最小可视验证，不包含通用绘图语义。

## 6. 硬件映射与一致性约束

- 以当前仓库主线硬件约定为准：FSMC Bank1 SRAM4，地址 `0x6C000000`。
- LCD 命令/数据地址映射在驱动内集中定义，避免散落在上层文件。
- 若参考工程使用了不同片选（例如 NE3 / `0x68000000`），迁移时统一转换到本项目约定并在注释明确标记。

## 7. 错误处理策略

- `fsmc_lcd_init()` 失败：
  - UART 输出错误码或阶段性错误信息。
  - 应用层进入可观察退化状态（保留 PF9 翻转或固定节拍日志）。
- 关键初始化步骤（FSMC init、LCD init 指令序列）使用返回值短路，禁止“失败继续跑”。

## 8. 构建与验证策略

构建验证：

- `cmake --preset stage3`
- `cmake --build --preset stage3`
- 回归构建：
  - `cmake --preset stage1 && cmake --build --preset stage1`
  - `cmake --preset stage2 && cmake --build --preset stage2`

板级验证（最小验收）：

- 上电后屏幕非黑屏，能看到纯色画面。
- 每隔固定周期切换颜色（如红/绿/蓝/白）。
- 串口可见 Stage 3 启动信息与运行节拍日志。

## 9. 测试范围与边界

本阶段测试关注：

- 编译可通过。
- LCD 链路可见输出。
- 阶段切换不影响旧阶段。

不在本阶段测试：

- LVGL 对接正确性。
- 动画时间基准与 UI 体验。
- 多任务调度与 protothreads 逻辑。

## 10. 与后续阶段的衔接

- Stage 4 将在本阶段已打通的 LCD 写入路径之上接入 LVGL `flush_cb`。
- Stage 5 再引入 protothreads，管理动画与任务节奏。
- 因此 Stage 3 的完成定义是“最小可显示 + 稳定可观测 + 构建不回归”，而非“接口一次性定型到最终形态”。
