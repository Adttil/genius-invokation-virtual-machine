[返回](../events.md)

# active_character_changed

出战角色已经被设置后的只读通知。初始化、规则强制设置和主动切人最终都使用同一个通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `current` | `const character_id` | 当前出战角色。 |

需要所属玩家时，从 `current.player_id` 读取。当前核心不再为主动切人额外区分另一种 changed/switched 事件。
