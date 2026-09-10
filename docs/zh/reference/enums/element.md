[givm](../../reference.md) / [游戏用语](../enums.md) / **element**

# givm::element

定义于头文件 `<givm/enums/element.hpp>`

```cpp
enum class element : std::uint8_t
{
    none,
    cryo,
    hydro,
    pyro,
    electro,
    anemo,
    geo,
    dendro
};
```

对局中的元素种类，可用于描述元素附着、伤害带来的元素以及元素反应。

## 枚举值

|  |  |
| --- | --- |
| `none` | 无元素 |
| `cryo` | 冰 |
| `hydro` | 水 |
| `pyro` | 火 |
| `electro` | 雷 |
| `anemo` | 风 |
| `geo` | 岩 |
| `dendro` | 草 |

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_reaction.hpp>

int main()
{
    const auto reaction = givm::reaction_between(givm::element::hydro, givm::element::pyro);
    std::println("水与火发生蒸发: {}", reaction == givm::elemental_reaction::vaporize);
    const auto aura = givm::aura_from_element(givm::element::cryo);
    std::println("冰元素对应冰附着: {}", aura == givm::element_aura::cryo);
}
```

输出

```text
水与火发生蒸发: true
冰元素对应冰附着: true
```
