[返回](../events.md)

# changing_energy

角色充能写入前的可修改增量。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `target` | `const character_id` | 目标角色。 |
| `delta` | `std::int32_t` | 本次正负增量，可修改。 |

## 响应

响应可修改增量；实际上下限由 `change_energy` 在广播后统一应用。
