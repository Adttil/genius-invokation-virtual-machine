[givm](../../../reference.md) / [定义](../../definition.md) / [事件输入](../events.md) / **healing_application**

# givm::healing_application

定义于头文件 `<givm/definition.hpp>`。

`heal{}` 的动态输入。它描述来源、目标和初始治疗量；目标可以是精确的 `character_id`，也可以是带 `selection` 范围的 [`relative_character_target`](relative_character_target.md)。精确角色目标只治疗该角色。

```cpp
using healing_target = std::variant<character_id, relative_character_target>;

struct healing_application
{
    effect_source_id source;
    healing_target target;
    std::uint32_t value;
};
```

动态输入在命令开始时解析目标位置。相对目标的 `character`、`others` 和 `all` 分别表示定位角色、除定位角色之外的其他角色和两者全部。精确角色目标和 `character` 允许生命值为 `0`；`others` 和 `all` 只选择存活角色。

范围治疗从定位角色开始按循环顺序处理，`others` 跳过定位角色，轮到每个角色时判断其是否存活。每个目标先完成 [`healing`](healing.md) 广播和实际加血，全部目标完成后再按相同顺序广播 [`healed`](healed.md)。本输入本身不广播。
