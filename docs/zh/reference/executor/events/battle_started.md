[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **battle_started**

# givm::battle_started

定义于头文件 `<givm/executor.hpp>`

```cpp
struct battle_started;
```

对局首次进入战斗的通知。它用于处理战斗开始时生效的角色能力、装备或其他效果。

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

    static givm::program_entry<givm::battle_started> handle(
        const definition_type& definition, const givm::character_view&,
        givm::battle_started&, const givm::card_table&, givm::random_fn&)
    {
        ++*definition.count;
        return givm::program_entry<givm::battle_started>::null();
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
    givm::card_table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{
        .characters = { ids.get_id<givm::character_view>("observer") }
    });
    const auto observer = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    auto random_source = []() -> std::uint32_t { return 0; };
    givm::random_fn random{ random_source };
    givm::battle_started event{};
    const auto view = std::as_const(table)[observer.id()];
    library[view.definition_id()].handle<givm::battle_started>(view, event, table, random);
    std::println("响应次数: {}", count);
}
```

输出

```text
响应次数: 1
```
