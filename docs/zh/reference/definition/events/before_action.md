[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **before_action**

# givm::before_action

定义于头文件 `<givm/definition.hpp>`

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

    static givm::program_entry handle(
        const definition_type& definition, const givm::character_view&,
        givm::before_action&, givm::handle_context& context)
    {
        ++*definition.count;
        return {};
    }
};

int main()
{
    int count = 0;
    observer_source source{ &count };
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, std::tuple{}, givm::compile_mode::normal);
    const auto id = ids.get_id<givm::character_view>("observer");
    std::println("提供此事件的响应: {}", library.can_handle<givm::before_action, givm::character_view>(id));
}
```

输出

```text
提供此事件的响应: true
```
