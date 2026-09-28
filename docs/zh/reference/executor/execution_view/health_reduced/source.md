[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<health_reduced>](../health_reduced.md) / **source**

# givm::execution_view<execution_state::health_reduced>::source

定义于头文件 `<givm/executor.hpp>`

```cpp
const damage_source_id& source() const noexcept(/* Release 为 true，Debug 为 false */);
```
[`damage_source_id`](../../../definition/events/damage_source_id.md)

取得本次伤害来源。

## 返回值

借用当前现场的伤害来源。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
