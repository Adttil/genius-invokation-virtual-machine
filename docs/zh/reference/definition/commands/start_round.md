[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **start_round**

# givm::start_round

定义于头文件 `<givm/definition.hpp>`

```cpp
struct start_round;
```

新回合的开始命令，负责回合数更新和双方元素骰的清空。超过回合上限时，结束对局并判定双方失败。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `max_rounds` | `std::uint32_t` | 允许进行的最大回合数，初始为 14 |

## 注意

先增加回合数，再判断是否超过上限；超过时以双方失败结束，不清空骰子。因此因回合数超限而结束时，牌桌回合数为 `max_rounds + 1`。

以 [`compile_mode::observed`](../../executor/compile_mode.md) 编译时，增加回合数后先返回 `execution_state::round_started`，随后推进才判断上限及清空骰子。

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
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 2 } }, givm::compile_mode::normal);
    givm::table table{};
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    std::println("回合数: {}", table.state().round_number);
    std::println("超过上限后双败: {}", execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
}
```

输出

```text
回合数: 3
超过上限后双败: true
```
