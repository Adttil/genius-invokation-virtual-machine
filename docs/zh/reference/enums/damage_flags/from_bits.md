[givm](../../../reference.md) / [游戏用语](../../enums.md) / [damage_flags](../damage_flags.md) / **from_bits**

# givm::damage_flags::from_bits

定义于头文件 `<givm/enums/damage_flags.hpp>`

```cpp
static constexpr damage_flags from_bits(std::uint8_t bits) noexcept;
```

从保存的位值恢复伤害性质集合。

## 参数

|  |  |
| --- | --- |
| `bits` | 各标志位的组合 |

## 返回值

包含给定位值的集合。

## 示例

```cpp
#include <print>

#include <givm/enums/damage_flags.hpp>

int main()
{
    givm::damage_flags flags{ givm::damage_flag_bits::skill_damage };
    const auto restored = givm::damage_flags::from_bits(flags.value());
    std::println("恢复了技能伤害标志: {}", restored.contains(givm::damage_flag_bits::skill_damage));
}
```

输出

```text
恢复了技能伤害标志: true
```
