[givm](../../reference.md) / [执行](../executor.md) / **card_payment_validation**

# givm::card_payment_validation

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class card_payment_validation : std::uint8_t
{
    valid,
    requirement_mismatch,
    insufficient_dice,
    energy_tag_mismatch,
    insufficient_energy
};
```

出牌的支付检查结果，区分骰子费用不匹配、持有骰子不足、充能类型不匹配与充能不足。

## 枚举值

| | |
| --- | --- |
| `valid` | 所选骰子符合费用要求且持有数量足够，非零充能费用的类型匹配且数量足够。 |
| `requirement_mismatch` | 所选骰子的种类或总数不符合费用要求。 |
| `insufficient_dice` | 所选骰子符合费用要求，但持有数量不足。 |
| `energy_tag_mismatch` | 骰子检查通过，充能费用非零，但出战角色与费用的 `energy_tag` 不同。 |
| `insufficient_energy` | 骰子与充能类型检查通过，但出战角色充能不足。 |

## 注意

[`card_payment_validate`](execution_view/action_selection/card_payment_validate.md) 依次检查骰子费用匹配、持有数量、非零充能费用的类型匹配和充能数量，遇到第一个失败立即返回；全部通过时返回 `valid`。零充能费用忽略 `energy_tag`。检查不验证目标或其他行动条件。
