[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **discard_deck_cards**

# givm::discard_deck_cards

定义于头文件 `<givm/definition.hpp>`

```cpp
struct discard_deck_cards;
```

从指定玩家的牌堆顶舍弃至多 `count` 张牌。牌堆不足时舍弃剩余全部牌。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`discard_deck_cards_input`](../command_inputs/discard_deck_cards_input.md)，动态模式下的输入类型 |

## 成员

| | |
| --- | --- |
| `count` | 要舍弃的牌数。 |
| `player` | 相对于命令执行时本方的玩家，默认 `relative_player::self`。 |

指定 `count` 时使用固定参数，如 `discard_deck_cards{ .count = 2 }`。默认构造 `discard_deck_cards{}` 时，消费响应通过 `invoke` 提交的一个 [`discard_deck_cards_input`](../command_inputs/discard_deck_cards_input.md)。

## 结算

先确定牌堆顶的本批卡牌，并将全部卡牌及其附属状态标记为离场。随后按从牌堆顶向下的顺序逐张结算：

1. 仅向该牌发送 [`deck_card_discard_effect`](../events/deck_card_discard_effect.md)，完整执行其舍弃效果。
2. 全场广播 [`deck_card_discarded`](../events/deck_card_discarded.md)，完整执行全部响应。
3. 再处理本批的下一张牌。

前一张牌的效果不会改变本批已经选定的卡牌。所有效果与通知都能观察到本批已经全部离场。

以 `compile_mode::observed` 编译时，整批离场后、第一张牌的自身效果前，报告 [`execution_state::deck_cards_discarded`](../../executor/execution_state.md)；其[视图](../../executor/execution_view/deck_cards_discarded.md)给出本批全部卡牌 ID。若实际没有舍弃任何牌，则无观察现场、效果或通知。

本命令与[舍弃手牌](discard_hand_card.md)采用不同的效果和通知类型。抽牌溢出造成的牌堆卡牌移除不属于舍弃。
