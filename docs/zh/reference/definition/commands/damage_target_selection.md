[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **damage_target_selection**

# givm::damage_target_selection

定义于头文件 `<givm/definition.hpp>`。

指定伤害在定位角色并完成属性修饰后选择哪些角色。固定描述通过 [`relative_damage_target`](relative_damage_target.md) 与相对位置一起提供；动态描述通过 [`damage::selection`](../events/damage.md) 提供，可以配合精确角色 ID 或相对位置。

```cpp
enum class damage_target_selection : std::uint8_t
{
    character,
    others,
    all
};
```

| 值 | 说明 |
| --- | --- |
| `character` | 仅命中定位到的存活角色 |
| `others` | 仅命中该玩家中除此角色以外的存活角色 |
| `all` | 先命中该角色，再命中其余存活角色 |

范围按属性修饰后的角色开始的循环顺序展开，每个角色最多选择一次；选择 `others` 时跳过此角色。范围中的每条伤害分别判定反应、计算数值和扣除生命，属性修饰只对初始描述广播一次。默认反应还可能另外派生同组伤害。
