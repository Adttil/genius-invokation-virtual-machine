[givm](../../reference.md) / [游戏用语](../enums.md) / **dice_counts**

# givm::dice_counts

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
class dice_counts;
```

一组元素骰的持有数量，分别记录每种元素以及万能元素的数量。

## 成员函数

|  |  |
| --- | --- |
| [`operator[]`](dice_counts/operator_at.md) | 读取或修改某种骰子的数量 |
| [`total`](dice_counts/total.md) | 取得骰子总数 |
| [`contains`](dice_counts/contains.md) | 检查各类骰子的数量是否足够 |
| [`operator+=`](dice_counts/operator_add_assign.md) | 增加各类骰子 |
| [`operator-=`](dice_counts/operator_subtract_assign.md) | 扣除各类骰子 |

## 非成员函数

|  |  |
| --- | --- |
| [`operator==`](dice_counts/operator_equal.md) | 比较各类骰子的数量 |

## 注意

默认构造时各种骰子的数量均为零。万能元素独立计数；`contains` 不会用万能骰自动替代其他元素。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::dice_counts dice{};
    dice[givm::elemental_dice::pyro] = 3;
    dice[givm::elemental_dice::omni] = 1;
    givm::dice_counts payment{};
    payment[givm::elemental_dice::pyro] = 2;
    std::println("骰子总数: {}", dice.total());
    std::println("足够支付指定火骰: {}", dice.contains(payment));
    dice -= payment;
    std::println("支付后总数: {}", dice.total());
}
```

输出

```text
骰子总数: 4
足够支付指定火骰: true
支付后总数: 2
```
