[givm](../../../reference.md) / [游戏用语](../../enums.md) / [elemental_dice_cost](../elemental_dice_cost.md) / **elemental_dice_cost**

# givm::elemental_dice_cost::elemental_dice_cost

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
constexpr elemental_dice_cost() noexcept;
constexpr elemental_dice_cost(elemental_dice dice, unsigned char count);
```

构造费用。无参数时所有费用为零；给定种类和数量时仅设置该类费用。

## 参数

|  |  |
| --- | --- |
| `dice` | 骰子种类 |
| `count` | 该类骰子的数量 |

## 返回值

（无）

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::elemental_dice_cost empty{};
    givm::elemental_dice_cost fire{ givm::elemental_dice::pyro, 3 };
    std::println("空费用中的火骰: {}", empty[givm::elemental_dice::pyro]);
    std::println("指定的火骰费用: {}", fire[givm::elemental_dice::pyro]);
}
```

输出

```text
空费用中的火骰: 0
指定的火骰费用: 3
```
