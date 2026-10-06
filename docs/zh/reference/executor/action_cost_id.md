[givm](../../reference.md) / [执行](../executor.md) / **action_cost_id**

# givm::action_cost_id

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<class TCost>
class action_cost_id;
```

行动窗口中的报价标识，表示已经为具体来源和目标计算好的费用及确认时需执行的效果。

## 模板参数

| | |
| --- | --- |
| `TCost` | 对应报价事件类型；通常直接使用 `switch_cost_id`、`card_cost_id`、`skill_cost_id` 或 `technique_cost_id`。 |

## 非成员函数

| | |
| --- | --- |
| [`operator==`](action_cost_id/operator_eq.md) | 比较两个同类报价标识是否相同。 |

## 注意

标识由报价接口返回，可以复制，不可公开构造。其有效期是所属行动窗口；追加报价不使已有标识失效。复制执行器时，既有报价随窗口复制，已有标识可用于包含该报价的副本。副本各自新增的报价仅属于各自的缓存，不可因候选相同而互换标识。

借用的费用引用可能在继续报价时失效，需要通过对应标识重新读取。Debug 检查错误窗口，Release 由调用方保证标识有效。
