[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **increase_max_health**

# givm::increase_max_health

定义于头文件 `<givm/definition.hpp>`

```cpp
struct increase_max_health;
```

增加一个角色的生命上限，并恢复相同数量的生命。完成后只发送 [`healed`](../events/healed.md) 通知，不经过 [`healing`](../events/healing.md) 治疗量修饰，也不产生额外的生命上限变更事件。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`increase_max_health_input`](../command_inputs/increase_max_health_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `increase_max_health_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`relative_character_target`](../events/relative_character_target.md) | 固定来源角色的位置；默认本方的出战角色 |
| `target` | [`relative_character_target`](../events/relative_character_target.md) | 固定目标位置；默认采用动态输入 |
| `value` | `std::uint32_t` | 固定增加量；默认 `0` |

## 输入

- 默认构造 `increase_max_health{}`，消费响应通过 `invoke` 提交的一个 [`increase_max_health_input`](../command_inputs/increase_max_health_input.md)，读取其中的来源、目标和增加量；该输入不作为 `healing` 广播。
- 显式指定 `target` 时使用命令中的固定参数，不消费响应输入。

动态输入须指定有效角色。固定参数在命令执行时分别定位来源和目标；缺少任一角色时跳过命令。来源不会被替换成目标，也不会从外层响应推断；需要精确技能、牌或召唤物来源时应采用动态输入。

生命上限最多增加至 `std::uint32_t` 的最大值，治疗值为实际增加量。通知时上限和生命均已更新；增加量为 `0` 时仍通知。

例如角色生命为 `7/10`，增加 `3` 点生命上限后变为 `10/13`，完成通知中的 `value` 为 `3`。原本满血的角色也恢复相同数量，仍然满血。

## 编译检查

```cpp
struct increase_max_health_error;
```

`increase_max_health::error_type` 是 `givm::increase_max_health_error` 的别名。`increase_max_health_error` 是本命令的结构化编译错误，`increase_max_health_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_source_player` | `source.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_source_selection` | `source.selection` 不是 `character_selection::character`；此处只允许单个角色 |
| `invalid_target_player` | `target.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_target_selection` | `target.selection` 不是 `character_selection::character`；此处只允许单个角色 |

### `increase_max_health_error` 的成员对象

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
    const givm::relative_character_target target{ givm::relative_player::self, 0 };
    const givm::increase_max_health command{ .source = target, .target = target, .value = 3 };
    std::println("生命上限增加量: {}", command.value);
}
```

输出

```text
生命上限增加量: 3
```
