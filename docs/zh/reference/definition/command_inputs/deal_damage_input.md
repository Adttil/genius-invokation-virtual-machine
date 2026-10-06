[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **deal_damage_input**

# givm::deal_damage_input

定义于头文件 `<givm/definition.hpp>`

响应为一条动态伤害命令提交的伤害序列。数组长度可以在响应时决定，每个元素描述一次单体或范围伤害。

```cpp
struct deal_damage_input
{
    std::span<const damage> damages;
};
```

`damages` 按顺序执行，允许为空或重复目标；元素类型为 [`damage`](damage.md)。每个元素开始时解析自己的目标范围，后续元素可以看到之前操作的变化。同一元素的目标集合在开始时确定，不因中途复活、切换或新增角色重新采样。

数组内容在 `invoke` 或 `pack_inputs` 时复制，提交完成后不再借用原数组。本输入不自动分段或结算；同段、同目标的伤害与其他伤害命令一起合并通知，数组边界不影响合并。
