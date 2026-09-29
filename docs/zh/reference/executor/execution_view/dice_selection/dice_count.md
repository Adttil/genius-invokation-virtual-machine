[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_selection>](../dice_selection.md) / **dice_count**

# givm::execution_view<execution_state::dice_selection>::dice_count

定义于头文件 `<givm/runtime.hpp>`

```cpp
constexpr std::uint32_t dice_count() const noexcept(/* Release 为 true，Debug 为 false */);
```

取得本次投骰阶段的骰子数量。

## 返回值

每方本次投出的骰子总数。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
