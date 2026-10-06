[givm](../../reference.md) / [执行](../executor.md) / **skill_cost_id**

# givm::skill_cost_id

定义于头文件 `<givm/runtime.hpp>`

```cpp
using skill_cost_id = action_cost_id<cost_of_skill>;
```

使用技能报价的强类型标识，由 [`calculate_skill_cost`](execution_view/action_selection/calculate_skill_cost.md) 返回。

只能用于产生它的行动窗口及包含该报价的窗口副本。它可复制、比较相等性，与其他行动的报价标识不能互换；没有公开默认构造或数字转换。

同一候选的不同目标组合可各有一个标识。追加其他报价不使已有标识失效；窗口结束后失效。

参阅 [`action_cost_id`](action_cost_id.md)。
