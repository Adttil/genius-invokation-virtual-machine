[givm](../../reference.md) / [枚举值](../enums.md) / **elemental_dice**

# givm::elemental_dice

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
enum class elemental_dice : std::uint8_t
{
    cryo,
    hydro,
    pyro,
    electro,
    geo,
    dendro,
    anemo,
    omni
};
```

元素骰的种类，包括七种元素和万能元素。

## 枚举值

|  |  |
| --- | --- |
| `cryo` | 冰元素 |
| `hydro` | 水元素 |
| `pyro` | 火元素 |
| `electro` | 雷元素 |
| `geo` | 岩元素 |
| `dendro` | 草元素 |
| `anemo` | 风元素 |
| `omni` | 万能元素 |

枚举按冰、水、火、雷、岩、草、风、万能从零开始连续编号，对应骰子均可使用时的优先顺序。`omni` 始终位于最后，其底层值加一等于骰子种类总数。需要万能元素时应显式使用 `elemental_dice::omni`；值初始化 `elemental_dice{}` 得到冰元素。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::dice_counts dice{};
    dice[givm::elemental_dice::pyro] = 2;
    dice[givm::elemental_dice::omni] = 1;
    std::println("火骰数量: {}", dice[givm::elemental_dice::pyro]);
    std::println("万能骰数量: {}", dice[givm::elemental_dice::omni]);
    std::println("骰子总数: {}", dice.total());
}
```

输出

```text
火骰数量: 2
万能骰数量: 1
骰子总数: 3
```

## 参阅

|  |  |
| --- | --- |
| [`elemental_dice_from_random`](elemental_dice_from_random.md) | 从随机值取得骰子种类 |
| [`dice_counts`](dice_counts.md) | 各种元素骰的持有数量 |
| [`elemental_dice_cost`](elemental_dice_cost.md) | 各种元素骰的费用数量 |
| [`elemental_dice_requirement`](elemental_dice_requirement.md) | 指定、同色和任意元素骰的费用需求 |
