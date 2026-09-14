[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **add**

# givm::player_entity::add

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr support_entity<TStorage> add(
    definition_id<support_view> definition_id, const support_state& state
) const requires is_mutable;

constexpr summon_entity<TStorage> add(
    definition_id<summon_view> definition_id, const summon_state& state
) const requires is_mutable;

constexpr combat_status_entity<TStorage> add(
    definition_id<combat_status_view> definition_id, const combat_status_state& state
) const requires is_mutable;

constexpr character_entity<TStorage> add(
    definition_id<character_view> definition_id, const character_state& state
) const requires is_mutable;
```

为玩家加入一个角色、支援、召唤物或出战状态。实体所属种类由定义 ID 的类型决定。

## 参数

|  |  |
| --- | --- |
| [`definition_id`](../definition_id.md) | 配套定义库中的相应种类定义 ID |
| `state` | 新实体的初始状态 |

## 返回值

新加入的相应实体访问对象。

## 注意

该操作不检查容量上限或合并同类实体，不运行创建事件；增加角色也不自动为其添加技能或设置出战角色。

## 示例

```cpp
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

template<class Category>
struct example_source
{
    using definition_category = Category;
    struct definition_type {};

    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    givm::definition_source_library sources{};
    const example_source<givm::summon_view> summon_source{};
    sources.add(summon_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::summon_view>("示例");
    const auto entity = player.add(definition, { .count = 3 });
    std::println("召唤物计数: {}", entity.state().count);
    std::println("召唤物数量: {}", std::ranges::distance(player.summons()));
}
```

输出

```text
召唤物计数: 3
召唤物数量: 1
```
