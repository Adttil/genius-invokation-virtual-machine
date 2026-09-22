[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<health_reduced>**

# givm::execution_view<execution_state::health_reduced>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::health_reduced>;
```

伤害扣除角色生命后的现场视图。可以读取本段伤害的结算结果、原始元素反应及其替代标签；此时击倒处理、终局判定、相关元素附着与默认反应实体生成尚未进行。

## 成员函数

| | |
| --- | --- |
| [`source`](health_reduced/source.md) | 取得本次伤害来源。 |
| [`target`](health_reduced/target.md) | 取得本次伤害目标。 |
| [`value`](health_reduced/value.md) | 取得结算后的伤害值。 |
| [`type`](health_reduced/type.md) | 取得本次伤害类型。 |
| [`flags`](health_reduced/flags.md) | 取得本次伤害标志。 |
| [`reaction`](health_reduced/reaction.md) | 取得本次伤害引起的原始元素反应种类。 |
| [`replacement_reaction`](health_reduced/replacement_reaction.md) | 取得本次反应的替代标签。 |

## 注意

[`value`](health_reduced/value.md) 是经过伤害计算与抵挡后的完整伤害值，不以目标原有生命为上限。目标剩余 1 点生命、受到 999 点伤害时，此处仍为 999，而牌桌上的生命值已经扣至 0。最终伤害值为 0 时不产生此现场。

若本次扣血使目标生命降至零，其 attachment 和充能在此现场仍保留；继续推进先判定终局。若对局结束，则保留这些数据并立即停止；若对局继续，才删除 attachment、清空充能。濒死响应与复活流程尚未接入。

一组伤害可以多次到达本现场；此时只是当前一次扣血完成，不表示整组完成。当前段的默认反应实体尚未生成，牌桌可能已有前段生成且仍在场的实体。对局继续时，先处理当前段的击倒、元素附着与未被替代的默认实体生成，再继续下一段伤害。

整组扣血结束后，先处理本组首次登记的默认超载：为登记玩家当时的出战角色选择下一个存活角色，完成切换及其通知，再调用反应后和伤害后响应。原受击角色后来被击倒或中途发生切换，不取消已经登记的超载。以观察模式执行时，超载切换也会到达相应切人现场。若此前已终局，后续处理不再进行。

当前段引起冻结时，新的冻结附属在本现场尚未施加；继续推进后只向仍存活的目标施加，所选定义带 `control` 标签时还受免控保护限制。超载则在组末检查当时出战角色的免控保护，受保护时不切换，也不产生切人通知或观察现场。

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
