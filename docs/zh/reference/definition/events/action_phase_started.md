[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **action_phase_started**

# givm::action_phase_started

定义于头文件 `<givm/definition.hpp>`

```cpp
struct action_phase_started;
```

本回合行动阶段开始的通知。响应者可以在玩家第一次选择行动前处理行动阶段开始时的效果。

## 注意

调用方应在进入 [`begin_action`](../commands/begin_action.md) 前确定首位行动玩家，广播沿用 `table.state().active_player`。本事件的所有响应会在首次 [`before_action`](before_action.md) 之前结算完毕。

## 示例

```cpp
#include <print>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>

#include <givm/givm.hpp>

struct observer_source
{
    using definition_category = givm::character_view;
    struct definition_type { int* count; };
    int* count;

    std::string_view name() const { return "observer"; }
    definition_type compile(givm::definition_compile_context&) const { return { count }; }

    static givm::program_entry<givm::action_phase_started> handle(
        const definition_type& definition, const givm::character_view&,
        givm::action_phase_started&, const givm::table&, givm::random_fn&)
    {
        ++*definition.count;
        return givm::program_entry<givm::action_phase_started>::null();
    }
};

int main()
{
    int count = 0;
    observer_source source{ &count };
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, std::tuple{}, givm::compile_mode::normal);
    givm::table table{};
    load_deck(table, library, givm::linked_deck{
        .characters = { ids.get_id<givm::character_view>("observer") }
    }, {});
    const auto observer = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    auto random_source = []() -> std::uint32_t { return 0; };
    givm::random_fn random{ random_source };
    givm::action_phase_started event{};
    const auto view = std::as_const(table)[observer.id()];
    library[view.definition_id()].handle<givm::action_phase_started>(view, event, table, random);
    std::println("响应次数: {}", count);
}
```

输出

```text
响应次数: 1
```
