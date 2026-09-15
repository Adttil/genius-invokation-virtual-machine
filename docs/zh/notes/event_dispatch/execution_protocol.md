[开发备忘](../../notes.md) / [事件分派](../event_dispatch.md)

# 指令、重入与调用现场

这里记录 command 编译和广播推进器共同依赖的内部约束。公开观察与输入见[执行观察与输入](../execution_observation.md)。

## 干净退出与 Context

干净退出作用于整个 command：其编译出的内部指令可以通过临时帧配合，但完整结束时必须恢复进入 command 前的栈形状，不向相邻 command 留下额外协议。进入终局时旧现场被逻辑废弃，是明确例外。每次 execute 调用仍然同步完成，不保存尚未返回的 C++ 调用。

响应 command 读取的是仍在结算的事件及当前响应实体，而不是前置 command 遗留的数据。同一响应程序中的后续 command 可以继续使用调用者 Context；事件结算完全结束后，再接一条响应 command 不能重获该 Context。

Context 相容性在编译入口检查；编译后不保存运行期 Context 标签。这个检查不能证明 execute 的帧建立和撤销正确，完整 command 的清理仍由实现负责。

## 重入时必须保留的现场

静态操作数保存在不可变字节流中；事件、输入、循环游标和其他运行数据保存在栈中，持久规则状态保存在 table。恢复点由执行位置指向对应 execute 函数，栈不再要求 stage 后缀，也不按 stage 进行第二次分派。

暂停或进入子程序以前，execute 必须把执行位置与栈调整为下次恢复所期望的状态，不能依赖本次函数的局部变量继续存在。无 Context 依赖的 command 不要求虚构根帧；其自己的输入或广播帧仍须正常建立和清理。

## 子程序返回的条件

activation 只保存返回位置：

```cpp
frame<execution_context::return_info>
```

`enter(entry)` 保存调用时的当前执行位置。需要恢复同一广播推进器时保留执行位置；需要在响应后清理付款帧等场合，先把执行位置设置为相应恢复入口再调用 enter。返回只恢复保存的执行位置，不能统一加一个槽位。

响应程序的 command 正常完成后撤销自身临时帧，保留 activation 及调用者 Context。响应末尾的返回函数读取 `return_info`、弹出完整 activation，并跳回保存的字节偏移，不检查或重置 stage。

返回位置先复制到局部值，再弹出 activation，避免读取已经移除帧中的引用。内部跳转和返回由调度循环继续执行，不向外层报告独立状态。

## 控制操作后的栈借用

`frame_view` 以及从中取得的 event、cursor、handler 等引用都借用栈存储。追加 frame 可能触发扩容；弹出 frame 后，其中内容不再属于活动现场，也不能继续使用旧借用。移动或交换栈后，也不能继续把已有借用当作原执行现场。详细的布局和搬迁机制见[栈布局备忘](../stack_layout.md)。

进入子程序本身就会追加 activation，因此也是可能使现有栈 view 失效的操作。`continue_broadcast` 在调用 handler 前保存当前响应实体并推进游标，取得非空入口后进入子程序并立即返回，之后不再使用先前借出的 event、cursor 或 handler。响应结束后再次推进广播时，重新从栈取得这些对象。

修改这类推进器时，应检查每次可能扩容或移除 frame 的操作之后，是否还读取了操作之前借出的引用。仅仅“仍在同一次指令调用中”不能保证引用有效。

实现核对位置：[executor.hpp](../../../../include/givm/executor/executor.hpp)、[broadcast.hpp](../../../../include/givm/executor/broadcast.hpp) 与 [stack.hpp](../../../../include/givm/utils/stack.hpp)。
