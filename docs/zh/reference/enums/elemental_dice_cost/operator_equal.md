[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_dice_cost](../elemental_dice_cost.md) / **operator==**

# givm::operator==

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
friend constexpr bool operator==(elemental_dice_cost, elemental_dice_cost) = default;
```

比较两项费用的每类骰子数量是否相同。

## 参数

|  |  |
| --- | --- |
| 第一实参 | 第一项费用 |
| 第二实参 | 第二项费用 |

## 返回值

各类数量都相同为 `true`，否则为 `false`。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::elemental_dice_cost fire{ givm::elemental_dice::pyro, 3 };
    givm::elemental_dice_cost water{ givm::elemental_dice::hydro, 1 };
    std::println("火骰和水骰费用相同: {}", fire == water);
}
```

输出

```text
火骰和水骰费用相同: false
```
