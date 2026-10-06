[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **card_equipment_target_validation**

# givm::card_equipment_target_validation

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_equipment_target_validation;
```

手牌能否在当前情况下装备给指定角色的查询。例如，为所属角色的天赋牌减费的圣遗物，可通过本查询判断该天赋是否适用于此角色，以及是否满足必须出战的限制。

## 成员类型

| | |
| --- | --- |
| `result_t` | `bool` |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | [`hand_card_view`](../../table/hand_card_view.md) | 待判断的手牌。 |
| `character` | [`character_view`](../../table/character_view.md) | 待装备的角色。 |

## 注意

本查询由 `card_definition` 提供，符合装备对象要求时返回 `true`，否则返回 `false`；缺少源查询时，[默认方法](../query_default.md)返回 `false`。两个实体须属于同一牌桌。定义源自行判断所属玩家、角色定义、存活条件及前后台限制，不根据行动速度推断能否装备给后台角色。

天赋牌应声明对应角色的名称硬依赖，在 `compile` 中解析并保存角色定义 ID，查询时与 `character.definition_id()` 比较。角色是否出战等信息由角色 view 取得，无需为每种角色另设标签。实际出牌的 [`card_target_validation`](card_target_validation.md) 可以共用定义源内部的适用性判断。

本查询只判断装备对象，不检查支付或其他完整用牌条件，不提交目标选择，也不返回效果入口。减费响应可通过 [`handle_context::query`](../../executor/handle_context/query.md) 按需调用；动态定义源按现有协议提供 `can_query` 和 `query`。

费用预览已经在 [`cost_of_card`](../events/cost_of_card.md) 中给出完整目标。减费响应可先确认本实体与已选目标的关系，再使用本查询检查装备适用性；费用及确认时消耗的次数可以依赖该目标。本查询不保证合法装备对象唯一。
