[返回](../events.md)

# skill_used

技能操作已经完成提交的只读通知；字段会说明定义效果是否被无效化。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `skill` | `const skill_id` | 已使用技能。 |
| `kind` | `const skill_kind` | 技能类别。 |
| `speed` | `const action_speed` | 最终行动速度。 |
| `effect_cancelled` | `const bool` | 技能定义效果是否被无效化。 |

## 响应

响应安排的“使用技能后”效果位于技能定义自身安排的效果之后。
