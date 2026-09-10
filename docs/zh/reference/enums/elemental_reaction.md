[givm](../../reference.md) / [游戏用语](../enums.md) / **elemental_reaction**

# givm::elemental_reaction

定义于头文件 `<givm/enums/elemental_reaction.hpp>`

```cpp
enum class elemental_reaction : std::uint8_t
{
    none,
    melt,
    vaporize,
    overloaded,
    superconduct,
    electro_charged,
    lunar_charged,
    frozen,
    swirl,
    crystallize,
    burning,
    bloom,
    quicken
};
```

元素相遇时产生的反应种类。反应的伤害和后续效果由相应的结算决定。

## 枚举值

|  |  |
| --- | --- |
| `none` | 无反应 |
| `melt` | 融化 |
| `vaporize` | 蒸发 |
| `overloaded` | 超载 |
| `superconduct` | 超导 |
| `electro_charged` | 感电 |
| `lunar_charged` | 月感电 |
| `frozen` | 冻结 |
| `swirl` | 扩散 |
| `crystallize` | 结晶 |
| `burning` | 燃烧 |
| `bloom` | 绽放 |
| `quicken` | 激化 |

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_reaction.hpp>

int main()
{
    const auto melt = givm::reaction_between(givm::element::cryo, givm::element::pyro);
    const auto coexist = givm::reaction_between(givm::element::cryo, givm::element::dendro);
    std::println("冰与火的反应是融化: {}", melt == givm::elemental_reaction::melt);
    std::println("冰与草不发生反应: {}", coexist == givm::elemental_reaction::none);
}
```

输出

```text
冰与火的反应是融化: true
冰与草不发生反应: true
```
