[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<health_reduced>](../health_reduced.md) / **value**

# givm::execution_view<execution_state::health_reduced>::value

定义于头文件 `<givm/executor.hpp>`

```cpp
std::uint32_t value() const noexcept;
```

取得结算后的伤害值。

## 返回值

经过修正及抵挡、用于减少生命的完整伤害值，不以目标原有生命为上限。
