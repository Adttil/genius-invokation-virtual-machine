[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **set_energy_input**

# givm::set_energy_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
struct set_energy_input
{
    character_id target;
    std::uint32_t value;
};
```

[`set_energy`](../commands/set_energy.md) 的动态输入，包含目标角色和要设置的充能值。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `character_id` | 要修改的有效角色实体，允许生命值为零 |
| `value` | `std::uint32_t` | 要设置的充能值，执行时裁剪至角色当前 `max_energy` |

## 注意

本类型仅作为命令输入，不是可订阅的通知。命令不改变充能类型，也不发送充能变化广播。
