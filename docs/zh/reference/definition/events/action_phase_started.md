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
    using definition_category = givm::skill_view;
    struct definition_type { int* count; };
    int* count;

    std::string_view name() const { return "observer"; }
    definition_type compile(givm::definition_compile_context&) const { return { count }; }

    static givm::program_entry handle(
        const definition_type& definition, const givm::skill_view&,
        givm::action_phase_started&, givm::handle_context& context)
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
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, std::tuple{}, givm::compile_mode::normal);
    const auto id = ids.get_id<givm::skill_view>("observer");
    std::println("提供此事件的响应: {}", library.can_handle<givm::action_phase_started, givm::skill_view>(id));
}
```

输出

```text
提供此事件的响应: true
```
