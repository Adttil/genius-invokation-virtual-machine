[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **end_game**

# givm::execution_context::end_game

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr bool end_game(game_result result) noexcept;
```

以给定结果结束对局。

## 参数

|  |  |
| --- | --- |
| `result` | 终局结果，不能为 `game_result::no_result` |

## 返回值

`false`。

## 示例

```cpp
#include <print>
#include <cstdint>
#include <tuple>

#include <givm/givm.hpp>

struct effect
{
    using context_type = void;

    bool execute(givm::card_table& table, givm::execution_context& context, givm::random_fn&) const
    {
        return context.end_game(givm::game_result::player_0_win);
    }
};

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{ effect{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }
    );
    givm::card_table table{ library };
    givm::executor execution{};
    execution.enter_entry(library);
    auto random = []() -> std::uint32_t { return 0; };
    const bool continued = execution.execute_next(table, random);
    std::println("继续自动推进: {}", continued);
    std::println("玩家 0 获胜: {}", execution.status() == givm::game_result::player_0_win);
}
```

输出

```text
继续自动推进: false
玩家 0 获胜: true
```
