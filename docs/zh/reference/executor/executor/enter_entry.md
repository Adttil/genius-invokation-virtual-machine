[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **enter_entry**

# givm::executor::enter_entry

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void enter_entry(const definition_library& library);
```
[`definition_library`](../definition_library.md)

准备按照 `library` 提供的游戏流程开始一场对局。

原有的待完成结算被丢弃。本函数不执行指令；通过 [`run`](run.md) 或 [`step`](step.md) 开始推进。

## 参数

| | |
| --- | --- |
| `library` | 本场对局使用的定义库 |

## 返回值

（无）

## 注意

本函数不修改牌桌。调用方应准备与该次执行配套的 [`table`](../../table/table.md)，并在每次推进时显式传入配套的 `library`。本函数不保存定义库的指针或引用。

## 示例

```cpp
#include <print>
#include <cstdint>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 1 } }
    );
    givm::table table{};
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    std::println("牌桌回合数保持原值: {}", table.state().round_number);
    const auto state = execution.run(library, table, random);
    std::println("随后推进至终局: {}", state == givm::execution_state::finished);
}
```

输出

```text
牌桌回合数保持原值: 0
随后推进至终局: true
```

## 参阅

| | |
| --- | --- |
| [`run`](run.md) | 推进至输入现场或终局 |
| [`step`](step.md) | 推进至下一处观察或输入现场 |
