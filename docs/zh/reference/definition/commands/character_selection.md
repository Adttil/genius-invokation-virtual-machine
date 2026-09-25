[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **character_selection**

# givm::character_selection

定义于头文件 `<givm/definition.hpp>`。

指定伤害或治疗在定位角色后选择哪些角色。相对目标通过 [`relative_character_target`](../events/relative_character_target.md) 的 `selection` 成员提供；动态伤害使用精确角色 ID 时，由 [`damage::selection`](../command_inputs/damage.md) 提供。

```cpp
enum class character_selection : std::uint8_t
{
    character,
    others,
    all
};
```

| 值 | 说明 |
| --- | --- |
| `character` | 仅选择定位到的角色 |
| `others` | 仅选择该玩家中除此角色以外的存活角色 |
| `all` | 选择该玩家的全部存活角色 |

伤害按属性修饰后的角色开始的循环顺序展开，每个存活角色最多选择一次；选择 `others` 时跳过此角色。每条伤害分别判定反应、计算数值和扣除生命，属性修饰只对初始描述广播一次。默认反应还可能另外派生同组伤害。

治疗的 `character` 允许选择生命值为 `0` 的角色，`others` 和 `all` 则只治疗存活角色。范围治疗按定位角色开始的循环顺序处理，`others` 跳过此角色，轮到每个角色时判断其是否存活。全部目标的治疗调整和加血完成后，再按相同顺序逐个广播完成通知。
