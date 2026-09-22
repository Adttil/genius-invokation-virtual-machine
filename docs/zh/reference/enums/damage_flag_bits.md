[givm](../../reference.md) / [枚举值](../enums.md) / **damage_flag_bits**

# givm::damage_flag_bits

定义于头文件 `<givm/enums/damage_flags.hpp>`

```cpp
enum class damage_flag_bits : std::uint8_t
{
    combat_damage = 1u << 0,
    skill_damage = 1u << 1,
    reaction_damage = 1u << 2,
    ignore_shield = 1u << 3,
    normal_attack = 1u << 4,
    elemental_skill = 1u << 5,
    elemental_burst = 1u << 6
};
```

一次伤害可以附带的独立性质。多个性质通过 [`damage_flags`](damage_flags.md) 组合。

## 枚举值

|  |  |
| --- | --- |
| `combat_damage` | 战斗伤害 |
| `skill_damage` | 技能伤害 |
| `reaction_damage` | 反应伤害 |
| `ignore_shield` | 无视护盾的伤害 |
| `normal_attack` | 本段被视为普通攻击伤害 |
| `elemental_skill` | 本段被视为元素战技伤害 |
| `elemental_burst` | 本段被视为元素爆发伤害 |

这些标志彼此独立，可以组合；设置技能分类位不会自动设置 `skill_damage`。定义源应按规则明确提供本段伤害需要的全部标志。

普通攻击、元素战技和元素爆发标志描述本段伤害的性质，可在 [`damage_preparation`](../definition/events/damage_preparation.md) 中修改。它们不改变实际使用的技能，也不改写 [`skill_will_be_used`](../definition/events/skill_will_be_used.md) 或 [`skill_used`](../definition/events/skill_used.md) 中的技能身份。

## 示例

```cpp
#include <print>

#include <givm/enums/damage_flags.hpp>

int main()
{
    givm::damage_flags flags{ givm::damage_flag_bits::skill_damage };
    flags.set(givm::damage_flag_bits::ignore_shield);
    std::println("技能伤害: {}", flags.contains(givm::damage_flag_bits::skill_damage));
    std::println("忽略护盾: {}", flags.contains(givm::damage_flag_bits::ignore_shield));
    flags.reset(givm::damage_flag_bits::ignore_shield);
    std::println("移除标志后忽略护盾: {}", flags.contains(givm::damage_flag_bits::ignore_shield));
}
```

输出

```text
技能伤害: true
忽略护盾: true
移除标志后忽略护盾: false
```

## 参阅

|  |  |
| --- | --- |
| [`damage_flags`](damage_flags.md) | 伤害的性质组合 |
