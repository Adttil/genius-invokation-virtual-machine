[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **modify_energy**

# givm::modify_energy

定义于头文件 `<givm/definition.hpp>`

```cpp
struct modify_energy;
```

按有符号增量修改角色充能的命令，用于获得或消耗充能。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | [`relative_character_target`](../events/relative_character_target.md) | 固定目标位置；默认采用动态输入 |
| `delta` | `std::int64_t` | 固定充能增量，正数增加、负数减少，默认零 |

## 注意

默认构造 `modify_energy{}` 使用动态模式，由响应通过 `invoke` 提交一个 [`energy_modification`](../events/energy_modification.md)。显式指定 `target` 时采用固定模式，不消费响应输入。

固定模式在命令执行时按相对位置定位角色，`target.selection` 必须为 `character_selection::character`；没有有效目标时跳过命令。允许目标为已战败但未离场的角色，不会因为角色战败而顺延到其他角色。动态输入必须指定实际存在的有效角色。

命令执行时读取当前 `energy`，加上 `delta`，再将结果限制在零与当前 `max_energy` 之间。动态输入提交的是增量，不是响应时预先算出的最终值，因此此前命令对充能的修改也会参与本次计算。`INT64_MIN`、`INT64_MAX` 也遵循这一饱和规则，不发生算术回绕。

命令不改变 `energy_tag`，操作普通充能还是替代充能由定义自行决定。命令只修改牌桌，不发送 [`changing_energy`](../events/changing_energy.md) 或 [`energy_changed`](../events/energy_changed.md)，也不产生专门的观察现场。

技能使用不会自动增加充能。需要获得充能的技能应在自身效果程序的适当位置安排本命令，通常放在其他技能效果之后。

## 参阅

| | |
| --- | --- |
| [`energy_modification`](../events/energy_modification.md) | 按增量修改充能的动态输入 |
| [`set_energy`](set_energy.md) | 充能赋值命令及执行示例 |
