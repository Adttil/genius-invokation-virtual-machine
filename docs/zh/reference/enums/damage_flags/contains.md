[givm](../../../reference.md) / [游戏用语](../../enums.md) / [damage_flags](../damage_flags.md) / **contains**

# givm::damage_flags::contains

定义于头文件 `<givm/enums/damage_flags.hpp>`

```cpp
[[nodiscard]] constexpr bool contains(damage_flag_bits bit) const noexcept;
```

判断这次伤害是否具有指定性质。

## 参数

|  |  |
| --- | --- |
| `bit` | 要检查的一种性质 |

## 返回值

包含该性质为 `true`，否则为 `false`。

## 示例

```cpp
#include <print>

#include <givm/enums/damage_flags.hpp>

int main()
{
    givm::damage_flags flags{ givm::damage_flag_bits::skill_damage };
    std::println("是否反应伤害: {}", flags.contains(givm::damage_flag_bits::reaction_damage));
}
```

输出

```text
是否反应伤害: false
```
