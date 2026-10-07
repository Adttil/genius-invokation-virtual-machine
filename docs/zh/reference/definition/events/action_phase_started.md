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
    static constexpr auto category = givm::definition_category::skill;
    struct definition_type { int* count; };
    int* count;

    std::string_view name() const { return "observer"; }
    definition_type compile(givm::definition_compile_context&) const { return { count }; }

    static givm::normal_effect handle(
        const definition_type& definition,
        givm::action_phase_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
    {
        ++*definition.count;
        return {};
    }
};

int main()
{
    int count = 0;
    observer_source source{ &count };
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    const auto id = ids.get_id<givm::definition_category::skill>("observer");
    std::println("提供此事件的响应: {}", library.can_handle<givm::action_phase_started, givm::entity_category::skill>(id));
}
```

输出

```text
提供此事件的响应: true
```
