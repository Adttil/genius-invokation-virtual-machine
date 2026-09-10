[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_flags](../damage_flags.md) / **damage_flags**

# givm::damage_flags::damage_flags

定义于头文件 `<givm/enums/damage_flags.hpp>`

```cpp
constexpr damage_flags() noexcept;
constexpr damage_flags(damage_flag_bits bit) noexcept;
```

构造空的伤害性质集合，或从一种性质开始构造。

## 参数

|  |  |
| --- | --- |
| `bit` | 初始包含的伤害性质 |

## 返回值

（无）

## 示例

```cpp
#include <print>

#include <givm/enums/damage_flags.hpp>

int main()
{
    givm::damage_flags flags{ givm::damage_flag_bits::skill_damage };
    givm::damage_flags empty{};
    std::println("空集合包含技能伤害: {}", empty.contains(givm::damage_flag_bits::skill_damage));
    std::println("指定集合包含技能伤害: {}", flags.contains(givm::damage_flag_bits::skill_damage));
}
```

输出

```text
空集合包含技能伤害: false
指定集合包含技能伤害: true
```
