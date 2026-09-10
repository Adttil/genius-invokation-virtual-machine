[givm](../../reference.md) / [牌桌](../table.md) / **deck_card_status_id**

# givm::deck_card_status_id

定义于头文件 `<givm/table/entity_id.hpp>`

```cpp
struct deck_card_status_id;
```

牌库卡牌上的状态在一张牌桌中的身份。使用此 ID 可以通过 [`card_table::operator[]`](card_table/operator_subscript.md) 再次取得相应实体。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card_id` | [`deck_card_id`](deck_card_id.md) | 所属卡牌的 ID |
| `index` | `size_t` | 该实体的标识值；不表示筛除已移除实体后的排列位置 |

## 非成员函数

```cpp
friend constexpr bool operator==(deck_card_status_id, deck_card_status_id) = default;
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
    const example_source<givm::card_definition> card_source{};
    const example_source<givm::status_definition> status_source{};
    sources.add(card_source, status_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto card_definition = id_map.get_id<givm::card_definition>("示例");
    const auto definition = id_map.get_id<givm::status_definition>("示例");
    const auto card = player.add_deck_card(card_definition, {});
    const auto entity = card.add(definition, { .count = 3 });
    const givm::deck_card_status_id id = entity.id();
    std::println("通过 ID 取得定义: {}", table[id].definition().name());
}
```

输出

```text
通过 ID 取得定义: 示例
```
