[givm](../../reference.md) / [执行](../executor.md) / **action_argument**

# givm::action_argument

定义于头文件 `<givm/executor.hpp>`

```cpp
struct action_argument;
```

一次行动的支付参数，记录选择支付的骰子。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `paid_dice` | [`dice_counts`](../enums/dice_counts.md) | 实际选择支付的骰子；必须拥有这些骰子并满足本次费用 |

## 注意

当前主动切换通过 [`switch_active_character`](execution_view/action_selection/switch_active_character.md) 直接接收角色 ID 和 `dice_counts`，无需构造本类型。可以先用行动现场的 [`check_switch_payment`](execution_view/action_selection/check_switch_payment.md) 检查所选骰子；提交接口不会自动检查。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::action_argument argument{};
    argument.paid_dice[givm::elemental_dice::omni] = 1;
    std::println("支付骰子总数: {}", argument.paid_dice.total());
}
```

输出

```text
支付骰子总数: 1
```

## 参阅

| | |
| --- | --- |
| [`begin_action`](../definition/commands/begin_action.md) | 行动阶段的处理命令 |
