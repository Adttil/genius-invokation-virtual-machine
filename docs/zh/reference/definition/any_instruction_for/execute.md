[givm](../../../reference.md) / [定义](../../definition.md) / [any_instruction_for](../any_instruction_for.md) / **execute**

# givm::any_instruction_for::execute

定义于头文件 `<givm/definition.hpp>`

```cpp
constexpr bool execute(card_table& table, execution_context& context, random_fn& random) const;
```

执行所保存的那条指令。用于编写会转交操作的自定义指令；普通调用方通过执行器推进对局。

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

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

struct invoke_instruction
{
    using context_type = void;
    const givm::any_instruction_for<void>* instruction;

    bool execute(givm::card_table& table, givm::execution_context& context, givm::random_fn& random) const
    {
        return instruction->execute(table, context, random);
    }
};

int main()
{
    const givm::any_instruction_for<void> round{ givm::start_round{ .max_rounds = 1 } };
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{ invoke_instruction{ &round } }
    );
    givm::card_table table{ library };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("完成回合数: {}", table.state().round_number);
}
```

输出

```text
完成回合数: 1
```
