[givm](../../reference.md) / [牌桌](../table.md) / **character_view**

# givm::character_view

定义于头文件 `<givm/table.hpp>`

```cpp
class character_view;
```

对局中一名角色的只读视图。它承载角色的生命、充能、元素附着，以及角色拥有的技能和附属实体。

## 成员函数

|  |  |
| --- | --- |
| [`is_valid`](character_view/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](character_view/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](character_view/size.md) | 取得单实体范围的元素数 |
| [`begin`](character_view/begin.md) | 取得单实体范围的起点 |
| [`end`](character_view/end.md) | 取得单实体范围的终点 |
| [`player`](character_view/player.md) | 取得所属玩家 |
| [`id`](character_view/id.md) | 取得实体 ID |
| [`definition_id`](character_view/definition_id.md) | 取得实体的定义 ID |
| [`state`](character_view/state.md) | 访问实体状态 |
| [`skills`](character_view/skills.md) | 遍历角色的技能 |
| [`attachments`](character_view/attachments.md) | 遍历角色的附属实体 |
| [`has`](character_view/has.md) | 判断是否具有指定类别的装备 |
| [`get`](character_view/get.md) | 取得指定类别的当前装备 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个角色。视图的存活和移除约定见[实体的身份与访问](entity_access.md)。


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
    std::println("初始化后的生命值: {}", view.state().health);
}
```

输出

```text
初始化后的生命值: 10
```

## 参阅

|  |  |
| --- | --- |
| [`character_state`](character_state.md) | 该实体的状态 |
