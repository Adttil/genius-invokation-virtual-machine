[givm](../../../reference.md) / [游戏用语](../../enums.md) / [elemental_dice_cost](../elemental_dice_cost.md) / **operator/**

# givm::operator/

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
constexpr elemental_dice_cost operator/(elemental_dice_cost cost, unsigned char divisor) noexcept;
```

把每类费用分别除以给定数。

## 参数

|  |  |
| --- | --- |
| `cost` | 原费用 |
| `divisor` | 非零除数 |

## 返回值

逐类整数除法得到的新费用，舍去各类余数。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::elemental_dice_cost fire{ givm::elemental_dice::pyro, 3 };
    givm::elemental_dice_cost water{ givm::elemental_dice::hydro, 1 };
    const auto half = fire / 2;
    std::println("折半后的火骰费用: {}", half[givm::elemental_dice::pyro]);
}
```

输出

```text
折半后的火骰费用: 1
```
