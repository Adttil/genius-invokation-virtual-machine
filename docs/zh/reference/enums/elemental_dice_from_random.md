[givm](../../reference.md) / [枚举值](../enums.md) / **elemental_dice_from_random**

# givm::elemental_dice_from_random

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
constexpr elemental_dice elemental_dice_from_random(std::uint32_t random) noexcept;
```

把一个随机值映射为一种元素骰。

## 参数

|  |  |
| --- | --- |
| `random` | 随机源提供的无符号值 |

## 返回值

`random % 8` 所对应的骰子，依次为万能、冰、水、火、雷、风、岩、草。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    std::println("0 对应万能: {}", givm::elemental_dice_from_random(0) == givm::elemental_dice::omni);
    std::println("11 对应火: {}", givm::elemental_dice_from_random(11) == givm::elemental_dice::pyro);
}
```

输出

```text
0 对应万能: true
11 对应火: true
```

## 参阅

|  |  |
| --- | --- |
| [`elemental_dice`](elemental_dice.md) | 元素骰的种类 |
| [`start_dice_roll_phase`](../executor/instructions/start_dice_roll_phase.md) | 投骰与重投对随机值的使用 |
