[返回](../events.md)

# set_active_character

直接设置目标玩家的出战角色。初始化、规则强制设置和当前最小主动切人分支都会复用这条指令。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `target` | `character_id` | 新出战角色，同时确定所属玩家。 |

## 执行

本命令不要求特定栈顶布局，也不直接读写 executor stack。

1. 将目标玩家的出战角色写为 `target`。
2. 推进 [`active_character_changed`](../events/active_character_changed.md) 广播。

## 注意

- 本命令用于单个玩家或规则强制设置。开局双方同时选择出战角色通常使用 [`select_active_character_both`](select_active_character_both.md)。
