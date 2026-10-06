[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **healing_kind**

# givm::healing_kind

定义于头文件 `<givm/definition.hpp>`。

区分普通治疗、免于击倒的恢复及已击倒角色的复苏。用于 heal 的固定参数、heal_input 和只读的 healed 通知。

```cpp
enum class healing_kind : std::uint8_t
{
    normal,
    prevent_defeat,
    revive
};
```

| 值 | 目标资格及行为 |
| --- | --- |
| `normal` | alive 且生命非零，经过普通 healing 修饰 |
| `prevent_defeat` | alive 且生命为零，跳过普通治疗修饰，不产生复苏通知 |
| `revive` | 已死亡，跳过普通治疗修饰；实际恢复非零生命时恢复 alive，并在 healed 前记录 character_revived |

三种恢复都记录实际治疗量。恢复种类由命令指定，不因命令位于某种响应中而自动改变。
