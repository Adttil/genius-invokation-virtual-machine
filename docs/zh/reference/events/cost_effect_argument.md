[givm](../../reference.md) / [事件](../events.md) / **cost_effect_argument**

# givm::cost_effect_argument

定义于头文件 `<givm/executor/events.hpp>`

```cpp
template<class TCostEvent>
struct cost_effect_argument;
```

一次费用调整效果记录的减费内容。它让实际支付时的效果能够知道本次采用了怎样的减费。

## 模板参数

| | |
| --- | --- |
| `TCostEvent` | 这次支付对应的费用事件类型，例如 [`cost_of_switch`](cost_of_switch.md) |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `reduced_dice` | [`elemental_dice_requirement`](elemental_dice_requirement.md) | 本次效果减少的骰子需求 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::cost_effect_argument<givm::cost_of_switch> argument{};
    argument.reduced_dice.any = 1;
    std::println("切换减免骰数: {}", argument.reduced_dice.any);
}
```

输出

```text
切换减免骰数: 1
```
