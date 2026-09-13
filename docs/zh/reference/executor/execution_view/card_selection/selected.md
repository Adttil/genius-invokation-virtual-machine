[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<card_selection>](../card_selection.md) / **selected**

# givm::execution_view<execution_state::card_selection>::selected

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr std::bitset<selection_capacity> selected() const noexcept;
```
[`selection_capacity`](../../selection_capacity.md)

取得当前填写的换牌选择。

## 返回值

选择位集的副本。
