[givm](../../../reference.md) / [定义](../../definition.md) / [effect](../effect.md) / **operator== (effect<Category>)**

# givm::operator== (effect<Category>)

定义于头文件 `<givm/definition.hpp>`

```cpp
friend constexpr bool operator==(effect<Category>, effect<Category>) noexcept = default;
```

比较两个入口是否选择了相同的后续效果。

## 参数

|  |  |
| --- | --- |
| 两个操作数 | 属于同一定义库的入口或空入口 |

## 返回值

入口相等时返回 `true`，否则返回 `false`。

## 示例

```cpp
#include <cstdint>
#include <utility>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct result_source
{
    static constexpr auto category = givm::definition_category::support;
    using entry_type = givm::normal_effect;

    std::string_view name() const { return "终局判定"; }
    entry_type compile(givm::definition_compile_context& context) const
    {
        const auto first = context.add_normal_effect(
            std::tuple{ givm::settle{}, givm::end_game{ .result = givm::game_result::player_0_win } });
        const auto second = context.add_normal_effect(
            std::tuple{ givm::settle{}, givm::end_game{ .result = givm::game_result::player_1_win } });
        std::println("选择同一效果: {}", first == second);
        std::println("默认入口为空: {}", entry_type{} == entry_type::null());
        return first;
    }
    static givm::normal_effect handle(
        const entry_type& entry, givm::round_ended&,
        givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
    {
        return context.invoke(entry);
    }
};

int main()
{
    const result_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{}, givm::settle{} }, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
}
```

输出

```text
选择同一效果: false
默认入口为空: true
```
