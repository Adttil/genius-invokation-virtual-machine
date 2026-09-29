[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<health_reduced>](../health_reduced.md) / **target**

# givm::execution_view<execution_state::health_reduced>::target

定义于头文件 `<givm/runtime.hpp>`

```cpp
character_id target() const noexcept(/* Release 为 true，Debug 为 false */);
```
[`character_id`](../../../table/character_id.md)

取得本次伤害目标。

## 返回值

受到伤害的角色 ID。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
