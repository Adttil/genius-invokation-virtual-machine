[返回](../events.md)

# skill_will_be_used

技能效果实际被安排前的可修改工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `skill` | `const skill_id` | 要使用的技能。 |
| `kind` | `const skill_kind` | 技能类别。 |
| `speed` | `action_speed` | 行动速度，可修改。 |
| `effect_cancelled` | `bool` | 是否仅无效化技能定义效果，默认 false。 |

## 响应

响应可无效化技能效果或改变行动速度；无效化不撤销支付，费用竞争已在 payment 计算阶段完成。
