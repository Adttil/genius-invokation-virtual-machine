[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_flags](../damage_flags.md) / **reset**

# givm::damage_flags::reset

定义于头文件 `<givm/enums/damage_flags.hpp>`

```cpp
constexpr void reset(damage_flag_bits bit) noexcept;
```

移除伤害的一种性质，保留其他性质。

## 参数

|  |  |
| --- | --- |
| `bit` | 要移除的性质 |

## 返回值

（无）

## 示例

```cpp
#include <print>

#include <givm/enums/damage_flags.hpp>

int main()
{
    givm::damage_flags flags{ givm::damage_flag_bits::skill_damage };
    flags.reset(givm::damage_flag_bits::skill_damage);
    std::println("移除后仍是技能伤害: {}", flags.contains(givm::damage_flag_bits::skill_damage));
}
```

输出

```text
移除后仍是技能伤害: false
```
