# Stage 2：UART 调试输出

## 目标

- 学会从 Cube/HAL 初始化代码中抽取最小 UART 驱动
- 通过串口日志建立“软件可观测性”

## 代码入口

- `project/main/Drivers/Src/board_uart.c`
- `project/main/App/Src/app_stage.c`（`APP_STAGE=2`）

## 构建

```bash
cmake --preset stage2
cmake --build --preset stage2
```

## 预期串口输出

- 启动 banner：`[stage2] uart printf is ready.`
- 心跳日志：`[stage2] heartbeat`
