[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage**

# givm::damage

定义于头文件 `<givm/definition.hpp>`

一段伤害的动态初始描述。由响应通过 `invoke` 提交给 [`deal_damage`](../commands/deal_damage.md)；编译时的固定描述使用 [`fixed_damage`](../commands/fixed_damage.md)。一段描述可以选择一个角色、该角色以外的其他角色或全部角色。它本身不广播；命令先定位角色并通过一次 [`damage_preparation`](damage_preparation.md) 修饰属性，再将范围展开为具体伤害。

```cpp
struct damage;
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`damage_source_id`](damage_source_id.md) | 造成伤害的来源 |
| `target` | [`damage_target`](damage_target.md) | 具体角色或相对出战位置 |
| `selection` | [`character_selection`](../commands/character_selection.md) | `target` 为精确角色 ID 时的范围，默认为 `character`；相对目标使用其自身的 `selection` |
| `value` | `std::uint32_t` | 初始伤害值 |
| `multiplier_numerator` | `std::uint16_t` | 初始倍率分子，默认为 1 |
| `multiplier_denominator` | `std::uint16_t` | 初始倍率的非零分母，默认为 1 |
| `type` | [`damage_type`](../../enums/damage_type.md) | 伤害种类 |
| `flags` | [`damage_flags`](../../enums/damage_flags.md) | 伤害附加属性 |

## 目标

- [`character_id`](../../table/character_id.md)：指定定位角色，开始处理该描述时若已移除或生命为 0，跳过该描述。作用范围由 `damage::selection` 确定。
- [`relative_character_target`](relative_character_target.md)：在处理该描述时，先以当前效果本方为基准确定玩家，再以所选一方的当前出战角色为基准定位存活目标。作用范围由 `relative_character_target::selection` 确定，此分支忽略 `damage::selection`。使用此分支须有有效的 `self_player`，精确角色 ID 分支不依赖本方。

每段描述只广播一次 `damage_preparation`，其中的 `target` 是定位到的具体角色。响应可以修改此角色及伤害属性，但不能修改 `selection`。修饰结束后，`character` 选择修饰后的角色，`others` 选择该角色以外的其他存活角色，`all` 选择该角色及其他存活角色。其他角色按该角色的下一位置开始循环选取；所有展开的伤害使用同一份修饰后的来源、元素、数值、倍率和标志，不分别广播属性修饰事件。

范围在准备该描述时确定，实际展开数量取决于存活角色数量；无需在编译时指定目标数量。需要同时命中多个独立的精确目标时，也可以通过 `invoke` 为同一 `deal_damage` 依次提交多个 `damage`，命令的 `input_count` 与初始描述数量一致。例如 `input_count = 2` 仍只要求两个输入，即使其中一个输入选择了 `others`。

整组先准备属性和反应，再依次进行数值计算、护盾处理和扣血。前一次扣血可以影响后一次的数值计算，但不撤销已经准备的反应及其派生伤害；同组的伤害后响应推迟到全部伤害完成以后。完整规则见 [`deal_damage`](../commands/deal_damage.md)。

`flags` 中的普通攻击、元素战技和元素爆发标志描述本段伤害的性质，可独立组合。命令不会根据 `source` 自动补充这些标志；来源为技能时也应由定义源明确提供本段需要的标志。
