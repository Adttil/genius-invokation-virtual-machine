[givm](../../../reference.md) / [枚举值](../../enums.md) / [elemental_dice_cost](../elemental_dice_cost.md) / **operator+**

# givm::operator+

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
constexpr elemental_dice_cost operator+(elemental_dice_cost l, elemental_dice_cost r) noexcept;
```

把两项费用逐类相加。

## 参数

|  |  |
| --- | --- |
| `l` | 第一项费用 |
| `r` | 第二项费用 |

## 返回值

相加后的新费用。

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
    const auto total = fire + water;
    std::println("合并后的水骰费用: {}", total[givm::elemental_dice::hydro]);
}
```

输出

```text
合并后的水骰费用: 1
```
