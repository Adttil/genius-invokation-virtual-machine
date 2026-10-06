[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **heal**

# givm::heal

定义于头文件 `<givm/definition.hpp>`

```cpp
struct heal;
```

使一个或多个角色接受治疗。先为每个目标广播 [`healing`](../events/healing.md) 并完成实际加血，通知登记到当前段，在结算点广播 [`healed`](../events/healed.md) 通知实际恢复值。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`heal_input`](../command_inputs/heal_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `heal_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`relative_character_target`](../events/relative_character_target.md) | 固定来源角色的位置；默认本方的出战角色 |
| `target` | [`relative_character_target`](../events/relative_character_target.md) | 固定目标位置和范围；默认采用动态输入 |
| `value` | `std::uint32_t` | 固定治疗量；默认 `0` |
| `kind` | `healing_kind` | 普通治疗、免于击倒恢复或复苏，默认 `normal` |

## 输入

- 默认构造 `heal{}`，接受响应通过 `invoke` 提交的一个 [`heal_input`](../command_inputs/heal_input.md)。
- 显式指定 `target` 时使用命令中的固定参数，不消费响应输入。

动态输入是治疗参数数组，按输入顺序执行，允许为空或重复目标；每项指定有效来源和目标。目标可以是精确 `character_id`，也可以是带范围的 [`relative_character_target`](../events/relative_character_target.md)。固定参数描述一次单体或范围治疗，在命令执行时分别定位来源和目标；缺少任一角色时跳过命令。来源不会被替换成目标，也不会从外层响应推断；需要精确技能、牌或召唤物来源时应采用动态输入。

`normal` 只治疗存活且生命非零的角色，并进行普通 `healing` 修饰。`prevent_defeat` 只恢复仍存活、生命为零的濒死角色；`revive` 只恢复已死亡角色。这两种特殊恢复不经过普通治疗量修饰，但都产生 `healed`。复苏实际恢复非零生命后将 `alive` 设为 true，并先登记 `character_revived`。

## 结算

每个有效目标立即恢复生命，恢复量为 `min(value, max_health - health)`，通知记录实际值和只读 `kind`。满血普通治疗仍可产生实际值为零的通知。每项范围在开始时采样角色集合，按角色循环顺序执行，不因中途复活或新增角色扩充；每个目标仍按所选 kind 判断资格。数组边界不自动分段或结算，通知到结算点才执行。

## 编译检查

```cpp
struct heal_error;
```

`heal::error_type` 是 `givm::heal_error` 的别名。`heal_error` 是本命令的结构化编译错误，`heal_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_source_player` | `source.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_source_selection` | `source.selection` 不是 `character_selection::character`；此处只允许单个角色 |
| `invalid_target_player` | `target.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_target_selection` | `target.selection` 不是 `character_selection::character`、`others` 或 `all` |

### `heal_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

## 示例

```cpp
#include <print>
#include <givm/definition.hpp>

int main()
{
    const givm::relative_character_target target{ givm::relative_player::self, 0, givm::character_selection::all };
    const givm::heal command{ .target = target, .value = 2 };
    std::println("治疗量: {}", command.value);
}
```

输出

```text
治疗量: 2
```
