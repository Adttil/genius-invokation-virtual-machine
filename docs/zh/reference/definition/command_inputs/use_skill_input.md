[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **use_skill_input**

# givm::use_skill_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
using use_skill_input = skill_effect;
```

直接使用技能的动态输入。技能必须属于当前出战角色；命令不支付技能费用。 配合 [`use_skill`](../commands/use_skill.md) 和 [`handle_context::invoke`](../../executor/handle_context/invoke.md) 使用。

## 注意

本类型是 [`skill_effect`](../events/skill_effect.md) 的别名，成员及其限定与该事件相同。`use_skill::input_type` 指向此类型，响应可以用别名或原事件类型构造输入；将它用于输入不会单独触发广播，结算与通知仍由命令决定。
