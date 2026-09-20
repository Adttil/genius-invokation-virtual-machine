[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **elemental_tuning_dice_validate**

# givm::execution_view<execution_state::action_selection>::elemental_tuning_dice_validate

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr elemental_tuning_dice_validation elemental_tuning_dice_validate(const table& card_table, elemental_dice from) const noexcept;
```

检查一枚指定元素的骰子能否用于元素调和。正常转换的目标元素取自当前出战角色的 `state().element`。

## 参数

| | |
| --- | --- |
| `card_table` | 当前行动发生的牌桌。 |
| `from` | 选择转换的骰子元素。 |

## 返回值

返回 `elemental_tuning_dice_validation`。按以下顺序检查，首次失败立即返回：

| 枚举值 | 含义 |
| --- | --- |
| `omni_not_allowed` | 选择的是万能骰。 |
| `missing_character_element` | 出战角色的自身元素为 `element::none`。 |
| `same_element` | 选择的骰子与出战角色自身元素相同。 |
| `insufficient_dice` | 当前玩家没有这种元素骰。 |
| `valid` | 所有检查通过。 |

## 注意

该检查与卡牌许可检查相互独立，不提交行动。修饰事件可以改变最终转换结果，但不放宽这里的选骰条件。
