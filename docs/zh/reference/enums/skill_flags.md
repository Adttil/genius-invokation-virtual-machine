[givm](../../reference.md) / [枚举值](../enums.md) / **skill_flags**

# givm::skill_flags

定义于头文件 `<givm/enums/skill_flags.hpp>`

```cpp
class skill_flags;
```

本次技能使用具有的性质集合，可同时包含普通攻击、元素爆发、重击等性质。

| 成员 | 作用 |
| --- | --- |
| [`(构造函数)`](skill_flags/constructor.md) | 构造空集合或单项集合 |
| [`value`](skill_flags/value.md) | 读取位值 |
| [`contains`](skill_flags/contains.md) | 检查性质 |
| [`set`](skill_flags/set.md) | 添加性质 |
| [`reset`](skill_flags/reset.md) | 移除性质 |
| [`to_damage_flags`](skill_flags/to_damage_flags.md) | 将技能性质用于其产生的伤害 |
| [`operator\|`](skill_flags/operator_or.md) | 组合性质 |
