[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_selection>](../dice_selection.md) / **selected**

# givm::execution_view<execution_state::dice_selection>::selected

定义于头文件 `<givm/runtime.hpp>`

```cpp
constexpr dice_counts selected() const noexcept(/* Release 为 true，Debug 为 false */);
```
[`dice_counts`](../../../enums/dice_counts.md)

取得当前填写的重投选择。

## 返回值

当前填写的各类重投骰子数量的副本。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
