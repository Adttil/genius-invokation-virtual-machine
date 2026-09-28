[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **set_skill_state**

# givm::set_skill_state

定义于头文件 `<givm/definition.hpp>`

```cpp
struct set_skill_state_error;

struct set_skill_state
{
    using error_type = set_skill_state_error;

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
| [`error_type`](#编译检查) | `set_skill_state_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `character` | [`relative_character_target`](../events/relative_character_target.md) | 固定模式下技能所属角色的位置，默认本方出战角色 |
| `definition` | `definition_id<skill_view>` | 固定模式下要匹配的技能定义 |
| `state` | [`skill_state`](../../table/skill_state.md) | 固定模式下要写入的完整状态 |

## 编译检查

```cpp
struct set_skill_state_error;
```

`set_skill_state::error_type` 是 `givm::set_skill_state_error` 的别名。`set_skill_state_error` 是本命令的结构化编译错误，`set_skill_state_error::reason` 是原因枚举。[编译检查 `check`](../../executor/check.md) 使用本次定义集合与程序种类检查以下条件；[`compile`](../../executor/compile.md) 自动收集这些错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_character_player` | `character.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_character_selection` | `character.selection` 不是 `character_selection::character`；此处只允许单个角色 |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `skill_view` 定义数量 |

### `set_skill_state_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

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
