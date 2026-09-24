[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **card_effect**

# givm::card_effect

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_effect;
```

执行打出的牌自身效果时，提供这张牌及其目标。牌定义通过响应通过 `invoke` 提交的效果入口实现原本的用牌效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次打出的牌；只读。 |
| `targets` | `const std::array<card_target_id, 2>` | 本次采用的两个目标位置；只读，未使用的位置忽略。 |

## 注意

可打出的牌必须提供本事件的响应，没有后续效果时也可返回空入口。本事件仅调用打出的牌自己的定义。在支付和 [`card_will_be_played`](card_will_be_played.md) 的响应完成后，只有未被反制的牌才执行本事件。其效果完成后再广播 [`card_played`](card_played.md)。

此时牌已经离开手牌；其 ID 在牌桌 [`clean_up`](../../table/table/clean_up.md) 前仍能读取原牌信息，但普通手牌遍历与广播不会再包括该牌。牌自身效果仍按这次打出时的定义执行。

天赋牌可在效果程序中使用 [`use_skill`](../commands/use_skill.md)。该命令完整结算技能效果和 [`skill_used`](skill_used.md) 通知后，才继续本牌的效果程序；本牌的全部效果完成后再广播 `card_played`。技能命令不支付费用，是否为战斗行动仍由本牌的费用及出牌流程决定。
