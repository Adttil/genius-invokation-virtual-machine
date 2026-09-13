[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_selection>](../dice_selection.md) / **selected**

# givm::execution_view<execution_state::dice_selection>::selected

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr std::bitset<selection_capacity> selected() const noexcept;
```
[`selection_capacity`](../../selection_capacity.md)

取得当前填写的重投选择。

## 返回值

选择位集的副本。
