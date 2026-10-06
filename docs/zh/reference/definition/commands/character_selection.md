[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **character_selection**

# givm::character_selection

定义于头文件 `<givm/definition.hpp>`。

指定伤害、治疗或充能增量修改在定位角色后选择哪些角色。相对目标通过 [`relative_character_target`](../events/relative_character_target.md) 的 `selection` 成员提供；动态伤害使用精确角色 ID 时，由 [`damage::selection`](../command_inputs/deal_damage_input.md) 提供。

```cpp
enum class character_selection : std::uint8_t
{
    character,
    others,
    all,
    prioritized
};
```

| 值 | 说明 |
| --- | --- |
| `character` | 仅选择定位到的角色 |
| `others` | 仅选择该玩家中除此角色以外的存活角色 |
| `all` | 选择该玩家的全部存活角色 |
| `prioritized` | 仅用于伤害，从定位位置开始按循环顺序选择第一名 `alive && health > 0` 的角色 |

普通伤害范围以原始定位位置为锚点，在操作开始时采样目标；`others` 排除锚点，不先寻找替代锚点。生命为零但仍 `alive` 的角色可以继续受伤和触发反应，已确认击倒者被排除。每个命中分别广播属性修饰、计算伤害和执行反应；后续反应产生的伤害归入当前段。

`prioritized` 只选择一个角色，跳过零生命和已击倒者；没有候选时无操作。它只决定此次命中，不修改出战角色，不要求玩家选择新出战。普通 `character` 不使用这种自动转移规则。

治疗的范围从原始锚点按循环顺序采样，`others` 跳过锚点；普通治疗排除已击倒者，复苏范围允许包含已击倒者。轮到每个目标时再按治疗种类判断资格，见 [`heal`](heal.md)。治疗记录归入当前段，在结算点处理通知，不支持 `prioritized`。

固定模式的 [`modify_energy`](modify_energy.md) 也支持三个范围。`character` 允许选择生命值为零的角色，`others` 和 `all` 只修改有效存活角色；范围锚点是原本指定的位置，不因角色战败而改变。变化后把充能变化通知加入当前段。
