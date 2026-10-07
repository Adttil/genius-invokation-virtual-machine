[givm](../../reference.md) / [牌桌](../table.md) / **entity_id**

# givm::entity_id

定义于头文件 `<givm/table.hpp>`

```cpp
template<entity_category Category>
using entity_id = /* 对应的实体 ID 类 */;
```

由实体类别取得对应的 ID 类型。例如 `entity_id<entity_category::skill>` 就是 `skill_id`。它只负责类型映射；每种实体 ID 是独立的类，按其身份层级提供不同接口。

## 对应类型

| 类别 | ID 类 |
| --- | --- |
| `entity_category::player` | [`player_id`](player_id.md) |
| `entity_category::hand_card` | [`hand_card_id`](hand_card_id.md) |
| `entity_category::deck_card` | [`deck_card_id`](deck_card_id.md) |
| `entity_category::hand_card_status` | [`hand_card_status_id`](hand_card_status_id.md) |
| `entity_category::deck_card_status` | [`deck_card_status_id`](deck_card_status_id.md) |
| `entity_category::support` | [`support_id`](support_id.md) |
| `entity_category::summon` | [`summon_id`](summon_id.md) |
| `entity_category::combat_status` | [`combat_status_id`](combat_status_id.md) |
| `entity_category::character` | [`character_id`](character_id.md) |
| `entity_category::skill` | [`skill_id`](skill_id.md) |
| `entity_category::attachment` | [`attachment_id`](attachment_id.md) |
| `entity_category::reaction` | [`reaction_id`](reaction_id.md) |

`null` 没有对应的实体 ID 类。允许空值时使用 [`optional_entity_id`](optional_entity_id.md)；允许多个类别时使用 [`variant_entity_id`](variant_entity_id.md)。

各实体 ID 均占一个 `std::uint64_t`，当前实体索引位于低 32 位。技能、附属实体及卡牌状态还编码父级索引，目前该部分为 27 位；卡牌状态的低位索引指向全桌状态槽位。

默认构造保持平凡，未初始化对象须先赋值。非根级 ID 使用父级 ID 加末级索引构造；组合构造的编码前提只在 Debug 检查。ID 不携带牌桌身份；实际访问还须满足[实体的身份与访问](entity_access.md)中的存活和索引前提。
