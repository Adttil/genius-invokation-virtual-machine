[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **cost_of_technique**

# givm::cost_of_technique

定义于头文件 `<givm/definition.hpp>`

```cpp
struct cost_of_technique;
```

使用特技的费用计算事件。响应者可以调整所需骰子、充能与行动速度。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `technique` | `const attachment_id` | 本次准备使用的特技；只读。 |
| `targets` | `const std::array<technique_target_id, 2>` | 已选定的目标；`std::monostate` 为空槽。 |
| `requirement` | [`action_cost_requirement`](../../table/action_cost_requirement.md) | 特技使用的骰子、充能费用与行动速度。 |

## 注意

报价绑定来源及完整目标，预览响应仅调用一次，不接收响应编号。同一报价的全部响应读取不变的牌桌，依次修改费用事件，并通过 `invoke(...)` 缓存确认后才执行的程序及其输入。不同目标可以产生不同费用或不同的资源消耗效果。

响应不得使用随机数。缓存程序执行后不重新报价，程序返回编号忽略。同一窗口的同一操作和目标组合只报价一次，Debug 检查误用，Release 由调用方保证。
