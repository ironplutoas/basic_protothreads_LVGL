# Stage 1：最小系统与 HAL 启动

## 目标

- 理解从 `Reset_Handler` 到 `main()` 的启动路径
- 跑通 HAL 初始化与时钟配置
- 用 GPIO 周期翻转验证系统时基正常

## 代码入口

- `project/main/Core/Src/main.c`
- `project/main/App/Src/app_stage.c`（`APP_STAGE=1`）

## 构建

```bash
cmake --preset stage1
cmake --build --preset stage1
```

## 预期现象

- `PF9` 周期翻转（在星火一号上通常会体现为背光电平变化）
