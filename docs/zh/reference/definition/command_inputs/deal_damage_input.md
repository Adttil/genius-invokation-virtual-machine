[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **deal_damage_input**

# givm::deal_damage_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
struct deal_damage_input
{
    std::span<const damage> damages;
};
```

一组伤害的动态输入。组内初始描述数量在响应时决定，整个对象对应一条动态伤害命令。 配合 [`deal_damage`](../commands/deal_damage.md) 和 [`handle_context::invoke`](../../executor/handle_context/invoke.md) 使用。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `damages` | `std::span<const damage>` | 按处理顺序排列的伤害初始描述；允许为空 |

## 注意

`invoke` 复制 [`damage`](damage.md) 数组的内容，返回后不再借用该 span。数组须在复制期间保持有效。空数组不产生伤害；数组中的范围描述仍在命令执行时展开。组内处理顺序与广播时机见 [`deal_damage`](../commands/deal_damage.md)。
