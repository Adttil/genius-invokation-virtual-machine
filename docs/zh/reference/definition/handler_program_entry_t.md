[givm](../../reference.md) / [定义](../definition.md) / **handler_program_entry_t**

# givm::handler_program_entry_t

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TEvent>
using handler_program_entry_t = program_entry<handler_program_context_t<TEvent>>;
```

一个事件的响应函数所返回的效果入口类型。用事件类型指定它即可，无须另行决定效果对应的 context。

## 模板参数

|  |  |
| --- | --- |
| `TEvent` | 所响应的事件类型 |

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct character_source
{
    using definition_category = givm::character_view;

    std::string_view name() const { return "重投助手"; }
    int compile(givm::definition_compile_context&) const { return 1; }

    static givm::handler_program_entry_t<givm::dice_roll_preparation> handle(
        const int& extra_rerolls,
        const givm::character_view&,
        givm::dice_roll_preparation& event,
        const givm::table&,
        givm::random_fn&
    )
    {
        event.reroll_count[0] += extra_rerolls;
        return givm::handler_program_entry_t<givm::dice_roll_preparation>::null();
    }
};

int main()
{
    const character_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
    const auto id = ids.get_id<givm::character_view>("重投助手");

    givm::table table{};
    load_deck(table, library, givm::linked_deck{ .characters = { id } }, {});
    const auto entity = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
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

## 参阅

|  |  |
| --- | --- |
| [`program_entry`](program_entry.md) | 响应程序入口与空入口 |
