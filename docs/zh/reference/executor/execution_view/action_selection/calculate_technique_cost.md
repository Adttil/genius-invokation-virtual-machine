[givm](../../../../reference.md) / [行动选择](../action_selection.md) / **calculate_technique_cost**

# calculate_technique_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
const cost_of_technique& calculate_technique_cost(const definition_library& library, const table& card_table) const;
```

以附件定义的 `technique_initial_cost` 为初值，广播 `cost_of_technique` 并保留支付时执行的响应。前提为存在特技，且当前现场尚未计算其报价；禁止重复计算。费用响应禁止使用随机数。
