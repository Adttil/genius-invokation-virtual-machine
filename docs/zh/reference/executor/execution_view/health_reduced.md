[givm](../../../reference.md) / [执行](../../executor.md) / [执行视图](../execution_view.md) / **health_reduced**

# 扣血观察

观察模式下每次非零伤害扣血后的现场。`source()`、`target()`、`value()`、`type()`、`flags()` 和 `reaction()` 提供本击结果；`reaction()` 返回 `reaction_id`。`resume` 继续执行元素反应后续效果及随后命令。

角色生命为零时仍可能存活并等待段收尾的濒死广播。当前观察不代表已确认死亡，也不代表伤害后通知已经执行。`after_damage` 是段内合并摘要，扣血视图则保留单次来源和类型。
