[givm](../../../../reference.md) / [定义](../../../definition.md) / [命令输入](../../command_inputs.md) / [heal_input](../heal_input.md) / **item**

# givm::heal_input::item

定义于头文件 `<givm/definition.hpp>`。

一次治疗的参数，作为 [`heal_input`](../heal_input.md) 数组的元素。

```cpp
struct heal_input::item
{
    effect_source_id source;
    healing_target target;
    std::uint32_t value;
    healing_kind kind = healing_kind::normal;
};
```

`target` 可以是角色 ID，或带 `character`、`others`、`all` 范围的 [`relative_character_target`](../../events/relative_character_target.md)。精确 ID 不会自动改选其他角色，范围从原始锚点按角色循环顺序采样。

`normal` 要求目标 `alive` 且生命非零，并进行普通 [`healing`](../../events/healing.md) 修饰；`prevent_defeat` 要求 `alive` 且生命为零；`revive` 要求已经击倒。后两种恢复跳过普通治疗修饰。实际治疗登记 [`healed`](../../events/healed.md)，复苏成功还会在它之前登记 `character_revived`。
