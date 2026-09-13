[givm](../reference.md) / **执行**

# 执行

推进一场对局，并在需要输入、观察结果或结束对局时把控制权交回调用方。

## 类

|  |  |
| --- | --- |
| [`executor`](executor/executor.md) | 游戏对局的执行器 |
| [`execution_view`](executor/execution_view.md) | 一处对局执行现场的视图 |
| [`random_fn`](executor/random_fn.md) | 随机函数视图 |

## 枚举

|  |  |
| --- | --- |
| [`execution_state`](executor/execution_state.md) | 执行器交回控制权时的执行现场种类 |

## 常量

|  |  |
| --- | --- |
| [`selection_capacity`](executor/selection_capacity.md) | 单次选择可表示的位置数量 |

## [指令](executor/instructions.md)

核心提供的游戏操作类型。

## [事件](executor/events.md)

对局中的响应时机与事件数据。
