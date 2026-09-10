[givm](../../reference.md) / [事件](../events.md) / **elemental_dice_requirement**

# givm::elemental_dice_requirement

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct elemental_dice_requirement;
```

一项行动对元素骰的需求。它分别记录指定种类、相同种类和任意种类的骰子数量，便于费用效果单独调整各部分。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `fixed` | [`dice_counts`](../enums/dice_counts.md) | 指定骰子种类的需求 |
| `same` | `std::uint8_t` | 还需支付的同一种类骰子数量，初始为 0 |
| `any` | `std::uint8_t` | 还需支付的任意种类骰子数量，初始为 0 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::elemental_dice_requirement requirement{};
    requirement.fixed[givm::elemental_dice::pyro] = 2;
    requirement.any = 1;
    std::println("火元素骰需求: {}", requirement.fixed[givm::elemental_dice::pyro]);
    std::println("任意骰需求: {}", requirement.any);
}
```

输出

```text
火元素骰需求: 2
任意骰需求: 1
```
