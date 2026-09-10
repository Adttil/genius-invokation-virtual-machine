[开发备忘](../../notes.md) / [事件分派](../event_dispatch.md)

# 指令、重入与调用现场

这里记录维护指令和广播推进器时共同依赖的现场约束。控制接口的用法见 [execution_context](../../reference/executor/execution_context.md)；各指令自身的重入分支和临时布局直接查阅[指令源码](../../../../include/givm/executor/instructions)。

## 干净退出与 Context

干净退出约束指令如何管理自己建立的临时现场：一项结算可以经历多次重入并进入响应子程序，彻底离开该结算时必须恢复开始时的栈形状。进入终局并保留现场是明确例外。一次调用仍完整、同步地执行当前指令，不是在 C++ 函数中途挂起；这一区别及动态程序的历史表达方式见[一次完整执行与一次完整结算](../fixed_program.md#一次完整执行与一次完整结算)。

Context 约束程序段可以读取哪一种事件语境。响应指令依赖的是仍在处理的事件及当前响应实体，不是某条前置指令遗留的数据。干净指令退出时不破坏调用者的 Context，因而同一响应程序中的后续指令仍可使用它；事件结算已经彻底退出后，再接一条响应指令不能重获该事件。伤害与抵挡的具体反例见同一[固定程序说明](../fixed_program.md#一次完整执行与一次完整结算)。

这两个约束不能互相替代。Context 相容性在编译入口检查，擦除后的指令不保存运行期 Context 标签；能通过检查不证明实现正确恢复了现场。临时 frame 的建立和撤销仍要由指令自身配对维护。

## 重入时必须保留的现场

指令对象只保存编译时确定的操作数；运行中的事件、输入、游标和阶段保存在栈中，持续存在的规则状态保存在 table。指令存储的尺寸与复制条件见[指令存储的实现约束](../fixed_program.md#指令存储的实现约束)。

当前活动 frame 以 `stage_t` 结尾。stage 没有独立类型信息，须与当前指令共同解释。保留当前位置等待或进入子程序以前，指令必须先把栈调整为下一次重入可以解释的状态；不能依赖本次调用的局部变量在返回后继续存在。

无 Context 依赖只表示不读取调用者的事件或费用提交数据。这样的指令仍从 executor 获得活动 frame，仍受阶段和干净退出约束，不能因此在空栈上直接调用其执行入口。

## 子程序返回的条件

当前 activation 的内部布局为：

```cpp
frame<execution_context::return_info, stage_t>
```

进入子程序时，`return_info::return_position` 保存调用者当前指令的位置，新的 activation stage 初始化为零。返回时恢复这条原指令，由它根据保留的进度继续原结算；恢复位置不能自行加一，否则会跳过调用者的收尾或剩余广播。

子程序中的指令完成后，应撤销自己追加的临时 frame，保留 activation 及其下方的调用 Context。向下一条指令推进时，当前 stage 归零，使下一条指令从自己的初始状态开始。`return_from_subroutine()` 在弹出 activation 前断言其 stage 为零；临时 frame 未清理或阶段未归零都会破坏返回条件。

返回位置先复制到局部值，再弹出 activation，避免在移除该 frame 后继续读取借出的引用。内部返回和跳转如何由 executor 连续收束，见[程序的内部连接](../fixed_program.md#程序的内部连接)。

## 控制操作后的栈借用

`frame_view` 以及从中取得的 event、cursor、handler 等引用都借用栈存储。追加 frame 可能触发扩容；弹出 frame 后，其中内容不再属于活动现场，也不能继续使用旧借用。移动或交换栈后，也不能继续把已有借用当作原执行现场。详细的布局和搬迁机制见[栈布局备忘](../stack_layout.md)。

进入子程序本身就会追加 activation，因此也是可能使现有栈 view 失效的操作。`continue_broadcast` 在调用 handler 前保存当前响应实体并推进游标，取得非空入口后进入子程序并立即返回；它不在进入后继续使用先前借出的 event、cursor 或 handler。响应结束后再次推进广播时，重新从栈取得这些对象。

修改这类推进器时，应检查每次可能扩容或移除 frame 的操作之后，是否还读取了操作之前借出的引用。仅仅“仍在同一次指令调用中”不能保证引用有效。

实现核对位置：[executor.hpp](../../../../include/givm/executor/executor.hpp)、[broadcast.hpp](../../../../include/givm/executor/broadcast.hpp) 与 [stack.hpp](../../../../include/givm/utils/stack.hpp)。
