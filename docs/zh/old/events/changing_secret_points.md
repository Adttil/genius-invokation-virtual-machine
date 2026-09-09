[返回](../events.md)

# changing_secret_points

秘传点写入前的可修改增量。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `player` | `const player_id` | 目标玩家。 |
| `delta` | `std::int32_t` | 本次正负增量，可修改。 |

## 响应

响应按顺序修改同一个 `delta`；消耗来源等副作用由 handler 返回的固定响应程序完成。
