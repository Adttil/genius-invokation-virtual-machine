[givm](../../../reference.md) / [定义](../../definition.md) / [definition_library](../definition_library.md) / **handle**

# givm::definition_library::handle

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TEvent, class TDefinitionType, class TView>
handler_program_entry_t<TEvent> handle(
    definition_id<TDefinitionType> id,
    const TView& entity,
    TEvent& event,
    const card_table& table,
    random_fn& random
) const;
```

请求一个实体的定义响应当前事件。响应可以直接调整事件允许修改的内容，也可以返回需要继续执行的效果。

## 模板参数

|  |  |
| --- | --- |
| `TEvent` | 当前事件类型 |
| `TDefinitionType` | 由 ID 推导的定义类别 |
| `TView` | 该定义类别对应的只读实体 view |

## 参数

|  |  |
| --- | --- |
| `id` | 本定义库中的有效定义 ID |
| `entity` | 响应事件的实体，只读 view 须属于该定义类别 |
| `event` | 要响应的事件，可修改的成员用于反馈本次事件的调整 |
| `table` | 事件发生的牌桌，须使用本定义库 |
| `random` | 响应过程中使用的随机源 |

## 返回值

该事件下需要执行的 [`handler_program_entry_t`](../handler_program_entry_t.md)。空入口表示没有后续效果需要进入；本函数本身不执行返回入口对应的指令。

## 注意

对应的 [`can_handle<TEvent, TView>`](can_handle.md) 必须为 `true`。普通的事件分发由指令负责；直接调用本函数的调用者需要自行安排返回效果的执行。

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
