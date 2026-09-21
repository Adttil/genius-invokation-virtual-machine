[givm](../../../reference.md) / [定义](../../definition.md) / [program_entry](../program_entry.md) / **operator== (program_entry)**

# givm::operator== (program_entry)

定义于头文件 `<givm/definition.hpp>`

```cpp
friend constexpr bool operator==(program_entry, program_entry) noexcept = default;
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
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct result_source
{
    using definition_category = givm::support_view;
    using entry_type = givm::program_entry;

    std::string_view name() const { return "终局判定"; }
    entry_type compile(givm::definition_compile_context& context) const
    {
        const auto first = context.add_program(
            std::tuple{ givm::end_game{ .result = givm::game_result::player_0_win } });
        const auto second = context.add_program(
            std::tuple{ givm::end_game{ .result = givm::game_result::player_1_win } });
        std::println("选择同一效果: {}", first == second);
        std::println("默认入口为空: {}", entry_type{} == entry_type::null());
        return first;
    }
    static givm::program_entry handle(
        const entry_type& entry, const givm::support_view&, givm::round_ended&,
        givm::handle_context& context)
    {
        return context.invoke(entry);
    }
};

int main()
{
    const result_source source{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal);
}
```

输出

```text
选择同一效果: false
默认入口为空: true
```
