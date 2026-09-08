[返回](../events.md)

# secret_points_changed

秘传点修改完成的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `player` | `const player_id` | 目标玩家。 |
| `previous` | `const std::uint32_t` | 修改前数值。 |
| `current` | `const std::uint32_t` | 修改后数值。 |

## 响应

用于观察最终变化，响应不能重写结果。
