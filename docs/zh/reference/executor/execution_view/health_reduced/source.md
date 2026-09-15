[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<health_reduced>](../health_reduced.md) / **source**

# givm::execution_view<execution_state::health_reduced>::source

定义于头文件 `<givm/executor.hpp>`

```cpp
const damage_source_id& source() const noexcept;
```
[`damage_source_id`](../../../definition/events/damage_source_id.md)

取得本次伤害来源。

## 返回值

借用当前现场的伤害来源。
