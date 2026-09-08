[返回](../events.md)

# entity_count_changed

实体的完整 `std::uint32_t` 计数器已被替换后的只读通知。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `entity` | `const std::variant<support_id, summon_id, combat_status_id, attachment_id>` | 计数发生变化的实体。 |
| `previous` | `const std::uint32_t` | 旧计数器。 |
| `current` | `const std::uint32_t` | 新计数器。 |

## 响应

实体定义自行解释位布局，并可在任意规则条件下安排 `discard_entity`；框架不把零值自动等同于离场。
