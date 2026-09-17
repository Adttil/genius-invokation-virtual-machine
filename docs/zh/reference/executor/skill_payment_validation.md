[givm](../../reference.md) / [执行](../executor.md) / **skill_payment_validation**

# givm::skill_payment_validation

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class skill_payment_validation : std::uint8_t
{
    valid,
    requirement_mismatch,
    insufficient_dice,
    insufficient_energy
};
```

使用技能的支付检查结果，区分骰子费用不匹配、持有骰子不足与出战角色充能不足。

## 枚举值

| | |
| --- | --- |
| `valid` | 所选骰子符合费用要求，持有数量与出战角色充能均足够。 |
| `requirement_mismatch` | 所选骰子的种类或总数不符合费用要求。 |
| `insufficient_dice` | 所选骰子符合费用要求，但持有数量不足。 |
| `insufficient_energy` | 骰子检查通过，但出战角色充能不足。 |

## 注意

[`skill_payment_validate`](execution_view/action_selection/skill_payment_validate.md) 依次检查骰子费用匹配、持有数量和出战角色充能，遇到第一个失败立即返回；全部通过时返回 `valid`。检查不验证目标或其他行动条件。
