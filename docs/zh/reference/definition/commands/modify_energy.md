[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **modify_energy**

# givm::modify_energy

定义于头文件 `<givm/definition.hpp>`

```cpp
struct modify_energy;
```

按同一有符号增量修改一个或多个角色充能的命令，用于获得或消耗充能。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`modify_energy_input`](../command_inputs/modify_energy_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `modify_energy_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | [`relative_character_target`](../events/relative_character_target.md) | 固定目标位置及范围；默认采用动态输入 |
| `delta` | `std::int64_t` | 固定充能增量，正数增加、负数减少，默认零 |

## 编译检查

```cpp
struct modify_energy_error;
```

`modify_energy::error_type` 是 `givm::modify_energy_error` 的别名。`modify_energy_error` 是本命令的结构化编译错误，`modify_energy_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_target_player` | `target.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_target_selection` | `target.selection` 不是 `character_selection::character`、`others` 或 `all` |

### `modify_energy_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

## 注意

默认构造 `modify_energy{}` 使用动态模式，由响应通过 `invoke` 提交一个 [`modify_energy_input`](../command_inputs/modify_energy_input.md)。显式指定 `target` 时采用固定模式，不消费响应输入。

固定模式在命令执行时按相对位置和 `target.selection` 确定目标：

- `character`：仅修改定位角色，允许目标为已战败但未离场的角色，不因战败顺延到其他角色。
- `others`：修改该玩家中定位角色以外的全部有效存活角色。
- `all`：修改该玩家的全部有效存活角色。

范围模式以循环偏移后的原位置为锚点，不因锚点角色战败而改变位置。没有角色或没有出战角色时跳过命令。

例如 `givm::modify_energy{ .target = { givm::relative_player::self, 0, givm::character_selection::others }, .delta = 1 }` 给命令执行时的全部本方存活后台角色各增加一点充能。

动态输入通过角色 ID 列表指定本次目标，可以跨双方；允许为空，按输入顺序逐项修改，重复 ID 会被多次修改。每个目标必须在执行时有效，允许生命值为零。列表在调用 [`invoke`](../../executor/handle_context/invoke.md) 时复制，选中的角色不会因之后出战位置改变而替换；这与固定范围在执行时确定目标不同。

命令执行时读取当前 `energy`，加上 `delta`，再将结果限制在零与当前 `max_energy` 之间。动态输入提交的是增量，不是响应时预先算出的最终值，因此此前命令对充能的修改也会参与本次计算。`INT64_MIN`、`INT64_MAX` 也遵循这一饱和规则，不发生算术回绕。

命令不改变或筛选 `energy_tag`，操作普通充能还是替代充能由定义自行决定。需要排除某些充能类型时，定义应先筛选目标，再提交动态输入。命令只修改牌桌，不发送 [`changing_energy`](../events/changing_energy.md) 或 [`energy_changed`](../events/energy_changed.md)，也不产生专门的观察现场。

技能使用不会自动增加充能。需要获得充能的技能应在自身效果程序的适当位置安排本命令，通常放在其他技能效果之后。

## 参阅

| | |
| --- | --- |
| [`modify_energy_input`](../command_inputs/modify_energy_input.md) | 按增量修改充能的动态输入 |
| [`set_energy`](set_energy.md) | 充能赋值命令及执行示例 |
