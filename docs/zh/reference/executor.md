[givm](../reference.md) / **执行**

# 执行

推进一场对局，并在需要输入、观察结果或结束对局时把控制权交回调用方。

## 类

|  |  |
| --- | --- |
| [`executor`](executor/executor.md) | 游戏对局的执行器 |
| [`execution_context`](executor/execution_context.md) | 核心指令推进对局时使用的上下文 |
| [`random_fn`](executor/random_fn.md) | 随机函数视图 |
| [`selector`](executor/selector.md) | 一名玩家对一组候选项的选择结果 |

## 类型别名

|  |  |
| --- | --- |
| [`stage_t`](executor/stage_t.md) | 指令结算进度的数值类型 |

## 常量

|  |  |
| --- | --- |
| [`selection_capacity`](executor/selection_capacity.md) | 单次选择可表示的位置数量 |

## [指令](executor/instructions.md)

核心提供的游戏操作类型。

## [事件](executor/events.md)

对局中的响应时机与事件数据。
