[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **round_end_declared**

# givm::round_end_declared

定义于头文件 `<givm/definition.hpp>`

```cpp
struct round_end_declared;
```

玩家宣布本回合结束的通知。响应者可以在另一名玩家继续行动或本回合关闭前处理结束声明。

## 注意

广播期间，`table.state().active_player` 仍表示宣告者。第一次宣告结束时，`first_ended` 已设为 `true`，行动方会在响应结束后切换。双方的声明都结算完毕后，是否紧接 [`end_round`](../commands/end_round.md) 由调用方提供的流程决定。

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
    using definition_category = givm::skill_view;
    struct definition_type { int* count; };
    int* count;

    std::string_view name() const { return "observer"; }
    definition_type compile(givm::definition_compile_context&) const { return { count }; }

    static givm::program_entry handle(
        const definition_type& definition, const givm::skill_view&,
        givm::round_end_declared&, givm::handle_context& context)
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
    const auto id = ids.get_id<givm::skill_view>("observer");
    std::println("提供此事件的响应: {}", library.can_handle<givm::round_end_declared, givm::skill_view>(id));
}
```

输出

```text
提供此事件的响应: true
```
