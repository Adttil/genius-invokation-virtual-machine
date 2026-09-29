[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<health_reduced>](../health_reduced.md) / **value**

# givm::execution_view<execution_state::health_reduced>::value

定义于头文件 `<givm/runtime.hpp>`

```cpp
std::uint32_t value() const noexcept(/* Release 为 true，Debug 为 false */);
```

取得结算后的伤害值。

## 返回值

经过修正及抵挡、用于减少生命的完整伤害值，不以目标原有生命为上限。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
