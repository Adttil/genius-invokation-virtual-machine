[givm](../../reference.md) / [牌桌](../table.md) / **hand_card_status_entity**

# givm::hand_card_status_entity

定义于头文件 `<givm/table.hpp>`

```cpp
template<class TStorage>
class hand_card_status_entity;
```

附着在一张手牌上的状态，例如跟随该卡牌生效的持续影响。

## 模板参数

|  |  |
| --- | --- |
| `TStorage` | 由取得实体的牌桌决定；是否具有 const 限定决定实体能否修改。通常通过牌桌取得对象并使用 `auto`，无需显式指定此参数 |

## 成员类型

|  |  |
| --- | --- |
| `data_type` | 实体数据类型，随 `is_mutable` 保留只读限定 |
| `storage_type` | 实体访问所用的辅助类型 |
| `slot_type` | 卡牌状态数据所在的辅助类型 |

## 成员常量

|  |  |
| --- | --- |
| [`is_mutable`](hand_card_status_entity/is_mutable.md) | 实体是否允许修改 |

## 成员函数

|  |  |
| --- | --- |
| [`operator hand_card_status_view`](hand_card_status_entity/conversion.md) | 取得同一实体的只读视图 |
| [`is_valid`](hand_card_status_entity/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](hand_card_status_entity/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](hand_card_status_entity/size.md) | 取得单实体范围的元素数 |
| [`begin`](hand_card_status_entity/begin.md) | 取得单实体范围的起点 |
| [`end`](hand_card_status_entity/end.md) | 取得单实体范围的终点 |
| [`player`](hand_card_status_entity/player.md) | 取得所属玩家 |
| [`id`](hand_card_status_entity/id.md) | 取得实体 ID |
| [`definition`](hand_card_status_entity/definition.md) | 取得实体定义 |
| [`can_handle`](hand_card_status_entity/can_handle.md) | 判断是否具有某事件的处理程序 |
| [`state`](hand_card_status_entity/state.md) | 访问实体状态 |
| [`erase`](hand_card_status_entity/erase.md) | 移除实体 |
| [`card`](hand_card_status_entity/card.md) | 取得所属卡牌 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个手牌上的状态。访问对象的存活、只读转换和移除约定见[实体的身份与访问](entity_access.md)。

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
    const auto card = player.add_hand_card(card_definition, {});
    const auto entity = card.add(definition, { .count = 3 });
    std::println("状态计数: {}", entity.state().count);
}
```

输出

```text
状态计数: 3
```

## 参阅

|  |  |
| --- | --- |
| [`hand_card_status_view`](hand_card_status_view.md) | 对应的只读视图 |
| [`status_state`](status_state.md) | 该实体的状态 |
