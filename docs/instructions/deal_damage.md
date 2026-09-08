[返回](../events.md)

# deal_damage

推进一次伤害的计算、减免、生命扣除、元素附着和事后通知流程。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `damage_source_id` | 伤害来源实体。 |
| `target` | `character_id` | 受伤角色。 |
| `value` | `std::uint32_t` | 进入计算阶段的伤害值。 |
| `multiplier_numerator` | `std::uint16_t` | 最终统一应用的倍率分子，默认为 1。 |
| `multiplier_denominator` | `std::uint16_t` | 最终统一应用的倍率分母，默认为 1；不得为 0。 |
| `type` | `damage_type` | 伤害类型。 |
| `flags` | `damage_flags` | 伤害规则标记。 |

## 执行

本命令入口不要求额外栈输入。执行期间会临时压入 `damage_calculation`、`damage_effect` 和 `after_damage` 广播 frame；元素反应期间还会保存 `after_damage, stage_t` frame，并推进元素反应广播。彻底完成时会恢复入口栈形状。

1. 根据命令字段生成 [`damage_calculation`](../events/damage_calculation.md)。
2. 按当前 table 枚举可响应 `damage_calculation` 的实体，连同游标和事件对象一起压入广播 frame。
3. 推进 `damage_calculation` 广播并读取最终事件；若伤害类型携带元素且尚未处理反应，先应用默认伤害反应修正。
4. 将修正后的伤害值乘以倍率分子，再除以倍率分母，向下取整并饱和到 `std::uint32_t` 上限，以结果生成并推进 [`damage_effect`](../events/damage_effect.md) 广播。handler 修改后的倍率分母同样不得为 0。
5. 根据最终 `damage_effect` 扣除目标生命，最低降至 0。
6. 若伤害携带元素，在本指令内部推进与 [`apply_element`](apply_element.md) 相同的元素附着和反应协议。
7. 推进 [`after_damage`](../events/after_damage.md) 广播。
8. 检查双方是否仍有存活角色；若任一方全灭，结束对局并令 `executor.status()` 返回对应结果，否则进入下一固定指令。

所有可能降低角色生命的新增指令都必须在自身事务完成后承担同类终局检查，不能依赖一个游离的后置检查指令。
