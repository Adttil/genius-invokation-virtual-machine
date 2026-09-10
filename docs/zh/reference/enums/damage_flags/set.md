[givm](../../../reference.md) / [游戏用语](../../enums.md) / [damage_flags](../damage_flags.md) / **set**

# givm::damage_flags::set

定义于头文件 `<givm/enums/damage_flags.hpp>`

```cpp
constexpr void set(damage_flag_bits bit) noexcept;
```

为伤害增加一种性质。

## 参数

|  |  |
| --- | --- |
| `bit` | 要添加的性质 |

## 返回值

（无）

## 示例

```cpp
#include <print>

#include <givm/enums/damage_flags.hpp>

int main()
{
    givm::damage_flags flags{ givm::damage_flag_bits::skill_damage };
    flags.set(givm::damage_flag_bits::ignore_shield);
    std::println("添加后忽略护盾: {}", flags.contains(givm::damage_flag_bits::ignore_shield));
}
```

输出

```text
添加后忽略护盾: true
```
