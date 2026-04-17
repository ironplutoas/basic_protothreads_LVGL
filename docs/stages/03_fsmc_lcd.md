# Stage 3：FSMC LCD 驱动

## 目标

- 接入 ST7789（FSMC 模式）
- 验证地址线、片选线与命令/数据写入流程
- 建立 LCD 驱动与应用层之间的抽象边界

## 当前状态

- 已预留 `APP_STAGE=3` 切换位
- 本阶段驱动文件尚未落地（下一步实现）

## 构建

```bash
cmake --preset stage3
cmake --build --preset stage3
```
