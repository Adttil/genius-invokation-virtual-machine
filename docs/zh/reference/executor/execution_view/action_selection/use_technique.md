[givm](../../../../reference.md) / [行动选择](../action_selection.md) / **use_technique**

# use_technique

定义于头文件 `<givm/executor.hpp>`

```cpp
void use_technique(const dice_counts& paid_dice, std::span<const technique_target_id> targets = {}) const noexcept;
void use_technique(const definition_library& library, const table& card_table, const dice_counts& paid_dice, std::span<const technique_target_id> targets = {}) const;
```

选择当前特技装备，提交支付骰子与最多两个目标。无库参数版本使用已计算报价；另一版本先计算报价，要求此前未计算。调用者保证存在特技、出战角色未受控、支付与目标合法；执行不重复检查。充能按报价从支付时的出战角色扣除。支付响应结算后广播 `technique_will_be_used`，未取消时单播 `technique_effect`；最后总是广播 `technique_used`。行动是否交给对方由最终速度决定。
