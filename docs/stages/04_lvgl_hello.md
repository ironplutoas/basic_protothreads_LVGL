# Stage 4：LVGL Hello World

## 目标

- 建立 LVGL tick 与 flush 回调
- 完成最小 UI（label/button）渲染
- 理解 `lv_conf.h` 的关键配置（颜色深度、分辨率、内存）

## 已实现内容

- 已完成最小通路：
  - `lv_init()`
  - 显示驱动 `flush_cb`（通过 FSMC LCD 区域写入）
  - Tick 递增与 `lv_timer_handler()` 调度
  - `Hello LVGL` 标签渲染
- 新增文件：
  - `project/main/App/Inc/lv_conf.h`
  - `project/main/App/Inc/stage4_lvgl.h`
  - `project/main/App/Src/stage4_lvgl.c`

## 构建

```bash
cmake --preset stage4
cmake --build --preset stage4
```

## 运行验证

- 烧录 Stage 4 固件后，屏幕中心应显示 `Hello LVGL`
- 串口应输出：`[stage4] lvgl init ok`

## 已知限制

- 本阶段只做最小 UI 通路，不含按钮交互与动画
- 下一阶段再引入 protothreads 与动画流程
