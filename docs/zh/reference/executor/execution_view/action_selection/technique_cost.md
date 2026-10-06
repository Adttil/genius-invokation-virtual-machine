[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **technique_cost**

# givm::execution_view<execution_state::action_selection>::technique_cost

定义于头文件 `<givm/runtime.hpp>`

```cpp
const cost_of_technique& technique_cost(technique_cost_id id) const noexcept(/* Release 为 true，Debug 为 false */);
```

读取指定报价的使用特技费用及已经确定的行动信息。

## 参数

| | |
| --- | --- |
| `id` | 本窗口的对应报价标识。 |

## 返回值

借用的只读 `cost_of_technique` 引用。

## 异常

Debug 检查现场、候选或报价标识的有效性，并以结构化异常报告误用。费用响应抛出的异常直接传递；失败报价不能采用或在当前窗口重算。Release 不执行这些输入校验。

## 注意

本操作不重新报价。后续报价可能扩容并使引用失效，需要时通过同一标识重新取得。原报价在复制时随行动窗口复制，已有标识可用于该副本；窗口结束后失效。
