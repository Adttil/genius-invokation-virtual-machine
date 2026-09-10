[givm](../../reference.md) / [游戏用语](../enums.md) / **damage_flag_bits**

# givm::damage_flag_bits

定义于头文件 `<givm/enums/damage_flags.hpp>`

```cpp
enum class damage_flag_bits : std::uint8_t
{
    combat_damage = 1u << 0,
    skill_damage = 1u << 1,
    reaction_damage = 1u << 2,
    ignore_shield = 1u << 3
};
```

一次伤害可以附带的独立性质。多个性质通过 [`damage_flags`](damage_flags.md) 组合。

## 枚举值

|  |  |
| --- | --- |
| `combat_damage` | 战斗伤害 |
| `skill_damage` | 技能伤害 |
| `reaction_damage` | 反应伤害 |
| `ignore_shield` | 忽略护盾 |

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
