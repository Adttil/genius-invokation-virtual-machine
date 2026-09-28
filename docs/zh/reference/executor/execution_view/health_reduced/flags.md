[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<health_reduced>](../health_reduced.md) / **flags**

# givm::execution_view<execution_state::health_reduced>::flags

定义于头文件 `<givm/executor.hpp>`

```cpp
damage_flags flags() const noexcept(/* Release 为 true，Debug 为 false */);
```
[`damage_flags`](../../../enums/damage_flags.md)

取得本次伤害标志。

## 返回值

本次伤害附带的标志。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
