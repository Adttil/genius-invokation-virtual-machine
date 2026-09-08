[返回](../events.md)

# set_active_character

直接设置目标玩家的出战角色，供初始化或规则强制设置使用。主动切人由 [`begin_action`](begin_action.md) 自行推进，并使用相同的 `active_character_changed` 通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `target` | `character_id` | 新出战角色，同时确定所属玩家。 |

## 执行

本命令入口不要求额外栈输入。执行期间会临时压入 `active_character_changed` 广播 frame；广播完整结算后弹出该 frame，恢复入口栈形状。

1. 将目标玩家的出战角色写为 `target`。
2. 推进 [`active_character_changed`](../events/active_character_changed.md) 广播。

## 注意

- 本命令用于单个玩家或规则强制设置。开局双方同时选择出战角色通常使用 [`select_active_character_both`](select_active_character_both.md)。
- `target` 必须指向有效角色。即使目标已经是当前出战角色，仍会广播通知。
