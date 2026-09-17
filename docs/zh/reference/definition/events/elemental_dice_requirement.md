[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **elemental_dice_requirement**

# givm::elemental_dice_requirement

定义于头文件 `<givm/definition.hpp>`

```cpp
struct elemental_dice_requirement;
```

一项行动对元素骰的需求。它分别记录指定种类、相同种类和任意种类的骰子数量，便于费用效果单独调整各部分。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `fixed` | [`dice_counts`](../../enums/dice_counts.md) | 指定骰子种类的需求 |
| `same` | `std::uint8_t` | 还需支付的同一种类骰子数量，初始为 0 |
| `any` | `std::uint8_t` | 还需支付的任意种类骰子数量，初始为 0 |

## 注意

支付的骰子须能分配给互不重叠的三部分：

- `fixed` 中的普通元素需求可以用同类骰子或万能骰满足；`fixed[omni]` 必须使用万能骰。
- `same` 是额外的一组同色骰子，可以混入万能骰，也可以全部使用万能骰；所选颜色不必与 `fixed` 相同。
- `any` 可以使用剩余的任意种类骰子，包括万能骰。

总数必须恰好为 `fixed.total() + same + any`，不能少付或多付。只要存在满足三部分的分配就符合费用要求；是否持有这些骰子另行检查。行动现场的 [`switch_payment_validate`](../../executor/execution_view/action_selection/switch_payment_validate.md) 会依次检查这两项。

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
