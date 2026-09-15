[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **handle**

# givm::definition_library::definition_view::handle

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TEvent, class TView>
handler_program_entry_t<TEvent> handle(
    const TView& entity,
    TEvent& event,
    const table& card_table,
    random_fn& random
) const;
```

请求该定义为一个实体响应当前事件。

## 模板参数

|  |  |
| --- | --- |
| `TEvent` | 当前事件类型 |
| `TView` | 该定义类别对应的只读实体 view |

## 参数

|  |  |
| --- | --- |
| `entity` | 响应事件的实体，只读 view 须属于该定义类别 |
| `event` | 要响应的事件，可修改的成员用于反馈本次事件的调整 |
| `card_table` | 事件发生的牌桌，须使用本定义库 |
| `random` | 响应过程中使用的随机源 |

## 返回值

需要继续执行的效果入口；空入口表示没有后续命令。本函数不执行返回的效果入口。

## 注意

[`can_handle<TEvent, TView>()`](can_handle.md) 须为 `true`。实体、牌桌与定义视图须使用配套定义库。

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
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .characters = { id } });
    const auto entity = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    auto random_source = []() -> std::uint32_t { return 0; };
    givm::random_fn random{ random_source };
    givm::dice_roll_preparation event{ .count = 8 };
    const auto entry = library[id].handle<givm::dice_roll_preparation>(entity, event, table, random);
    std::println("玩家 0 重投次数: {}", event.reroll_count[0]);
    std::println("无需额外结算: {}", entry.is_null());
}
```

输出

```text
玩家 0 重投次数: 2
无需额外结算: true
```
