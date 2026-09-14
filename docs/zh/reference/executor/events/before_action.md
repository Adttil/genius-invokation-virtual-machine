[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **before_action**

# givm::before_action

定义于头文件 `<givm/executor.hpp>`

```cpp
struct before_action;
```

当前行动玩家选择行动前的事件。响应者可以在玩家作出选择前处理自动触发的效果。

## 注意

`table.state().active_player` 表示即将行动的玩家。在正常行动机会中，若 `first_ended` 为 `true`，则对手已经宣告结束，本玩家可以继续行动。

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

    static givm::program_entry<givm::before_action> handle(
        const definition_type& definition, const givm::character_view&,
        givm::before_action&, const givm::table&, givm::random_fn&)
    {
        ++*definition.count;
        return givm::program_entry<givm::before_action>::null();
    }
};

int main()
{
    int count = 0;
    observer_source source{ &count };
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, std::tuple{});
    givm::table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{
        .characters = { ids.get_id<givm::character_view>("observer") }
    });
    const auto observer = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    auto random_source = []() -> std::uint32_t { return 0; };
    givm::random_fn random{ random_source };
    givm::before_action event{};
    const auto view = std::as_const(table)[observer.id()];
    library[view.definition_id()].handle<givm::before_action>(view, event, table, random);
    std::println("响应次数: {}", count);
}
```

输出

```text
响应次数: 1
```
