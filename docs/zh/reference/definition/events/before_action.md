[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **before_action**

# givm::before_action

定义于头文件 `<givm/definition.hpp>`

```cpp
struct before_action;
```

当前行动玩家选择行动前的事件。响应者可以在玩家作出选择前处理自动触发的效果。

## 注意

`table.state().active_player` 表示即将行动的玩家。在正常行动机会中，若 `first_ended` 为 `true`，则对手已经宣告结束，本玩家可以继续行动。

本事件及其响应程序结束后，才检查出战角色的 [`this_prepared_skill_use`](this_prepared_skill_use.md) 响应能力。存在可执行的准备技能时自动执行该行动；否则提供玩家选择。快速行动后再次进入选择前也进行该检查。

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

    static givm::normal_effect handle(
        const definition_type& definition,
        givm::before_action&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
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
    const auto id = ids.get_id<givm::skill_view>("observer");
    std::println("提供此事件的响应: {}", library.can_handle<givm::before_action, givm::skill_view>(id));
}
```

输出

```text
提供此事件的响应: true
```
