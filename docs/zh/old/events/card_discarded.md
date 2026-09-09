[返回](../events.md)

# card_discarded

一张牌已经被标记弃置的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `card` | `const std::variant<hand_card_id, deck_card_id>` | 被弃置牌的历史 ID。 |

## 响应

目标牌已不可解引用。牌自身需要执行的离场响应必须由 `discard_card` 在失效前定点通知或提前安排。
