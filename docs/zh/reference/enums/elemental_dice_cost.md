[givm](../../reference.md) / [游戏用语](../enums.md) / **elemental_dice_cost**

# givm::elemental_dice_cost

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
class elemental_dice_cost;
```

一项效果所需的各类元素骰数量。可以组合费用，也可以按种类调整数量。

## 成员函数

|  |  |
| --- | --- |
| [`(构造函数)`](elemental_dice_cost/constructor.md) | 构造一项费用 |
| [`operator[]`](elemental_dice_cost/operator_at.md) | 读取或修改某类骰子的费用 |

## 非成员函数

|  |  |
| --- | --- |
| [`operator+`](elemental_dice_cost/operator_add.md) | 合并两项费用 |
| [`operator-`](elemental_dice_cost/operator_subtract.md) | 逐类扣减费用 |
| [`operator*`](elemental_dice_cost/operator_multiply.md) | 按倍数增加费用 |
| [`operator/`](elemental_dice_cost/operator_divide.md) | 逐类除以给定数 |
| [`operator==`](elemental_dice_cost/operator_equal.md) | 比较两项费用 |

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::elemental_dice_cost fire{ givm::elemental_dice::pyro, 3 };
    givm::elemental_dice_cost water{ givm::elemental_dice::hydro, 1 };
    const auto cost = fire + water;
    std::println("火骰费用: {}", cost[givm::elemental_dice::pyro]);
    std::println("水骰费用: {}", cost[givm::elemental_dice::hydro]);
}
```

输出

```text
火骰费用: 3
水骰费用: 1
```
