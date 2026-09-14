[givm](../../../reference.md) / [定义](../../definition.md) / [program_entry](../program_entry.md) / **null**

# givm::program_entry::null

定义于头文件 `<givm/definition.hpp>`

```cpp
[[nodiscard]] static constexpr program_entry null() noexcept;
```

取得空入口，用于表示当前事件无需进入额外效果。

## 返回值

空的 `program_entry<TContext>`。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct support_source
{
    using definition_category = givm::support_view;

    std::string_view name() const { return "重投助手"; }
    int compile(givm::definition_compile_context&) const { return 1; }

    static givm::handler_program_entry_t<givm::dice_roll_preparation> handle(
        const int& extra_rerolls,
        const givm::support_view&,
        givm::dice_roll_preparation& event,
        const givm::card_table&,
        givm::random_fn&
    )
    {
        event.reroll_count[0] += extra_rerolls;
        return givm::handler_program_entry_t<givm::dice_roll_preparation>::null();
    }
};

int main()
{
    const support_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{ givm::start_round{} }
    );
    const auto id = ids.get_id<givm::support_view>("重投助手");

    givm::card_table table{};
    const givm::support_view entity = table[givm::player_id{ 0 }].add(id, { .count = 1 });
    auto random_source = []() -> std::uint32_t { return 0; };
    givm::random_fn random{ random_source };
    givm::dice_roll_preparation event{ .count = 8 };
    const auto entry = library.handle<givm::dice_roll_preparation>(id, entity, event, table, random);
    std::println("玩家 0 重投次数: {}", event.reroll_count[0]);
    std::println("无需额外结算: {}", entry.is_null());
}
```

输出

```text
玩家 0 重投次数: 2
无需额外结算: true
```
