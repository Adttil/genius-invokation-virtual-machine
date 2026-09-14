[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **round_end_declared**

# givm::round_end_declared

定义于头文件 `<givm/executor.hpp>`

```cpp
struct round_end_declared;
```

玩家宣布本回合结束的通知。响应者可以在另一名玩家继续行动或本回合关闭前处理结束声明。

## 注意

广播期间，`table.state().active_player` 仍表示宣告者。第一次宣告结束时，`first_ended` 已设为 `true`，行动方会在响应结束后切换。双方的声明都结算完毕后，是否紧接 [`end_round`](../instructions/end_round.md) 由调用方提供的流程决定。

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

    static givm::program_entry<givm::round_end_declared> handle(
        const definition_type& definition, const givm::character_view&,
        givm::round_end_declared&, const givm::card_table&, givm::random_fn&)
    {
        ++*definition.count;
        return givm::program_entry<givm::round_end_declared>::null();
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
    givm::round_end_declared event{};
    const auto view = std::as_const(table)[observer.id()];
    library[view.definition_id()].handle<givm::round_end_declared>(view, event, table, random);
    std::println("响应次数: {}", count);
}
```

输出

```text
响应次数: 1
```
