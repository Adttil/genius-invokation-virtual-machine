[givm](../../../reference.md) / [游戏用语](../../enums.md) / [damage_flags](../damage_flags.md) / **operator|**

# givm::operator|

定义于头文件 `<givm/enums/damage_flags.hpp>`

```cpp
constexpr damage_flags operator|(damage_flags lhs, damage_flag_bits rhs) noexcept;
constexpr damage_flags operator|(damage_flag_bits lhs, damage_flag_bits rhs) noexcept;
```

组合伤害性质，得到新的集合。

## 参数

|  |  |
| --- | --- |
| `lhs` | 已有集合或一种性质 |
| `rhs` | 要合入的性质 |

## 返回值

合入 `rhs` 后的新集合。

## 注意

这些运算符由 [`damage_flags`](../damage_flags.md) 的友元声明提供。组合两个枚举位时，应先用第一个枚举位构造 `damage_flags`，再合入第二个位；直接写两个枚举值的 `a | b` 无法通过实参相关查找找到这里的重载。

## 示例

```cpp
#include <print>

#include <givm/enums/damage_flags.hpp>

int main()
{
    givm::damage_flags flags{ givm::damage_flag_bits::skill_damage };
    const auto combined = flags | givm::damage_flag_bits::ignore_shield;
    std::println("原集合忽略护盾: {}", flags.contains(givm::damage_flag_bits::ignore_shield));
    std::println("新集合忽略护盾: {}", combined.contains(givm::damage_flag_bits::ignore_shield));
}
```

输出

```text
原集合忽略护盾: false
新集合忽略护盾: true
```
