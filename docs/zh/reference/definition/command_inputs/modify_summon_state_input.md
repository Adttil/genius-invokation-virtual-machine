[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **modify_summon_state_input**

# givm::modify_summon_state_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
struct modify_summon_state_input
{
    std::span<const summon_id> summons;
    std::int64_t value{};
    std::int64_t usages{};
};
```

按同一组有符号增量批量修改召唤物状态的输入。 配合 [`modify_summon_state`](../commands/modify_summon_state.md) 使用。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `summons` | `std::span<const summon_id>` | 本次目标列表 |
| `value` | `std::int64_t` | 效果量增量，默认零 |
| `usages` | `std::int64_t` | 可用次数增量，默认零 |

## 注意

`summons` 按处理顺序指定目标，可以跨双方；允许为空，目标不得重复。全部状态修改完成后，才按该顺序处理带 `remove_at_zero_usages` 标签且当前次数为零的召唤物离场。

[`invoke`](../../executor/handle_context/invoke.md) 复制数组内容，返回后不再借用原数组。数组须在复制期间保持有效。目标须在命令开始执行时有效。
