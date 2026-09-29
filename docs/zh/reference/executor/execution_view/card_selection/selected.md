[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<card_selection>](../card_selection.md) / **selected**

# givm::execution_view<execution_state::card_selection>::selected

定义于头文件 `<givm/runtime.hpp>`

```cpp
constexpr std::bitset<selection_capacity> selected() const noexcept(/* Release 为 true，Debug 为 false */);
```
[`selection_capacity`](../../selection_capacity.md)

取得当前填写的换牌选择。

## 返回值

选择位集的副本。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
