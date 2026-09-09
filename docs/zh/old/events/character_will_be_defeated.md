[返回](../events.md)

# character_will_be_defeated

角色真正写入击倒状态前的可修改工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `target` | `const character_id` | 待击倒角色。 |
| `prevented` | `bool` | 是否阻止本次击倒，默认 false。 |
| `revive_health` | `std::uint32_t` | 阻止后应设置的生命值，默认 0。 |

## 响应

复活效果设置 `prevented` 和 `revive_health`，并通过返回固定响应入口完成自身消耗。后续响应能看到击倒已被阻止。
