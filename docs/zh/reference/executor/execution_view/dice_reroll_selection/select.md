[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_reroll_selection>](../dice_reroll_selection.md) / **select**

# givm::execution_view<execution_state::dice_reroll_selection>::select

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void select(const dice_counts& selected) const noexcept;
```
[`dice_counts`](../../../enums/dice_counts.md)

填写当前玩家本次要重投的骰子选择。

## 参数

| | |
| --- | --- |
| `selected` | 要重投的各类骰子数量，不得超过当前玩家持有数量；全零表示放弃全部剩余重投机会。 |

## 返回值

(无)

## 注意

可先通过 [`selection_validate`](selection_validate.md) 独立检查。本操作不检查，只填写选择；下一次推进才替换所选骰子并消耗一次机会，或按空选择结束本命令。
