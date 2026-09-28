[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_reroll_selection>](../dice_reroll_selection.md) / **selection_validate**

# givm::execution_view<execution_state::dice_reroll_selection>::selection_validate

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr bool selection_validate(const table& card_table, const dice_counts& selected) const noexcept(/* Release 为 true，Debug 为 false */);
```
[`table`](../../../table/table.md)
[`dice_counts`](../../../enums/dice_counts.md)

检查当前玩家是否持有准备重投的骰子。

## 参数

| | |
| --- | --- |
| `card_table` | 与当前重投现场配套的牌桌。 |
| `selected` | 拟重投的各类骰子数量。 |

## 返回值

各类所选骰子均不超过该玩家当前持有数量时返回 `true`，否则返回 `false`。全零选择合法，表示放弃全部剩余机会。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。

本操作只检查，不填写选择、消耗重投机会、推进对局或调用随机源。检查成功后仍须调用 [`select`](select.md) 提交；能够保证合法时也可以直接提交。Debug 提交时自动执行本检查；Release 不重复检查。
