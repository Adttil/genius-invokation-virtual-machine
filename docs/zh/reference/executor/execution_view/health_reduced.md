[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<health_reduced>**

# givm::execution_view<execution_state::health_reduced>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::health_reduced>;
```

伤害扣除角色生命后的现场视图。此时伤害值已经确定，相关元素附着及伤害后响应尚未处理。

## 成员函数

| | |
| --- | --- |
| [`source`](health_reduced/source.md) | 取得本次伤害来源。 |
| [`target`](health_reduced/target.md) | 取得本次伤害目标。 |
| [`value`](health_reduced/value.md) | 取得结算后的伤害值。 |
| [`type`](health_reduced/type.md) | 取得本次伤害类型。 |
| [`flags`](health_reduced/flags.md) | 取得本次伤害标志。 |

## 注意

[`value`](health_reduced/value.md) 是经过伤害计算与抵挡后的完整伤害值，不以目标原有生命为上限。目标剩余 1 点生命、受到 999 点伤害时，此处仍为 999，而牌桌上的生命值已经扣至 0。最终伤害值为 0 时不产生此现场。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
