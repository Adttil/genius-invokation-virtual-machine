[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **discard_hand_card**

# givm::discard_hand_card

定义于头文件 `<givm/definition.hpp>`

```cpp
struct discard_hand_card
{
    using input_type = discard_hand_card_input;

    relative_player player = relative_player::self;
    definition_id<card_definition> definition{};
};
```

舍弃一张手牌，先执行该牌自身的舍弃效果，再发送全场通知。元素调和、打出牌和手牌溢出的移除不属于舍弃。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`discard_hand_card_input`](../command_inputs/discard_hand_card_input.md)，动态模式下的输入类型 |

## 参数形式

指定 `definition` 时，从 `player` 的有效手牌中选择第一个采用该定义的牌；该牌必须存在。玩家相对于命令执行时的本方确定，见 [relative_player](relative_player.md)。

默认构造 `discard_hand_card{}` 时，消费响应通过 `invoke` 提交的一个 [`discard_hand_card_input`](../command_inputs/discard_hand_card_input.md)，其中 `card` 必须是有效手牌。

## 结算

1. 将手牌及其附属状态标记为离场。
2. 仅向被舍弃的手牌发送 `hand_card_discard_effect`，完整执行返回的程序。
3. 全场广播 [`hand_card_discarded`](../events/hand_card_discarded.md)，完整执行所有响应。

离场后的卡牌仍可通过 ID 读取其定义和状态，直到安全清理。自身舍弃效果与全场通知是两种事件：定义仅响应后者不代表它具有自身舍弃效果。

## 参阅

- [`discard_deck_cards`](discard_deck_cards.md)：批量舍弃牌堆顶的牌。
