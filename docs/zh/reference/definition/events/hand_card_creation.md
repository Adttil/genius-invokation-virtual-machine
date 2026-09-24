[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **hand_card_creation**

# givm::hand_card_creation

定义于头文件 `<givm/definition.hpp>`

```cpp
struct hand_card_creation
{
    player_id player;
    definition_id<card_definition> definition;
};
```

[`create_hand_card`](../commands/create_hand_card.md) 的动态输入，指定接收新牌的玩家及新牌的定义。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 接收新牌的玩家 |
| `definition` | `definition_id<card_definition>` | 新牌的定义 |

## 注意

玩家与定义必须有效。新牌的属性由其定义的 [`card_initial_state`](../queries/card_initial_state.md) 决定。

本类型仅作为命令输入，不广播，也不属于任何实体的订阅事件。生成成功后的通知为 [`hand_card_added`](hand_card_added.md)。
