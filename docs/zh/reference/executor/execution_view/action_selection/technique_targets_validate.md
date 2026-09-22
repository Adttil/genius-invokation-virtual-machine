[givm](../../../../reference.md) / [行动选择](../action_selection.md) / **technique_targets_validate**

# technique_targets_validate

定义于头文件 `<givm/executor.hpp>`

```cpp
target_validation technique_targets_validate(const definition_library& library, const table& card_table, std::span<const technique_target_id> targets = {}) const;
```

向特技附件查询 `technique_target_validation`。只使用 span 前两个目标；支持从空选择开始逐步检查。定义可假定前面的选择已经合法；未提供查询时仅空目标返回 `valid_complete`。检查受控可单独调用 `is_controlled`。
