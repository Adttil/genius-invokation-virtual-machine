[返回](../events.md)

# apply_element

对角色附着元素，并在需要时组织元素反应效果。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `source` | `element_application_source_id` | 元素附着来源。 |
| `target` | `character_id` | 目标角色。 |
| `element` | `element` | 要附着的元素。 |
| `cause` | `element_application_cause` | 普通效果附着或伤害附着；默认为 `effect`。 |

## 执行

本命令入口不要求特定栈顶布局，也不直接读取输入栈。执行期间可能临时压入 `elemental_reaction_will_occur` 和 `after_elemental_reaction` 广播 frame；彻底完成时会恢复入口栈形状。

1. 根据目标当前 aura 和本命令的 `element` 判断是否发生元素反应。
2. 若无反应，直接写入反应外的 aura，然后完成。
3. 若有反应，生成 [`elemental_reaction_will_occur`](../events/elemental_reaction_will_occur.md)。
4. 按当前 table 枚举可响应 `elemental_reaction_will_occur` 的实体，连同游标和事件对象一起压入广播 frame。
5. 推进该广播；若响应者没有接管默认反应，本指令直接应用默认 aura 结果。响应者若接管，则由其固定响应程序修改事件或 table。
6. 推进 [`after_elemental_reaction`](../events/after_elemental_reaction.md) 广播，然后完成。

定制响应若接管默认处理，应将 `already_handled` 设为 `true` 并在响应程序中完成所需替代效果；仅返回非空入口不会自动阻止默认 aura 更新。
