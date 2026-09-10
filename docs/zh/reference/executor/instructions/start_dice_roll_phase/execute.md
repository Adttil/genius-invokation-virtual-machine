[givm](../../../../reference.md) / [执行](../../../executor.md) / [指令](../../instructions.md) / [start_dice_roll_phase](../start_dice_roll_phase.md) / **execute**

# givm::start_dice_roll_phase::execute

定义于头文件 `<givm/executor/instructions/start_dice_roll_phase.hpp>`

```cpp
bool execute(card_table& table, execution_context& context, random_fn& random) const;
```

[`card_table`](../../../table/card_table.md)
[`execution_context`](../../execution_context.md)
[`random_fn`](../../random_fn.md)

为双方开始投骰阶段，先处理固定骰子和重投次数，再生成双方骰子并接受重投选择。

由 [`executor::execute_next`](../../executor/execute_next.md) 在对局推进过程中调用。

## 参数

| | |
| --- | --- |
| `table` | 本场对局的牌桌 |
| `context` | 本次指令执行的上下文；调用须满足本指令的事件或输入约定 |
| `random` | 本次执行可使用的随机源 |

## 返回值

返回 `true` 表示可以继续自动推进，`false` 表示应停止推进，包括等待外部输入或对局结束。对局结果由 [`executor::status`](../../executor/status.md) 判断。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::start_dice_roll_phase{ .count = 8 } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    // 双方保留首次投出的骰子，不进行重投。
    for(int submission = 0; submission < 2; ++submission)
    {
        auto&& [input, preserved] = execution.stack().top<givm::selector, givm::stage_t>();
        input.selected.reset();
        while(execution.execute_next(table, random)) {}
    }
    std::println("玩家 0 的骰子数量: {}", table[givm::player_id{ 0 }].state().dice.total());
    std::println("玩家 1 的骰子数量: {}", table[givm::player_id{ 1 }].state().dice.total());
}
```

输出

```text
玩家 0 的骰子数量: 8
玩家 1 的骰子数量: 8
```
