[givm](../../../reference.md) / [游戏用语](../../enums.md) / [elemental_dice_cost](../elemental_dice_cost.md) / **operator***

# givm::operator*

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
constexpr elemental_dice_cost operator*(elemental_dice_cost cost, unsigned char multiplier) noexcept;
constexpr elemental_dice_cost operator*(elemental_dice dice, unsigned char count) noexcept;
constexpr elemental_dice_cost operator*(unsigned char count, elemental_dice dice) noexcept;
```

将每类费用乘以给定倍数，或用骰子种类与数量直接创建一项费用。

## 参数

|  |  |
| --- | --- |
| `cost` | 原费用 |
| `multiplier` | 倍数 |
| `dice` | 骰子种类 |
| `count` | 数量 |

## 返回值

扩大后的费用，或仅包含指定种类的费用。

## 注意

调用方应保证每类计算结果可由 `unsigned char` 表示。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    const auto cost = givm::elemental_dice::pyro * 2;
    const auto doubled = cost * 2;
    std::println("翻倍后的火骰费用: {}", doubled[givm::elemental_dice::pyro]);
}
```

输出

```text
翻倍后的火骰费用: 4
```
