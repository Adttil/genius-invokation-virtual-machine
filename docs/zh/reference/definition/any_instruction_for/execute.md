[givm](../../../reference.md) / [定义](../../definition.md) / [any_instruction_for](../any_instruction_for.md) / **execute**

# givm::any_instruction_for::execute

定义于头文件 `<givm/definition.hpp>`

```cpp
constexpr bool execute(card_table& table, execution_context& context, random_fn& random) const;
```

执行所保存的核心指令，使用执行器提供的当前执行现场。调用方通过 [`executor::execute_next`](../../executor/executor/execute_next.md) 推进对局。

## 参数

|  |  |
| --- | --- |
| `table` | 本次操作作用的牌桌 |
| `context` | 满足该指令 context 要求的当前执行现场 |
| `random` | 本次执行使用的随机源 |

## 返回值

所保存指令的执行结果；`true` 表示可以自动继续，`false` 表示停止自动推进。

## 注意

构造时的类型检查不替代运行时的 context 条件。调用方须处于该指令允许的事件或对局环境中。
