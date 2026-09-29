[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<health_reduced>](../health_reduced.md) / **type**

# givm::execution_view<execution_state::health_reduced>::type

定义于头文件 `<givm/runtime.hpp>`

```cpp
damage_type type() const noexcept(/* Release 为 true，Debug 为 false */);
```
[`damage_type`](../../../enums/damage_type.md)

取得本次伤害类型。

## 返回值

本次伤害的类型。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
