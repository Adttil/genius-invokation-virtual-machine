[givm](../../../reference.md) / [指令](../../instructions.md) / [end_round](../end_round.md) / **execute**

# givm::end_round::execute

定义于头文件 `<givm/executor/instructions/end_round.hpp>`

```cpp
bool execute(card_table& table, execution_context& context, random_fn& random) const;
```

[`card_table`](../../table/card_table.md)
[`execution_context`](../../executor/execution_context.md)
[`random_fn`](../../executor/random_fn.md)

关闭本回合并准备下一回合的先手。它应放在双方的结束声明已经结算完毕之后。

由 [`executor::execute_next`](../../executor/executor/execute_next.md) 在对局推进过程中调用。

## 参数

| | |
| --- | --- |
| `table` | 本场对局的牌桌 |
| `context` | 本指令所属的执行现场，须满足所属指令的事件或输入约定 |
| `random` | 本次执行可使用的随机源 |

## 返回值

返回 `true` 表示可以继续自动推进，`false` 表示应停止推进，包括等待外部输入或对局结束。对局结果由 [`executor::status`](../../executor/executor/status.md) 判断。

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
        std::tuple{ givm::end_round{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    table.state().active_player = givm::player_id{ 1 };
    table.state().first_ended = true;
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("下一回合由玩家 0 先手: {}", table.state().active_player == givm::player_id{ 0 });
    std::println("结束声明标记已清除: {}", !table.state().first_ended);
}
```

输出

```text
下一回合由玩家 0 先手: true
结束声明标记已清除: true
```
