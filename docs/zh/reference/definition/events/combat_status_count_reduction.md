[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **combat_status_count_reduction**

# givm::combat_status_count_reduction

定义于头文件 `<givm/definition.hpp>`

```cpp
struct combat_status_count_reduction;
```

出战状态计数扣除的初始输入。由响应确定扣除对象和数量，提交给 [`reduce_combat_status_count`](../commands/reduce_combat_status_count.md) 执行；它不是一次独立的广播。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `status` | [`combat_status_id`](../../table/combat_status_id.md) | 要扣除计数的出战状态 |
| `count` | `std::uint32_t` | 要扣除的数量 |
