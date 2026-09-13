[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<card_selection>](../card_selection.md) / **select**

# givm::execution_view<execution_state::card_selection>::select

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void select(std::bitset<selection_capacity> selected) const noexcept;
```
[`selection_capacity`](../../selection_capacity.md)

填写当前指定玩家要替换的手牌选择。

## 参数

| | |
| --- | --- |
| `selected` | 按当前有效手牌遍历顺序标记的选择位集，空选择表示全部保留。 |

## 返回值

(无)

## 注意

本操作不执行换牌。下一次推进才消费选择，不检查位集或现场是否合法。
