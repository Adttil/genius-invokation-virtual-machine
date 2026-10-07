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
    static constexpr auto category = givm::definition_category::skill;
    struct definition_type { int* count; };
    int* count;

    std::string_view name() const { return "observer"; }
    definition_type compile(givm::definition_compile_context&) const { return { count }; }

    static givm::normal_effect handle(
        const definition_type& definition,
        givm::round_end_declared&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
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
    std::println("提供此事件的响应: {}", library.can_handle<givm::round_end_declared, givm::entity_category::skill>(id));
}
```

输出

```text
提供此事件的响应: true
```
