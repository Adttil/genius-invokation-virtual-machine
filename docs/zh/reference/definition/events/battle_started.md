[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **battle_started**

# givm::battle_started

定义于头文件 `<givm/definition.hpp>`

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
    using definition_category = givm::skill_view;
    struct definition_type { int* count; };
    int* count;

    std::string_view name() const { return "observer"; }
    definition_type compile(givm::definition_compile_context&) const { return { count }; }

    static givm::normal_effect handle(
        const definition_type& definition,
        givm::battle_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
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
    std::println("提供此事件的响应: {}", library.can_handle<givm::battle_started, givm::skill_view>(id));
}
```

输出

```text
提供此事件的响应: true
```
