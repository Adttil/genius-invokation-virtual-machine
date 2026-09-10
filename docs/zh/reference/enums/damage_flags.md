[givm](../../reference.md) / [游戏用语](../enums.md) / **damage_flags**

# givm::damage_flags

定义于头文件 `<givm/enums/damage_flags.hpp>`

```cpp
class damage_flags;
```

一次伤害同时具备的性质，例如技能伤害、反应伤害以及是否忽略护盾。

## 成员函数

|  |  |
| --- | --- |
| [`(构造函数)`](damage_flags/constructor.md) | 构造伤害性质集合 |
| [`from_bits`](damage_flags/from_bits.md) | 从位值还原集合 |
| [`value`](damage_flags/value.md) | 取得位值 |
| [`contains`](damage_flags/contains.md) | 判断是否具有一种性质 |
| [`set`](damage_flags/set.md) | 添加一种性质 |
| [`reset`](damage_flags/reset.md) | 移除一种性质 |

## 非成员函数

|  |  |
| --- | --- |
| [`operator|`](damage_flags/operator_or.md) | 组合伤害性质 |

## 示例

```cpp
#include <print>

#include <givm/enums/damage_flags.hpp>

int main()
{
    givm::damage_flags flags{ givm::damage_flag_bits::skill_damage };
    flags.set(givm::damage_flag_bits::ignore_shield);
    std::println("是否技能伤害: {}", flags.contains(givm::damage_flag_bits::skill_damage));
    std::println("是否忽略护盾: {}", flags.contains(givm::damage_flag_bits::ignore_shield));
}
```

输出

```text
是否技能伤害: true
是否忽略护盾: true
```
