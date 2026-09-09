[返回](../events.md)

# absorb_damage_by_count

使用当前响应实体的计数吸收当前伤害。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `maximum_count` | `std::uint32_t` | 本次最多消耗的计数；默认不设额外上限。 |

## 上下文

本指令声明 `context_type = damage_effect`，只能编译进 `damage_effect` 响应程序。它从当前 activation frame 下方的普通广播 frame 读取 `damage_effect` 和当前 handler id；当前 handler 必须是状态中带 `count` 的实体。

## 执行

令 `absorbed = min(event.value, self.state().count, maximum_count)`，随后同时执行：

```text
event.value -= absorbed
self.state().count -= absorbed
```

然后消费自身并继续固定响应程序。它不压入临时 frame，也不维护额外 stage。

触发条件不属于本指令。定义源 handler 应先判断伤害目标、`ignore_shield`、当前计数以及该护盾自己的其他规则，再决定返回本程序入口还是 null。计数变为 0 时本指令不会自动使实体离场，也不会产生 `entity_count_changed`；这些行为应由定义自己的后续程序表达。
