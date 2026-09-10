[givm](../../../reference.md) / [定义](../../definition.md) / [program_entry](../program_entry.md) / **player_1_win**

# givm::program_entry::player_1_win

定义于头文件 `<givm/definition.hpp>`

```cpp
[[nodiscard]] static constexpr program_entry player_1_win() noexcept;
```

取得使对局以玩家 1 获胜结束的入口。取得入口本身不会结束对局，进入该入口后才产生终局结果。

## 返回值

表示玩家 1 获胜的非空终局入口。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

struct finish_game
{
    using context_type = void;
    givm::program_entry<void> entry;

    bool execute(givm::card_table&, givm::execution_context& context, givm::random_fn&) const
    {
        return context.enter(entry);
    }
};

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{},
        std::tuple{ finish_game{ givm::program_entry<void>::player_1_win() } }
    );
    givm::card_table table{ library };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("玩家 1 获胜: {}", execution.status() == givm::game_result::player_1_win);
}
```

输出

```text
玩家 1 获胜: true
```
