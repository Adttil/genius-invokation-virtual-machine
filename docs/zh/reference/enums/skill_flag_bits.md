[givm](../../reference.md) / [枚举值](../enums.md) / **skill_flag_bits**

# givm::skill_flag_bits

定义于头文件 `<givm/enums/skill_flags.hpp>`

```cpp
enum class skill_flag_bits : std::uint8_t;
```

一次技能使用可以同时具有多种性质。普通攻击、元素战技与元素爆发来自定义标签；重击和下落攻击取决于本次行动。

| 值 | 含义 |
| --- | --- |
| `normal_attack` | 普通攻击 |
| `elemental_skill` | 元素战技 |
| `elemental_burst` | 元素爆发 |
| `charged_attack` | 重击 |
| `plunging_attack` | 下落攻击 |
