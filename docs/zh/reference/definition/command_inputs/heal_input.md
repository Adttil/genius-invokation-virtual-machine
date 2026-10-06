[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **heal_input**

# givm::heal_input

定义于头文件 `<givm/definition.hpp>`。

`heal{}` 的动态治疗序列。每个元素独立描述来源、目标、初始治疗量和治疗种类；一次响应可以决定本次需要多少次治疗。

```cpp
struct heal_input
{
    struct item;
    std::span<const item> healings;
};
```

`healings` 按顺序执行，允许为空或重复目标。元素类型及治疗资格见 [`item`](heal_input/item.md)。每个元素开始时确定自己的目标集合，后续元素读取已经更新的局面；同一范围不会因中途复活或新增角色而扩充。

数组内容在 `invoke` 或 `pack_inputs` 时复制。本输入不自动分段或结算，每次治疗的通知记录归入当前段。
