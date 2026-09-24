[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **skill_effect**

# givm::skill_effect

定义于头文件 `<givm/definition.hpp>`

```cpp
struct skill_effect;
```

执行所选技能自身效果时，提供这项技能及其目标。技能定义通过响应中的 `invoke` 提交入口以实现技能效果。本类型也用作 [`use_skill{}`](../commands/use_skill.md) 的动态输入。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `skill` | `const skill_id` | 本次使用的技能；只读。 |
| `targets` | `const std::array<skill_target_id, 2>` | 本次采用的两个目标位置；只读，未使用的位置忽略。 |
| `flags` | `const skill_flags` | 本次使用的技能性质；效果与前后通知沿用同一结果 |

## 注意

行动选择只将出战角色中支持本事件的技能列为候选。不支持本事件的技能仍可响应其他事件，不会成为主动使用候选。支持响应不要求每次都提交后续效果。

本事件仅调用所选技能自己的定义。行动选择先完成支付，然后广播 [`skill_will_be_used`](skill_will_be_used.md)；[`use_skill`](../commands/use_skill.md) 命令直接从该广播开始，不支付技能费用。广播及其响应完成后，只有未被取消的技能效果才执行本事件。其效果完成后再广播 [`skill_used`](skill_used.md)。

技能分类通过定义标签表达，不影响本事件是否存在。

行动选择中，普通攻击的重击和下落攻击性质在行动候选建立时确定，费用响应可以据此减费。`use_skill` 固定模式仅使用技能标签确定的固有分类，动态模式使用输入的 `flags`，不会自动追加这两种性质。定义通过 `event.flags.to_damage_flags()` 将相应性质显式写入动态伤害输入；不会自动修改其他命令的伤害。
