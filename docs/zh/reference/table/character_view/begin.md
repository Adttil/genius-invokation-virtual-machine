[givm](../../../reference.md) / [牌桌](../../table.md) / [character_view](../character_view.md) / **begin**

# givm::character_view::begin

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto begin(this const auto& self);
```

取得这个角色的单实体范围起点。

## 参数

|  |  |
| --- | --- |
| `self` | 当前实体的只读视图 |

## 返回值

单实体范围的起始迭代器。实体有效时，解引用取得该实体的只读 [`character_view`](../character_view.md)；实体无效时与 `end()` 相等。

## 注意

范围依赖访问对象本身的存活。遍历有效实体时执行一次循环；遍历已移除实体时不执行循环。


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
        givm::genshin_impact::burning_flame_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    const auto definition = ids.get_id<givm::character_view>("示例");
    givm::table table{};
    load_deck(table, library, givm::linked_deck{ .characters = { definition } }, {});

    const givm::character_view view = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    for(const auto& item : view)
    {
        std::println("范围中的实体: {}", item.id() == view.id());
    }
}
```

输出

```text
范围中的实体: true
```
