[givm](../../../reference.md) / [游戏用语](../../enums.md) / [dice_counts](../dice_counts.md) / **operator+=**

# givm::dice_counts::operator+=

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
constexpr dice_counts& operator+=(const dice_counts& other) noexcept;
```

增加每种骰子的数量。

## 参数

|  |  |
| --- | --- |
| `other` | 要增加的数量 |

## 返回值

`*this`。

## 注意

调用方应确保每类结果均在 0 到 255 的范围内。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::dice_counts dice{};
    dice[givm::elemental_dice::pyro] = 3;
    dice[givm::elemental_dice::omni] = 1;
    givm::dice_counts added{};
    added[givm::elemental_dice::hydro] = 2;
    dice += added;
    std::println("增加后总数: {}", dice.total());
}
```

输出

```text
增加后总数: 6
```
