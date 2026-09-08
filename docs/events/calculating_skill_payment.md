[返回](../events.md)

# calculating_skill_payment

计算一个技能 payment 时发布的事件。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `skill` | `const skill_id` | 正在报价的技能。 |

## 响应

payment 保存在该技能对应的 table 槽位。响应通过 command 修改费用、速度或 `on_pay`，后一个响应读取前一个响应完成后的最新槽位。
