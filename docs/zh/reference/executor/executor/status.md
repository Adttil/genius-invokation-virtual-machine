[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **status**

# givm::executor::status

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr game_result status() const noexcept;
```
[`game_result`](../../enums/game_result.md)

查询执行器记录的对局结果。

## 返回值

返回下列 [`game_result`](../../enums/game_result.md) 值之一。

| | |
| --- | --- |
| `game_result::no_result` | 尚无对局结果 |
| `game_result::player_0_win` | 玩家 0 获胜 |
| `game_result::player_1_win` | 玩家 1 获胜 |
| `game_result::both_loss` | 双方均告负 |

## 注意

`no_result` 仅表示尚无对局结果；默认构造或等待输入时也会返回此值。

[`execute_next`](execute_next.md) 返回 `false` 表示请求暂停自动推进。通过 `status()` 可以进一步区分是否已经结束对局。

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
    std::println("执行前尚无结果: {}", execution.status() == givm::game_result::no_result);
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("结束后双方告负: {}", execution.status() == givm::game_result::both_loss);
}
```

输出

```text
执行前尚无结果: true
结束后双方告负: true
```

## 参阅

| | |
| --- | --- |
| [`execute_next`](execute_next.md) | 完整执行当前位置的一次指令 |
| [`enter_entry`](enter_entry.md) | 准备开始一场对局 |
| [`position`](position.md) | 取得当前执行位置 |
