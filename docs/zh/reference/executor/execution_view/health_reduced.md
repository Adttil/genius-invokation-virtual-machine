[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<health_reduced>**

# givm::execution_view<execution_state::health_reduced>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::health_reduced>;
```

伤害扣除角色生命后的现场视图。此时伤害值已经确定，击倒处理、终局判定、相关元素附着及伤害后响应尚未进行。

## 成员函数

| | |
| --- | --- |
| [`source`](health_reduced/source.md) | 取得本次伤害来源。 |
| [`target`](health_reduced/target.md) | 取得本次伤害目标。 |
| [`value`](health_reduced/value.md) | 取得结算后的伤害值。 |
| [`type`](health_reduced/type.md) | 取得本次伤害类型。 |
| [`flags`](health_reduced/flags.md) | 取得本次伤害标志。 |
| [`reaction`](health_reduced/reaction.md) | 取得本次伤害引起的元素反应种类。 |

## 注意

[`value`](health_reduced/value.md) 是经过伤害计算与抵挡后的完整伤害值，不以目标原有生命为上限。目标剩余 1 点生命、受到 999 点伤害时，此处仍为 999，而牌桌上的生命值已经扣至 0。最终伤害值为 0 时不产生此现场。

若本次扣血使目标生命降至零，其 attachment 和充能在此现场仍保留；继续推进先判定终局。若对局结束，则保留这些数据并立即停止；若对局继续，才删除 attachment、清空充能。濒死响应与复活流程尚未接入。

一组伤害可以多次到达本现场；此时只是当前一次扣血完成，不表示整组完成。对局继续时，全部伤害扣血、击倒处理与元素附着完成以后，才调用该组的反应后和伤害后响应；若此前已终局，后续处理不再进行。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
