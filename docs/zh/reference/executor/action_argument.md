[givm](../../reference.md) / [执行](../executor.md) / **action_argument**

# givm::action_argument

定义于头文件 `<givm/executor.hpp>`

```cpp
struct action_argument;
```

执行一次行动时由调用方提交的支付骰子。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `paid_dice` | [`dice_counts`](../enums/dice_counts.md) | 实际选择支付的骰子；必须拥有这些骰子并满足本次费用 |

## 注意

当前主动切换的目标由行动候选下标确定，无需另行填写目标。可以先用行动现场的 [`check_payment`](execution_view/action_selection/check_payment.md) 检查 `paid_dice`；提交接口不会自动检查。

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
