# Stage 5：开机动画 + Protothreads

## 目标

- 在裸机系统中引入 protothreads（宏协程状态机）
- 把动画步骤拆成可维护的“伪线程”流程
- 建立“渲染任务 / 输入任务 / 业务任务”的协作方式

## 参考资源

- `resources/protothreads/pt.h`
- `resources/protothreads/pt-sem.h`

## 已实现内容

- Stage5 入口已接入：
  - `project/main/App/Src/app_stage.c`
  - 当 `APP_STAGE==5` 时执行 `stage5_boot_pt_init()` / `stage5_boot_pt_loop()`
- 新增文件：
  - `project/main/App/Inc/stage5_boot_pt.h`
  - `project/main/App/Src/stage5_boot_pt.c`
- 动画实现方式：
  - 基于 LVGL 显示驱动最小通路（`lv_init` + `flush_cb` + `lv_timer_handler`）
  - 使用 protothreads 线程函数 `stage5_boot_anim_thread(struct pt *pt)`
  - 动画流程：`透明 -> 淡入 -> 停留 -> 淡出 -> 间隔 -> 循环`
- 时间基准：
  - 每一步等待由 `HAL_GetTick()` 计算，不依赖主循环执行频率
  - 避免“主循环变慢导致动画时长漂移”的问题

## 学习重点

- 为什么用 protothreads：
  - 用串行写法描述“多步骤异步流程”，比裸状态机 `switch-case` 更直观
  - 不引入 RTOS，也能把复杂时序拆成清晰流程
- 本阶段建议先读懂 3 个宏：
  - `PT_BEGIN / PT_END`
  - `PT_WAIT_UNTIL`
  - `PT_SCHEDULE`

## 构建

```bash
cmake --preset stage5
cmake --build --preset stage5
```

## 运行验证

- 烧录后屏幕应显示：
  - 黑底
  - `LVGL Boot` 文字做循环淡入淡出
  - `Protothreads` 提示文字常亮
- 串口应输出：
  - `[stage5] lvgl + protothreads init ok`

## 下一步可选增强

- 用 `PT_SPAWN` 拆分子线程（渲染线程 / 逻辑线程）
- 引入按键输入，把动画状态切换做成事件驱动
- 把固定步长动画升级为按目标时长插值
