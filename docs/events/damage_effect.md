[返回](../events.md)

# damage_effect

伤害计算完成后、实际扣血前的效果结算工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `const std::variant<hand_card_id, hand_card_status_id, deck_card_status_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id>` | 伤害来源。 |
| `target` | `const character_id` | 受伤角色。 |
| `value` | `std::uint32_t` | 剩余伤害值，唯一可修改字段。 |
| `type` | `const damage_type` | 最终伤害类型。 |
| `flags` | `const damage_flags` | 最终伤害标记。 |

## 响应

不需要修改 table 的减伤响应可以直接修改 `value`。需要同时消耗实体计数的护盾可由 handler 判断目标、伤害标记和自身状态，再返回包含 [`absorb_damage_by_count`](../instructions/absorb_damage_by_count.md) 的固定程序入口；该指令从广播 frame 读取当前 `damage_effect` 和当前 handler，并同时修改剩余伤害与实体计数。后一个响应立即看见前一个响应后的剩余值。

所有响应都返回 Context 强类型固定入口；没有动态调度接口。
