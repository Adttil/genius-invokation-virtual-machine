[返回](../events.md)

# elemental_tuning_will_apply

元素调和实际弃牌和转骰前的可取消工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 将用于调和的手牌。 |
| `from` | `const elemental_dice` | 原骰子类型。 |
| `to` | `const elemental_dice` | 目标骰子类型。 |
| `cancelled` | `bool` | 设为 true 可取消调和生效。 |

## 响应

响应不能直接修改 table；需要付出的额外代价由返回的固定响应程序完成。若任一响应取消，后续元素调和效果不会执行。
