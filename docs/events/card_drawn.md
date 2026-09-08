[返回](../events.md)

# card_drawn

一张牌已经因抽牌进入手牌后的只读通知。一个事件永远只描述一张牌；批量抽牌由多个依次广播的 `card_drawn` 表示。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 被抽到且当前位于手牌中的牌。 |

## 时序保证

[`draw_cards`](../instructions/draw_cards.md) 和 [`replace_cards`](../instructions/replace_cards.md) 都先完成整个批次的牌区状态修改，再按照牌进入手牌的顺序逐张广播本事件。

因超过手牌上限而在抽取时直接丢弃的牌没有进入手牌，也不会产生本事件。

响应可读取 `card` 对应的手牌实体并返回固定响应入口。响应开始时，同一批次中成功抽取的所有牌都已经在手牌中；每个响应程序在当前 handler 返回后立即执行，随后广播继续。
