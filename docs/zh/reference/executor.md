[givm](../reference.md) / **执行**

# 执行

推进一场对局，并在需要输入、观察结果或结束对局时把控制权交回调用方。

|  |  |
| --- | --- |
| [`executor`](executor/executor.md) | 游戏对局的执行器 |
| [`execution_context`](executor/execution_context.md) | 指令决定继续、暂停或结束时使用的上下文 |
| [`random_fn`](executor/random_fn.md) | 取得调用方提供的随机值 |
| [`selector`](executor/selector.md) | 记录玩家对候选项的选择 |
| [`selection_capacity`](executor/selection_capacity.md) | 选择可表示的位置数量 |
| [`stage_t`](executor/stage_t.md) | 指令记录自身进度的数值类型 |

## 相关主题

|  |  |
| --- | --- |
| [指令](instructions.md) | 可以编排的游戏操作 |
| [事件](events.md) | 可以响应的游戏变化 |
| [驱动与输入](guides/driving.md) | 外层程序如何推进对局并提交输入 |
