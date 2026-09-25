[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **set_skill_state**

# givm::set_skill_state

定义于头文件 `<givm/definition.hpp>`

```cpp
struct set_skill_state
{
    using input_type = set_skill_state_input;

    relative_character_target character{};
    definition_id<skill_view> definition{};
    skill_state state{};
};
```

技能状态的赋值命令，用于记录技能定义自行解释的计数。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`set_skill_state_input`](../command_inputs/set_skill_state_input.md)，动态模式下的输入类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `character` | [`relative_character_target`](../events/relative_character_target.md) | 固定模式下技能所属角色的位置，默认本方出战角色 |
| `definition` | `definition_id<skill_view>` | 固定模式下要匹配的技能定义 |
| `state` | [`skill_state`](../../table/skill_state.md) | 固定模式下要写入的完整状态 |

## 注意

默认构造 `set_skill_state{}` 使用动态模式，由响应通过 `invoke` 提交一个 [`set_skill_state_input`](../command_inputs/set_skill_state_input.md)，指定实际技能实体及新状态。

显式指定 `definition` 时使用固定模式，不消费响应输入。命令执行时按 `character` 定位角色，在其技能中选择首个有效、定义 ID 匹配的技能；角色与技能都必须存在。`character.selection` 必须为 `character_selection::character`。

固定位置允许定位后台角色及已战败但未离场的角色，不会因为角色战败而顺延到其他角色。动态输入同样不限制技能属于当前出战角色。

命令直接替换完整状态，不限制或裁剪 `count`，也不因 `count` 为零而删除技能。命令不广播事件，也不产生专门的观察现场。

## 参阅

| | |
| --- | --- |
| [`set_skill_state_input`](../command_inputs/set_skill_state_input.md) | 技能状态赋值的动态输入 |
| [`skill_state`](../../table/skill_state.md) | 技能的计数状态 |
