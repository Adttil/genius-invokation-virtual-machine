[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **enter_entry**

# givm::executor::enter_entry

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void enter_entry(const definition_library& library);
```
[`definition_library`](../../definition/definition_library.md)

准备按照 `library` 提供的游戏流程开始一场对局。

原有的待完成结算被丢弃。本函数不执行指令；通过 [`execute_next`](execute_next.md) 开始执行。

## 参数

| | |
| --- | --- |
| `library` | 本场对局使用的定义库 |

## 返回值

（无）

## 注意

本函数不修改牌桌。调用方应准备与该次执行配套的 [`card_table`](../../table/card_table.md)，并使用同一个 `library`。

## 示例

```cpp
#include <print>
#include <cstdint>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 1 } }
    );
    givm::card_table table{ library };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    table.state().round_number = 4;
    execution.enter_entry(library);
    std::println("牌桌回合数保持原值: {}", table.state().round_number);
    std::println("首个操作是洗牌: {}", library.instruction(execution.position()).is<givm::shuffle_deck>());
}
```

输出

```text
牌桌回合数保持原值: 4
首个操作是洗牌: true
```

## 参阅

| | |
| --- | --- |
| [`execute_next`](execute_next.md) | 完整执行当前位置的一次指令 |
| [`clear`](clear.md) | 清空执行栈 |
| [`position`](position.md) | 取得当前执行位置 |
