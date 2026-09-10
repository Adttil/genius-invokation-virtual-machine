[givm](../../../reference.md) / [指令](../../instructions.md) / [start_round](../start_round.md) / **execute**

# givm::start_round::execute

定义于头文件 `<givm/executor/instructions/start_round.hpp>`

```cpp
bool execute(card_table& table, execution_context& context, random_fn&) const;
```

[`card_table`](../../table/card_table.md)
[`execution_context`](../../executor/execution_context.md)
[`random_fn`](../../executor/random_fn.md)

开始下一回合，更新回合数并清空双方的元素骰。超过回合上限时，结束对局并判定双方失败。

由 [`executor::execute_next`](../../executor/executor/execute_next.md) 在对局推进过程中调用。

## 参数

| | |
| --- | --- |
| `table` | 本场对局的牌桌 |
| `context` | 本指令所属的执行现场，须满足所属指令的事件或输入约定 |

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
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 2 } });
    givm::card_table table{ library };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("回合数: {}", table.state().round_number);
    std::println("达到上限后双败: {}", execution.status() == givm::game_result::both_loss);
}
```

输出

```text
回合数: 2
达到上限后双败: true
```
