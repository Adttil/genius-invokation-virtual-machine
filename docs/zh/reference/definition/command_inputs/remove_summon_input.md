[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **remove_summon_input**

# givm::remove_summon_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
struct remove_summon_input
{
    std::span<const summon_id> summons;
};
```

按顺序移除一组召唤物的输入。 配合 [`remove_summon`](../commands/remove_summon.md) 使用。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `summons` | `std::span<const summon_id>` | 本次目标列表 |

## 注意

`summons` 可以跨双方；允许为空，目标不得重复。每次移除后完整结算离场广播，再处理下一项。此前响应已移除的后续目标跳过，新产生的召唤物不加入本批。命令不改变目标的效果量或可用次数。

[`invoke`](../../executor/handle_context/invoke.md) 复制数组内容，返回后不再借用原数组。数组须在复制期间保持有效。目标须在命令开始执行时有效。
