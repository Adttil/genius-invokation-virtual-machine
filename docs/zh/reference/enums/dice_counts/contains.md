[givm](../../../reference.md) / [游戏用语](../../enums.md) / [dice_counts](../dice_counts.md) / **contains**

# givm::dice_counts::contains

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
constexpr bool contains(const dice_counts& other) const noexcept;
```

检查每种骰子的数量是否都足以提供指定的一组骰子。

## 参数

|  |  |
| --- | --- |
| `other` | 需要的各类骰子数量 |

## 返回值

每一类的数量都不少于 `other` 时为 `true`，否则为 `false`。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::dice_counts dice{};
    dice[givm::elemental_dice::pyro] = 3;
    dice[givm::elemental_dice::omni] = 1;
    givm::dice_counts required{};
    required[givm::elemental_dice::pyro] = 4;
    std::println("足够提供四枚火骰: {}", dice.contains(required));
}
```

输出

```text
足够提供四枚火骰: false
```
