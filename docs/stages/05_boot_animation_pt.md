# Stage 5：开机动画 + Protothreads

## 目标

- 在裸机系统中引入 protothreads（宏协程状态机）
- 把动画步骤拆成可维护的“伪线程”流程
- 建立“渲染任务 / 输入任务 / 业务任务”的协作方式

## 参考资源

- `resources/protothreads/pt.h`
- `resources/protothreads/pt-sem.h`

## 当前状态

- 已预留 `APP_STAGE=5` 切换位
- protothreads 实战示例待补充

## 构建

```bash
cmake --preset stage5
cmake --build --preset stage5
```
