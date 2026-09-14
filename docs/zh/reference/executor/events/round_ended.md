[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **round_ended**

# givm::round_ended

定义于头文件 `<givm/executor.hpp>`

```cpp
struct round_ended;
```

本回合结束的通知。响应者可以结算回合结束时生效的效果。

## 注意

在 [`end_round`](../instructions/end_round.md) 按约定接在双方结束声明之后执行时，广播中的 `table.state().active_player` 是首先结束的玩家，也就是下一回合的先手；`first_ended` 已清除，`round_number` 尚未递增。

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

    static givm::program_entry<givm::round_ended> handle(
        const definition_type& definition, const givm::character_view&,
        givm::round_ended&, const givm::table&, givm::random_fn&)
    {
        ++*definition.count;
        return givm::program_entry<givm::round_ended>::null();
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
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, std::tuple{});
    givm::table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{
        .characters = { ids.get_id<givm::character_view>("observer") }
    });
    const auto observer = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    auto random_source = []() -> std::uint32_t { return 0; };
    givm::random_fn random{ random_source };
    givm::round_ended event{};
    const auto view = std::as_const(table)[observer.id()];
    library[view.definition_id()].handle<givm::round_ended>(view, event, table, random);
    std::println("响应次数: {}", count);
}
```

输出

```text
响应次数: 1
```
