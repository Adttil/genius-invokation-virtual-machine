[givm](../../../reference.md) / [枚举值](../../enums.md) / [dice_counts](../dice_counts.md) / **total**

# givm::dice_counts::total

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
constexpr std::uint32_t total() const noexcept;
```

统计这组骰子的总数。

## 返回值

所有元素骰和万能骰的数量之和。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::dice_counts dice{};
    dice[givm::elemental_dice::pyro] = 3;
    dice[givm::elemental_dice::omni] = 1;
    std::println("骰子总数: {}", dice.total());
}
```

输出

```text
骰子总数: 4
```
