[givm](../../reference.md) / [牌桌](../table.md) / **attachment_id**

# givm::attachment_id

定义于头文件 `<givm/table/entity_id.hpp>`

```cpp
struct attachment_id;
```

角色附属实体在一张牌桌中的身份。使用此 ID 可以通过 [`card_table::operator[]`](card_table/operator_subscript.md) 再次取得相应实体。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `character_id` | [`character_id`](character_id.md) | 所属角色的 ID |
| `index` | `size_t` | 该实体的标识值；不表示筛除已移除实体后的排列位置 |

## 非成员函数

```cpp
friend constexpr bool operator==(attachment_id, attachment_id) = default;
```

比较各成员是否相等；比较不检查实体是否尚未移除。

## 注意

ID 本身不包含牌桌身份。清理实体后，原有 ID 可能失效；详见[实体的身份与访问](entity_access.md)。

## 示例

```cpp
#include <print>
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
    const example_source<givm::character_view> character_source{};
    const example_source<givm::attachment_view> attachment_source{};
    sources.add(character_source, attachment_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto character_definition = id_map.get_id<givm::character_view>("示例");
    const auto definition = id_map.get_id<givm::attachment_view>("示例");
    const auto character = player.add(character_definition, { .max_health = 10, .health = 10 });
    const auto entity = character.add(definition, { .count = 3 });
    const givm::attachment_id id = entity.id();
    std::println("通过 ID 取得定义: {}", library[table[id].definition_id()].name());
}
```

输出

```text
通过 ID 取得定义: 示例
```
