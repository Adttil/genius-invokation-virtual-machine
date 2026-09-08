[返回](../events.md)

# energy_changed

角色充能修改完成的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `target` | `const character_id` | 目标角色。 |
| `previous` | `const std::uint32_t` | 修改前充能。 |
| `current` | `const std::uint32_t` | 修改后充能。 |

## 响应

用于充能变化后的效果。
