[返回](../events.md)

# card_drawn

一张牌已经因抽牌进入手牌后的只读通知。一个事件永远只描述一张牌；批量抽牌由多个依次广播的 `card_drawn` 表示。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本批次抽取时取得的手牌 ID；后续响应可能已使该实体失效。 |

## 时序保证

[`draw_cards`](../instructions/draw_cards.md) 和 [`replace_cards`](../instructions/replace_cards.md) 都先完成整个批次的牌区状态修改，再按照牌进入手牌的顺序逐张广播本事件。

因超过手牌上限而在抽取时直接丢弃的牌没有进入手牌，也不会产生本事件。

每张牌开始广播时，分别根据当时的 table 采样响应者；不是整批共用一次快照。响应可以返回固定入口，该程序在当前 handler 返回后立即执行，随后广播继续。

首个事件开始前，同一批次成功抽取的所有牌都已进入手牌。前面事件或 handler 的响应程序可能继续移动或移除牌，因此后续响应读取 `card` 时仍须判断实体是否有效。[`replace_cards_both`](../instructions/replace_cards_both.md) 的开局双方换牌不广播本事件。
