[givm](../reference.md) / **执行**

# 执行

将定义源与游戏流程编译为对局规则，推进一场对局，并在需要输入、观察结果或结束对局时把控制权交回调用方。

## 类

### 编译与定义库

|  |  |
| --- | --- |
| [`definition_compile_context`](executor/definition_compile_context.md) | 单项定义的编译上下文 |
| [`definition_library`](executor/definition_library.md) | 对局使用的定义与流程 |
| [`program_entry`](executor/program_entry.md) | 响应效果的入口 |
| [`any_instruction_for`](executor/any_instruction_for.md) | 同一种 context 下的同构指令值 |

### 对局执行

|  |  |
| --- | --- |
| [`executor`](executor/executor.md) | 游戏对局的执行器 |
| [`execution_view`](executor/execution_view.md) | 一处对局执行现场的视图 |
| [`random_fn`](executor/random_fn.md) | 随机函数视图 |

## 函数

|  |  |
| --- | --- |
| [`compile`](executor/compile.md) | 编译选定定义与对局流程 |

## 概念

|  |  |
| --- | --- |
| [`instruction_compatible_with`](executor/instruction_compatible_with.md) | 指令与 context 相容的约束 |

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
