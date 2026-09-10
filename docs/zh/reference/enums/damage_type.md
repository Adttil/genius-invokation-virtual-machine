[givm](../../reference.md) / [枚举值](../enums.md) / **damage_type**

# givm::damage_type

定义于头文件 `<givm/enums/damage_type.hpp>`

```cpp
enum class damage_type : std::uint8_t
{
    cryo,
    hydro,
    pyro,
    electro,
    anemo,
    geo,
    dendro,
    physical,
    piercing,
    true_damage
};
```

伤害的元素或特殊类别。它用于决定元素附着和相应的伤害结算方式。

## 枚举值

|  |  |
| --- | --- |
| `cryo` | 冰元素伤害 |
| `hydro` | 水元素伤害 |
| `pyro` | 火元素伤害 |
| `electro` | 雷元素伤害 |
| `anemo` | 风元素伤害 |
| `geo` | 岩元素伤害 |
| `dendro` | 草元素伤害 |
| `physical` | 物理伤害 |
| `piercing` | 穿透伤害 |
| `true_damage` | 真实伤害 |

## 示例

```cpp
#include <print>

#include <givm/enums/damage_type.hpp>

int main()
{
    const auto elemental = givm::element_from_damage_type(givm::damage_type::pyro);
    const auto physical = givm::element_from_damage_type(givm::damage_type::physical);
    std::println("火伤对应火元素: {}", elemental == givm::element::pyro);
    std::println("物理伤害没有对应元素: {}", physical == givm::element::none);
}
```

输出

```text
火伤对应火元素: true
物理伤害没有对应元素: true
```

## 参阅

|  |  |
| --- | --- |
| [`element_from_damage_type`](element_from_damage_type.md) | 取得伤害所带的元素 |
