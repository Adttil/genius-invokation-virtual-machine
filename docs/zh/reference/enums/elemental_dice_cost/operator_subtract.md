[givm](../../../reference.md) / [游戏用语](../../enums.md) / [elemental_dice_cost](../elemental_dice_cost.md) / **operator-**

# givm::operator-

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
constexpr elemental_dice_cost operator-(elemental_dice_cost l, elemental_dice_cost r) noexcept;
```

把两项费用逐类相减。

## 参数

|  |  |
| --- | --- |
| `l` | 原费用 |
| `r` | 扣减的费用 |

## 返回值

相减后的新费用。

## 注意

调用方应保证每类计算结果可由 `unsigned char` 表示。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::elemental_dice_cost fire{ givm::elemental_dice::pyro, 3 };
    givm::elemental_dice_cost water{ givm::elemental_dice::hydro, 1 };
    const auto result = (fire + water) - water;
    std::println("扣减后等于火骰费用: {}", result == fire);
}
```

输出

```text
扣减后等于火骰费用: true
```
