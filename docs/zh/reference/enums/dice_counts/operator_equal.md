[givm](../../../reference.md) / [游戏用语](../../enums.md) / [dice_counts](../dice_counts.md) / **operator==**

# givm::operator==

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
friend constexpr bool operator==(const dice_counts&, const dice_counts&) = default;
```

比较两组骰子是否逐类相同。

## 参数

|  |  |
| --- | --- |
| 第一实参 | 第一组骰子 |
| 第二实参 | 第二组骰子 |

## 返回值

所有种类数量相同为 `true`，否则为 `false`。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::dice_counts dice{};
    dice[givm::elemental_dice::pyro] = 3;
    dice[givm::elemental_dice::omni] = 1;
    auto copy = dice;
    std::println("两组相同: {}", copy == dice);
    copy[givm::elemental_dice::pyro] = 2;
    std::println("修改后相同: {}", copy == dice);
}
```

输出

```text
两组相同: true
修改后相同: false
```
