[givm](../../../reference.md) / [牌桌](../../table.md) / [character_view](../character_view.md) / **player**

# givm::character_view::player

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr player_view player() const;
```

取得这个角色所属的玩家。

## 返回值

[`player_view`](../player_view.md)。


## 示例

```cpp
#include <cstdint>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct example_source
{
    using definition_category = givm::character_view;
    struct definition_type {};
    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .health = 10 };
    }
};

int main()
{
    example_source source{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    const auto definition = ids.get_id<givm::character_view>("示例");
    givm::table table{};
    load_deck(table, library, givm::linked_deck{ .characters = { definition } }, {});

    const givm::character_view view = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    std::println("所属玩家: {}", view.player().id().index);
}
```

输出

```text
所属玩家: 0
```
